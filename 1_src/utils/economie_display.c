#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "economie_display.h"
#include "economie_model.h"

/*
 * Permet de donner un nom lisible à une ressource.
 * Exemple : resource_id = 3 -> "farine".
 *
 * Si la fonction renvoie NULL, l'affichage peut retomber sur l'indice brut.
 */
const char* ResourceToName(
    EcoResourceId resource_id,
    void *context
) {
    if( context == NULL) return NULL;

    EcoResourceId n = *   ((EcoResourceId**)context)  [0];
    if(resource_id > n) return NULL;

    return ((char**)context)   [resource_id];
}

/*
 * Permet de donner un nom lisible à un acteur.
 * Exemple : actor_id = 2 -> "boulanger".
 *
 * Si la fonction renvoie NULL, l'affichage peut retomber sur l'identifiant brut.
 */
const char* ActorToName(
    EcoActorId actor_id,
    void *context
) {
    if( context == NULL) return NULL;

    EcoResourceId n = *   ((EcoResourceId**)context)  [0];
    if(actor_id > n) return NULL;

    return ((char**)context)   [actor_id];
}

/* Remplit options avec des paramètres raisonnables par défaut. */
void eco_display_options_default(EcoDisplayOptions *options) {
    options->decimals = ECO_DISPLAY_DEFAULT_DECIMALS;
    options->show_indices = false;
    options->show_size_cells = true;
    options->compact = false;
    options->trailing_newline = true;
    options->indent = 2u;
    options->max_vector_entries = ECO_DISPLAY_NO_LIMIT;
    options->max_matrix_cols = ECO_DISPLAY_NO_LIMIT;
    options->actor_name = ActorToName;
    options->resource_name = ResourceToName;
    options->actor_name_context = NULL;
    options->resource_name_context = NULL;
}

/*
 * Écrit value dans buffer sous forme décimale.
 *
 * Exemple avec ECO_REAL_SCALE = 1000000 :
 *   value = 1234567, decimals = 3 -> "1.234"
 *
 * Retour proposé : nombre de caractères écrits, hors '\0', ou valeur négative.
 */
int eco_real_to_string(
    EcoReal value, 
    uint32_t decimals, 
    char *buffer, 
    size_t buffer_size
) 
{
    // Sécurité de base
    if (buffer_size == 0) return -1;
    if (buffer_size == 1) { buffer[0] = '\0'; return -1; }

    // 1. Gestion du signe et conversion en non-signé pour optimiser les divisions
    bool is_negative = value < 0;
    // Gestion sûre de la limite absolue (ex: INT64_MIN)
    uint64_t uvalue = is_negative ? (uint64_t)-(value + 1) + 1 : (uint64_t)value;

    uint64_t int_part = uvalue / ECO_REAL_SCALE;
    uint64_t frac_part = uvalue % ECO_REAL_SCALE;

    // 2. Détermination dynamique de la précision native (le nombre de zéros du scale)
    uint32_t scale_digits = 0;
    uint64_t temp_scale = ECO_REAL_SCALE;
    while (temp_scale > 1) {
        scale_digits++;
        temp_scale /= 10;
    }

    // 3. Ajustement de la partie fractionnaire par rapport au nombre de décimales demandé
    if (decimals < scale_digits) {
        for (uint32_t i = 0; i < (scale_digits - decimals); i++) frac_part /= 10;
    } else if (decimals > scale_digits) {
        for (uint32_t i = 0; i < (decimals - scale_digits); i++) frac_part *= 10;
    }

    // Un buffer temporaire local (65 octets sont largement suffisants pour un entier 64 bits)
    char temp[64];
    int pos = sizeof(temp) - 1;
    temp[pos] = '\0';

    // 4. Génération de la partie fractionnaire (de droite à gauche)
    if (decimals > 0) {
        for (uint32_t i = 0; i < decimals; i++) {
            pos--;
            temp[pos] = '0' + (frac_part % 10);
            frac_part /= 10;
        }
        pos--;
        temp[pos] = '.';
    }

    // 5. Génération de la partie entière (de droite à gauche)
    if (int_part == 0) {
        pos--;
        temp[pos] = '0';
    } else {
        while (int_part > 0) {
            pos--;
            temp[pos] = '0' + (int_part % 10);
            int_part /= 10;
        }
    }

    // 6. Ajout du signe si nécessaire
    if (is_negative) {
        pos--;
        temp[pos] = '-';
    }

    // 7. Vérification finale de la taille et copie
    size_t length = (sizeof(temp) - 1) - pos;
    if (length >= buffer_size) {
        return -1; // Le buffer est trop petit
    }

    for (size_t i = 0; i <= length; i++) {
        buffer[i] = temp[pos + i];
    }

    return length;
}


/* Écrit value dans stream sous forme décimale. */
int eco_real_fprint(
    FILE *stream,
    EcoReal value,
    uint32_t decimals
) {
    char* str = malloc(sizeof(char)*32); //il y à au plus 31 lettre mais 32 est une puissance de 2
    if(str != NULL){
        eco_real_to_string(value, decimals, str, 32);
        fprintf(stream,"%s", str);
        free(str);
        return 0;
    }
    return 1;
}

/* Écrit value sur stdout sous forme décimale. */
int eco_real_print(
    EcoReal value,
    uint32_t decimals
) {
    return eco_real_fprint(stdout, value, decimals);
}

/* ------------------------------------------------------------------------- */
/* Affichage des vecteurs et matrices                                        */
/* ------------------------------------------------------------------------- */

int eco_vector_fprint(
    FILE *stream,
    EcoVector vector,
    const EcoDisplayOptions *options
) {
    int n = 0;
    EcoIndex taille = vector[0];
    fprintf(stream, "%dv( ", taille);
    for(EcoIndex i = 1; i < taille; i++) {
        n += eco_real_fprint(stream, vector[i], options->decimals);
        fprintf(stream, ", ");
    }
    n += eco_real_fprint(stream, vector[taille], options->decimals);
    fprintf(stream, " )\n");
    return n;
}

int eco_vector_print(
    EcoVector vector,
   const EcoDisplayOptions *options
) {
    return eco_vector_fprint(stdout, vector, options);
}

int eco_matrix_fprint(
    FILE *stream,
    EcoMatrix matrix,
    const EcoDisplayOptions *options
) {
    EcoIndex n = ECO_MAT_ROWS(matrix);
    EcoIndex m = ECO_MAT_COLS(matrix);
    fprintf(stream, "(%d,%d)m -\n", n,m);

    EcoReal somme = 0;

    for(EcoIndex i = 1; i <= n; i++){
        fprintf(stream, "[ ");
        for(EcoIndex j = 1; j < m; j++){
            somme += eco_real_fprint(stream, matrix[i][j], options->decimals);
            fprintf(stream, ", ");
        }
        somme += eco_real_fprint(stream, matrix[i][m], options->decimals);
        fprintf(stream, " ]\n");
    }

    return somme;
}

int eco_matrix_print(
    EcoMatrix matrix,
    const EcoDisplayOptions *options
) {
    return eco_matrix_fprint(stdout, matrix, options);
}

int eco_flag_matrix_fprint(
    FILE *stream,
    EcoFlagMatrix matrix,
    const EcoDisplayOptions *options
) {
    //TODO Creer un système d'affichage qui gère le contexte
    if(options != NULL) return -1;

    EcoIndex n = ECO_MAT_ROWS(matrix);
    EcoIndex m = ECO_MAT_COLS(matrix);
    fprintf(stream, "(%d,%d)m -\n", n,m);

    EcoReal somme = 0;

    for(EcoIndex i = 1; i <= n; i++){
        fprintf(stream, "[ ");
        for(EcoIndex j = 1; j < m; j++){
            if(matrix[i][j] == 0) fprintf(stream, "X, ");
            else fprintf(stream, "O, ");
        }
        if(matrix[i][m] == 0) fprintf(stream, "X, ");
        else fprintf(stream, "O, ");
    }

    return somme;
}

int eco_flag_matrix_print(
    EcoFlagMatrix matrix,
    const EcoDisplayOptions *options
) {
    return eco_flag_matrix_fprint(stdout, matrix, options);
}

/* ------------------------------------------------------------------------- */
/* Affichage des objets du modèle                                            */
/* ------------------------------------------------------------------------- */

// typedef struct EcoActor {
//     EcoActorId id;
//     EcoVector resources;
//     EcoProductionFunction production;
//     void *production_context;
//     EcoResourceId produced_resource;
//     EcoVector consumption_pattern;
// } EcoActor;

int eco_actor_fprint(
    FILE *stream,
    const EcoActor *actor,
    const EcoDisplayOptions *options
) {
    const char* name = (options->actor_name)(actor->id, options->actor_name_context);
    const char* production = (options->resource_name)(actor->produced_resource, options->resource_name_context);
    if(name == NULL) {
        if(production == NULL) {
            fprintf(stream, "a(%d,%d)", actor->id, actor->produced_resource);
        } else {
            fprintf(stream, "a(%d,%s)", actor->id, production);
        }
    } else {
        if(production == NULL) {
            fprintf(stream, "a(%s,%d)", name, actor->produced_resource);
        } else {
            fprintf(stream, "a(%s,%s)", name, production);
        }
    }
    return 0;
}

int eco_actor_print(
    const EcoActor *actor,
    const EcoDisplayOptions *options
) {
    return eco_actor_fprint(stdout, actor, options);
}

int eco_actor_set_fprint(
    FILE *stream,
    const EcoActorSet *actors,
    const EcoDisplayOptions *options
) {
    EcoIndex nb_printed = 0;
    EcoIndex nb = actors->count;
    fprintf(stream, "A[ ");
    for(EcoIndex i = 1; i < nb; i++){
        nb_printed++;
        eco_actor_fprint(stream, actors->actors[i], options);
        fprintf(stream, ", ");
    }
    nb_printed++;
    eco_actor_fprint(stream, actors->actors[nb], options);
    fprintf(stream, " ]\n");

    return nb_printed;
}

int eco_actor_set_print(
    const EcoActorSet *actors,
    const EcoDisplayOptions *options
) {
    return eco_actor_set_fprint(stdout, actors, options);
}

int eco_tactic_fprint(
    FILE *stream,
    const EcoTactic *tactic,
    const EcoDisplayOptions *options
) {
    fprintf(stream, " === Tactic === \n\nDelta_e\n");
    eco_matrix_fprint(stream, tactic->emission, options);
    fprintf(stream, "\nDelta_r\n");
    eco_matrix_fprint(stream, tactic->reception, options);
    
    fprintf(stream, "\n === Tactic === \n");

    return 0;
}

int eco_tactic_print(
    const EcoTactic *tactic,
    const EcoDisplayOptions *options
) {
    return eco_tactic_fprint(stdout, tactic, options);
}

int eco_exchange_table_fprint(
    FILE *stream,
    const EcoExchangeTable *exchange_table,
    const EcoDisplayOptions *options
) {
    fprintf(stream, " === Exchange Table === \n\nEmitted\n");
    eco_matrix_fprint(stream, exchange_table->emitted, options);
    fprintf(stream, "\nReceived\n");
    eco_matrix_fprint(stream, exchange_table->received, options);
    
    fprintf(stream, "\n === Exchange Table === \n");

    return 0;
}

int eco_exchange_table_print(
    const EcoExchangeTable *exchange_table,
    const EcoDisplayOptions *options
) {
    return eco_exchange_table_fprint(stdout, exchange_table, options);
}


/* ------------------------------------------------------------------------- */
/* Affichages spécialisés utiles au débogage                                 */
/* ------------------------------------------------------------------------- */

/* Affiche uniquement les ressources possédées par un acteur. */
int eco_actor_resources_fprint(
    FILE *stream,
    const EcoActor *actor,
    const EcoDisplayOptions *options
) {
    EcoIndex n = actor->resources[0];
    for(EcoIndex i = 1; i <=n; i++){
        fprintf(stream, "\n === resources === \n");
        if(actor->resources[i] > 0) {
            fprintf(stream, "ressource %d -> ", i);
            eco_real_fprint(stream, actor->resources[i], options->decimals);
            fprintf(stream, "\n");
        } 
        fprintf(stream, "\n === resources === \n");
    }

    return n;
}

/* Affiche uniquement le patron de consommation d'un acteur. */
int eco_actor_consumption_pattern_fprint(
    FILE *stream,
    const EcoActor *actor,
    const EcoDisplayOptions *options
) {
    EcoIndex n = actor->consumption_pattern[0];
    for(EcoIndex i = 1; i <=n; i++){
        fprintf(stream, "\n === consumption patterns === \n");
        if(actor->consumption_pattern[i] > 0) {
            fprintf(stream, "ressource %d -> ", i);
            eco_real_fprint(stream, actor->consumption_pattern[i], options->decimals);
            fprintf(stream, "\n");
        }
        fprintf(stream, "\n === consumption patterns === \n");
    }

    return n;
}

/* Affiche une ligne de matrice, avec la convention d'indexation à partir de 1. */
int eco_matrix_row_fprint(
    FILE *stream,
    EcoMatrix matrix,
    EcoIndex row,
    const EcoDisplayOptions *options
) {
    return eco_vector_fprint(stream, matrix[row], options);
}

/*
 * Affiche une colonne de matrice.
 * Cette fonction est utile même si les colonnes ne sont pas contiguës en mémoire,
 * puisque la matrice est un tableau de tableaux.
 */
int eco_matrix_column_fprint(
    FILE *stream,
    EcoMatrix matrix,
    EcoIndex column,
    const EcoDisplayOptions *options
) {
    EcoIndex n = matrix[0][0];
    EcoVector col = malloc(sizeof(EcoReal)*(n+1));
    for(EcoIndex i = 1; i<= n; i++) col[i] = matrix[i][column];
    col[0] = n;

    int r = eco_vector_fprint(stream, col, options);
    free(col);
    return r;
}