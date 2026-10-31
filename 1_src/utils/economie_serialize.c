#include <stdlib.h>
#include <inttypes.h>
#include "economie_serialize.h"

/* --- INTERFACE PUBLIQUE DE SÉRIALISATION DES VECTEURS ET MATRICES --- */

int eco_serialize_vector(FILE *stream, const EcoVector vector) {
    if (vector == NULL) return -1;
    EcoIndex length = ECO_VEC_LENGTH(vector);
    if (fprintf(stream, "%" PRIu32, length) < 0) return -1;
    for (EcoIndex i = 1; i <= length; i++) {
        if (fprintf(stream, " %" PRId64, vector[i]) < 0) return -1;
    }
    return (fprintf(stream, "\n") < 0) ? -1 : 0;
}

int eco_serialize_matrix(FILE *stream, const EcoMatrix matrix) {
    if (matrix == NULL) return -1;
    
    // On extrait le nombre de lignes de manière sécurisée
    EcoIndex rows = matrix[0][0]; 
    
    // Sécurisation de l'accès à la ligne 1
    EcoIndex cols = (rows > 0 && matrix[1] != NULL) ? matrix[1][0] : 0;
    
    // Sauvegarde des dimensions : Lignes Colonnes
    if (fprintf(stream, "%" PRIu32 " %" PRIu32 "\n", rows, cols) < 0) return -1;
    
    for (EcoIndex i = 1; i <= rows; i++) {
        for (EcoIndex j = 1; j <= cols; j++) {
            if (matrix[i] == NULL) {
                if (fprintf(stream, "0 ") < 0) return -1;
            } else {
                if (fprintf(stream, "%" PRId64 " ", matrix[i][j]) < 0) return -1;
            }
        }
        if (fprintf(stream, "\n") < 0) return -1;
    }
    return 0;
}

EcoVector eco_deserialize_vector(FILE *stream) {
    if (stream == NULL) return NULL;
    EcoIndex length;
    if (fscanf(stream, "%" SCNu32, &length) != 1) return NULL;
    EcoVector vector = eco_vector_create(length);
    if (vector == NULL) return NULL;
    for (EcoIndex i = 1; i <= length; i++) {
        if (fscanf(stream, "%" SCNd64, &vector[i]) != 1) {
            eco_vector_destroy(vector);
            return NULL;
        }
    }
    return vector;
}

EcoMatrix eco_deserialize_matrix(FILE *stream) {
    if (stream == NULL) return NULL;
    EcoIndex rows, cols;
    if (fscanf(stream, "%" SCNu32 " %" SCNu32, &rows, &cols) != 2) return NULL;
    EcoMatrix matrix = eco_matrix_create(rows, cols);
    if (matrix == NULL) return NULL;
    for (EcoIndex i = 1; i <= rows; i++) {
        for (EcoIndex j = 1; j <= cols; j++) {
            if (fscanf(stream, "%" SCNd64, &matrix[i][j]) != 1) {
                eco_matrix_destroy(matrix);
                return NULL;
            }
        }
    }
    return matrix;
}

/* --- INTERFACE PUBLIQUE DE SÉRIALISATION DU MARCHÉ --- */

int eco_serialize_market(FILE *stream, const EcoMarket *market, const EcoMapper *mapper) {
    if (stream == NULL || market == NULL || mapper == NULL) return -1;

    /* 1. Paramètres globaux du marché */
    int update_id = mapper->update_to_id(market->update);
    int weight_id = mapper->weight_to_id(market->weight);
    if (fprintf(stream, "%d %d\n\n", update_id, weight_id) < 0) return -1;

    /* 2. Stratégie */
    EcoIndex tactic_count = market->strategy.tactic_count;
    if (fprintf(stream, "%" PRIu32 "\n", tactic_count) < 0) return -1;

    if (tactic_count == 0) {
        /* Mode Générateur */
        int gen_id = mapper->generator_to_id(market->strategy.generator);
        if (fprintf(stream, "%d\n\n", gen_id) < 0) return -1;
    } else {
        /* Mode suite finie de tactiques */
        for (EcoIndex i = 1; i <= tactic_count; i++) {
            if (market->strategy.tactics[i] == NULL) return -1;
            // Utilisation des nouvelles fonctions publiques
            if (eco_serialize_matrix(stream, market->strategy.tactics[i]->emission) < 0) return -1;
            if (eco_serialize_matrix(stream, market->strategy.tactics[i]->reception) < 0) return -1;
            if (fprintf(stream, "\n") < 0) return -1;
        }
    }

    /* 3. Ensemble des acteurs */
    if (market->initial_actors == NULL) return -1;
    EcoIndex actor_count = market->initial_actors->count;
    if (fprintf(stream, "%" PRIu32 "\n", actor_count) < 0) return -1;

    for (EcoIndex i = 1; i <= actor_count; i++) {
        EcoActor *actor = market->initial_actors->actors[i];
        if (actor == NULL) return -1;
        int prod_func_id = mapper->production_to_id(actor->production);
        
        if (fprintf(stream, "%" PRIu32 " %" PRIu32 " %d\n", actor->id, actor->produced_resource, prod_func_id) < 0) return -1;
        // Utilisation des nouvelles fonctions publiques
        if (eco_serialize_vector(stream, actor->resources) < 0) return -1;
        if (eco_serialize_vector(stream, actor->consumption_pattern) < 0) return -1;
    }

    return 0;
}

EcoMarket *eco_deserialize_market(FILE *stream, const EcoMapper *mapper) {
    if (stream == NULL || mapper == NULL) return NULL;

    EcoMarket *market = calloc(1, sizeof(EcoMarket));
    if (market == NULL) return NULL;

    /* 1. Paramètres globaux du marché */
    int update_id, weight_id;
    if (fscanf(stream, "%d %d", &update_id, &weight_id) != 2) {
        free(market);
        return NULL;
    }
    market->update = mapper->id_to_update(update_id);
    market->weight = mapper->id_to_weight(weight_id);
    market->update_context = NULL; 
    market->weight_context = NULL;

    /* 2. Stratégie */
    EcoIndex tactic_count;
    if (fscanf(stream, "%" SCNu32, &tactic_count) != 1) {
        free(market);
        return NULL;
    }
    market->strategy.tactic_count = tactic_count;

    if (tactic_count == 0) {
        int gen_id;
        if (fscanf(stream, "%d", &gen_id) != 1) {
            free(market);
            return NULL;
        }
        market->strategy.generator = mapper->id_to_generator(gen_id);
        market->strategy.generator_context = NULL;
        market->strategy.tactics = NULL;
    } else {
        market->strategy.generator = NULL;
        market->strategy.generator_context = NULL;
        market->strategy.tactics = calloc(tactic_count + 1, sizeof(EcoTactic*));
        if (market->strategy.tactics == NULL) {
            free(market);
            return NULL;
        }
        for (EcoIndex i = 1; i <= tactic_count; i++) {
            EcoTactic *tac = malloc(sizeof(EcoTactic));
            if (tac == NULL) {
                eco_market_destroy_custom(market);
                return NULL;
            }
            market->strategy.tactics[i] = tac;
            // Utilisation des nouvelles fonctions publiques
            tac->emission = eco_deserialize_matrix(stream);
            tac->reception = eco_deserialize_matrix(stream);
            if (tac->emission == NULL || tac->reception == NULL) {
                eco_market_destroy_custom(market);
                return NULL;
            }
        }
    }

    /* 3. Ensemble des acteurs */
    EcoIndex actor_count;
    if (fscanf(stream, "%" SCNu32, &actor_count) != 1) {
        eco_market_destroy_custom(market);
        return NULL;
    }

    market->initial_actors = malloc(sizeof(EcoActorSet));
    if (market->initial_actors == NULL) {
        eco_market_destroy_custom(market);
        return NULL;
    }
    market->initial_actors->count = actor_count;
    market->initial_actors->actors = calloc(actor_count + 1, sizeof(EcoActor*));
    if (market->initial_actors->actors == NULL) {
        free(market->initial_actors);
        market->initial_actors = NULL;
        eco_market_destroy_custom(market);
        return NULL;
    }

    for (EcoIndex i = 1; i <= actor_count; i++) {
        EcoActorId id;
        EcoResourceId prod_res;
        int prod_func_id;
        if (fscanf(stream, "%" SCNu32 " %" SCNu32 " %d", &id, &prod_res, &prod_func_id) != 3) {
            eco_market_destroy_custom(market);
            return NULL;
        }

        EcoActor *actor = eco_actor_create(id);
        if (actor == NULL) {
            eco_market_destroy_custom(market);
            return NULL;
        }
        market->initial_actors->actors[i] = actor;
        actor->produced_resource = prod_res;
        actor->production = mapper->id_to_production(prod_func_id);
        actor->production_context = NULL;

        // Utilisation des nouvelles fonctions publiques
        actor->resources = eco_deserialize_vector(stream);
        actor->consumption_pattern = eco_deserialize_vector(stream);
        if (actor->resources == NULL || actor->consumption_pattern == NULL) {
            eco_market_destroy_custom(market);
            return NULL;
        }
    }

    return market;
}

void eco_market_destroy_custom(EcoMarket *market) {
    if (market == NULL) return;

    if (market->strategy.tactics != NULL) {
        for (EcoIndex i = 1; i <= market->strategy.tactic_count; i++) {
            if (market->strategy.tactics[i] != NULL) {
                eco_matrix_destroy(market->strategy.tactics[i]->emission);
                eco_matrix_destroy(market->strategy.tactics[i]->reception);
                free(market->strategy.tactics[i]);
            }
        }
        free(market->strategy.tactics);
    }

    if (market->initial_actors != NULL) {
        if (market->initial_actors->actors != NULL) {
            for (EcoIndex i = 1; i <= market->initial_actors->count; i++) {
                if (market->initial_actors->actors[i] != NULL) {
                    eco_actor_destroy(market->initial_actors->actors[i]);
                }
            }
            free(market->initial_actors->actors);
        }
        free(market->initial_actors);
    }
    free(market);
}