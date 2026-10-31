#ifndef ECONOMIE_DISPLAY_H
#define ECONOMIE_DISPLAY_H

/*
 * economie_display.h
 *
 * Fonctions d'affichage pour les structures définies dans economie_model.h.
 *
 * Ce fichier ne contient aucune implémentation : seulement des types,
 * constantes et prototypes. Les fonctions prévues ici servent à afficher
 * les nombres en virgule fixe EcoReal, les vecteurs, les matrices, les acteurs
 * et les objets principaux du modèle.
 *
 * Convention de retour proposée :
 *   - les fonctions renvoient 0 en cas de succès ;
 *   - elles renvoient une valeur négative en cas d'erreur.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "economie_model.h"

/* ------------------------------------------------------------------------- */
/* Constantes d'affichage                                                    */
/* ------------------------------------------------------------------------- */

/* Nombre de décimales affichées par défaut pour EcoReal. */
#ifndef ECO_DISPLAY_DEFAULT_DECIMALS
#define ECO_DISPLAY_DEFAULT_DECIMALS 3u
#endif

/* Nombre maximal de décimales autorisées à l'affichage. */
#ifndef ECO_DISPLAY_MAX_DECIMALS
#define ECO_DISPLAY_MAX_DECIMALS 9u
#endif

/* Valeur spéciale : l'implémentation choisit un nombre pertinent de décimales. */
#define ECO_DISPLAY_DECIMALS_AUTO UINT32_MAX

/* Valeur spéciale : pas de limite d'affichage pour les éléments d'un tableau. */
#define ECO_DISPLAY_NO_LIMIT ((EcoIndex)0)

/* ------------------------------------------------------------------------- */
/* Fonctions optionnelles de nommage                                         */
/* ------------------------------------------------------------------------- */

/*
 * Permet de donner un nom lisible à une ressource.
 * Exemple : resource_id = 3 -> "farine".
 *
 * Si la fonction renvoie NULL, l'affichage peut retomber sur l'indice brut.
 */
typedef const char *(*EcoResourceNameFunction)(EcoResourceId resource_id,
                                               void *context);

/*
 * Permet de donner un nom lisible à un acteur.
 * Exemple : actor_id = 2 -> "boulanger".
 *
 * Si la fonction renvoie NULL, l'affichage peut retomber sur l'identifiant brut.
 */
typedef const char *(*EcoActorNameFunction)(EcoActorId actor_id,
                                            void *context);

/* ------------------------------------------------------------------------- */
/* Options générales d'affichage                                             */
/* ------------------------------------------------------------------------- */

typedef struct EcoDisplayOptions {
    /*
     * Nombre de décimales à afficher pour EcoReal.
     * Utiliser ECO_DISPLAY_DECIMALS_AUTO pour laisser l'implémentation choisir.
     */
    uint32_t decimals;

    /* Afficher les indices des vecteurs et matrices. */
    bool show_indices;

    /* Afficher les cases de taille v[0], M[0][0] et M[i][0]. */
    bool show_size_cells;

    /* Affichage compact sur une ligne lorsque c'est possible. */
    bool compact;

    /* Ajouter un retour à la ligne final. */
    bool trailing_newline;

    /* Nombre d'espaces d'indentation au début des lignes. */
    unsigned indent;

    /*
     * Limite du nombre d'éléments affichés pour les vecteurs.
     * ECO_DISPLAY_NO_LIMIT signifie : tout afficher.
     */
    EcoIndex max_vector_entries;

    /*
     * Limite du nombre de lignes affichées pour les matrices.
     * ECO_DISPLAY_NO_LIMIT signifie : tout afficher.
     */
    EcoIndex max_matrix_rows;

    /*
     * Limite du nombre de colonnes affichées par ligne pour les matrices.
     * ECO_DISPLAY_NO_LIMIT signifie : tout afficher.
     */
    EcoIndex max_matrix_cols;

    /* Fonctions optionnelles de nommage. */
    EcoActorNameFunction actor_name;
    void* actor_name_context;

    EcoResourceNameFunction resource_name;
    void* resource_name_context;
} EcoDisplayOptions;

/* Remplit options avec des paramètres raisonnables par défaut. */
void eco_display_options_default(EcoDisplayOptions *options);

/* ------------------------------------------------------------------------- */
/* Affichage des nombres EcoReal                                             */
/* ------------------------------------------------------------------------- */

/*
 * Écrit value dans buffer sous forme décimale.
 *
 * Exemple avec ECO_REAL_SCALE = 1000000 :
 *   value = 1234567, decimals = 3 -> "1.234"
 *
 * Retour proposé : nombre de caractères écrits, hors '\0', ou valeur négative.
 */
int eco_real_to_string(EcoReal value,
                       uint32_t decimals,
                       char *buffer,
                       size_t buffer_size);

/* Écrit value dans stream sous forme décimale. */
int eco_real_fprint(FILE *stream,
                    EcoReal value,
                    uint32_t decimals);

/* Écrit value sur stdout sous forme décimale. */
int eco_real_print(EcoReal value,
                   uint32_t decimals);

/* ------------------------------------------------------------------------- */
/* Affichage des vecteurs et matrices                                        */
/* ------------------------------------------------------------------------- */

int eco_vector_fprint(FILE *stream,
                      EcoVector vector,
                      const EcoDisplayOptions *options);

int eco_vector_print(EcoVector vector,
                     const EcoDisplayOptions *options);

int eco_matrix_fprint(FILE *stream,
                      EcoMatrix matrix,
                      const EcoDisplayOptions *options);

int eco_matrix_print(EcoMatrix matrix,
                     const EcoDisplayOptions *options);

int eco_flag_matrix_fprint(FILE *stream,
                           EcoFlagMatrix matrix,
                           const EcoDisplayOptions *options);

int eco_flag_matrix_print(EcoFlagMatrix matrix,
                          const EcoDisplayOptions *options);

/* ------------------------------------------------------------------------- */
/* Affichage des objets du modèle                                            */
/* ------------------------------------------------------------------------- */

int eco_actor_fprint(FILE *stream,
                     const EcoActor *actor,
                     const EcoDisplayOptions *options);

int eco_actor_print(const EcoActor *actor,
                    const EcoDisplayOptions *options);

int eco_actor_set_fprint(FILE *stream,
                         const EcoActorSet *actors,
                         const EcoDisplayOptions *options);

int eco_actor_set_print(const EcoActorSet *actors,
                        const EcoDisplayOptions *options);

int eco_tactic_fprint(FILE *stream,
                      const EcoTactic *tactic,
                      const EcoDisplayOptions *options);

int eco_tactic_print(const EcoTactic *tactic,
                     const EcoDisplayOptions *options);

int eco_exchange_table_fprint(FILE *stream,
                              const EcoExchangeTable *exchange_table,
                              const EcoDisplayOptions *options);

int eco_exchange_table_print(const EcoExchangeTable *exchange_table,
                             const EcoDisplayOptions *options);

// int eco_market_fprint(FILE *stream,
//                       const EcoMarket *market,
//                       const EcoDisplayOptions *options);

// int eco_market_print(const EcoMarket *market,
//                      const EcoDisplayOptions *options);

// int eco_state_sequence_fprint(FILE *stream,
//                               const EcoStateSequence *sequence,
//                               const EcoDisplayOptions *options);

// int eco_state_sequence_print(const EcoStateSequence *sequence,
//                              const EcoDisplayOptions *options);

/* ------------------------------------------------------------------------- */
/* Affichages spécialisés utiles au débogage                                 */
/* ------------------------------------------------------------------------- */

/* Affiche uniquement les ressources possédées par un acteur. */
int eco_actor_resources_fprint(FILE *stream,
                               const EcoActor *actor,
                               const EcoDisplayOptions *options);

/* Affiche uniquement le patron de consommation d'un acteur. */
int eco_actor_consumption_pattern_fprint(FILE *stream,
                                         const EcoActor *actor,
                                         const EcoDisplayOptions *options);

/* Affiche une ligne de matrice, avec la convention d'indexation à partir de 1. */
int eco_matrix_row_fprint(FILE *stream,
                          EcoMatrix matrix,
                          EcoIndex row,
                          const EcoDisplayOptions *options);

/*
 * Affiche une colonne de matrice.
 * Cette fonction est utile même si les colonnes ne sont pas contiguës en mémoire,
 * puisque la matrice est un tableau de tableaux.
 */
int eco_matrix_column_fprint(FILE *stream,
                             EcoMatrix matrix,
                             EcoIndex column,
                             const EcoDisplayOptions *options);

#endif /* ECONOMIE_DISPLAY_H */
