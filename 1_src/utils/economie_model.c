#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#include "economie_model.h"
#include "economie_display.h"

#define LLONG_MAX 9223372036854775807
/*
* Fonction mathématique utile
*/

EcoReal max(EcoReal a, EcoReal b) { return a < b ? b : a; }
EcoReal min(EcoReal a, EcoReal b) { return a < b ? a : b; }


/* ------------------------------------------------------------------------- */
/* Signatures optionnelles pour les futures implémentations                  */
/* ------------------------------------------------------------------------- */

//IMPLEMENTED eco_vector_create
EcoVector eco_vector_create(EcoIndex length){
    EcoVector vect =  calloc((length+1),sizeof(EcoReal));
    if(vect != NULL){
        vect[0] = length;
        return vect;
    }
    return NULL;
}

//IMPLEMENTED eco_vector_destroy
void eco_vector_destroy(EcoVector vector){
    if(vector != NULL) free(vector);
}


//IMPLEMENTED eco_matrix_create
EcoMatrix eco_matrix_create(EcoIndex rows, EcoIndex cols) {
    EcoMatrix matrix = malloc(sizeof(EcoVector)*(rows + 1));
    for(EcoIndex i = 0; i < rows + 1; i ++) matrix[i] = eco_vector_create(cols);

    matrix[0][0] = rows;
    return matrix;
}

//IMPLEMENTED eco_matrix_destroy
void eco_matrix_destroy(EcoMatrix matrix){
    if(matrix != NULL){
    EcoIndex n = ECO_MAT_ROWS(matrix);
    
        for(EcoIndex i = 0; i <= n; i++)eco_vector_destroy(matrix[i]);
        free(matrix);
    }
}

//IMPLEMENTED eco_actor_create
EcoActor *eco_actor_create(EcoIndex id) {


    EcoActor* actor = malloc(sizeof(EcoActor));
    actor->id = id;
    actor->resources = NULL;
    actor->production = NULL;
    actor->production_context = NULL;
    actor->produced_resource = 0;
    actor->consumption_pattern = NULL;
    return actor;
    
}

//IMPLEMENTED eco_actor_destroy
void eco_actor_destroy(EcoActor *actor) {

    if(actor == NULL) return;

    if(actor->resources != NULL) free(actor->resources);
    if(actor->consumption_pattern != NULL) free(actor->consumption_pattern);

    free(actor);
    return;
}

//IMPLEMENTED eco_actor_set_create
EcoActorSet *eco_actor_set_create(EcoIndex count) {
    EcoActorSet* set = malloc(sizeof(EcoActorSet));

    set->actors = malloc(sizeof(EcoActor*)*(count+1));
    set->actors[0] = NULL;

    set->count = count;

    return set;
}

//IMPLEMENTED eco_actor_set_destroye
void eco_actor_set_destroy(EcoActorSet *set){
    if(set == NULL) return;

    if(set->actors != NULL) {
        for(EcoIndex i = 1; i <= set->count; i++) {
            if(set->actors[i] != NULL) {
                eco_actor_destroy(set->actors[i]); // On utilise le vrai destructeur !
            }
        }
        free(set->actors); // On libère le tableau de pointeurs
    }

    free(set);
    return;
}

//IMPLEMENTED eco_tactic_create
EcoTactic *eco_tactic_create(EcoIndex actor_count) {
    EcoTactic* tactic = malloc(sizeof(EcoTactic));
    tactic->emission = eco_matrix_create(actor_count, actor_count);
    tactic->reception = eco_matrix_create(actor_count, actor_count);

    return tactic;
}

//IMPLEMENTED eco_tactic_destroy
void eco_tactic_destroy(EcoTactic *tactic){
    if(tactic == NULL) return;
    free(tactic->emission);
    free(tactic->reception);
    free(tactic);

    return;
}

//IMPLEMENTED eco_exchange_table_create
EcoExchangeTable *eco_exchange_table_create(EcoIndex actor_count) {
    EcoExchangeTable* exchanges = malloc(sizeof(EcoExchangeTable));
    exchanges->emitted = eco_matrix_create(actor_count, actor_count);
    exchanges->received = eco_matrix_create(actor_count,actor_count);

    return exchanges;
}

//IMPLEMENTED eco_exchange_table_destroy
void eco_exchange_table_destroy(EcoExchangeTable *exchange_table) {
    if( exchange_table == NULL) return; 
    free(exchange_table->emitted);
    free(exchange_table->received);
    free(exchange_table);

    return;
}

/*
 * Prototypes des briques mathématiques. A partir de là on code vraiment le TIPE
 */
//IMPLEMENTED eco_price_matrix_before_negotiation
EcoPriceMatrix eco_price_matrix_before_negotiation(
    const EcoActorSet *actors,
    const EcoTactic *tactic
) {

    EcoIndex n = actors->count;

    EcoMatrix non_negociated_price_matrix = eco_matrix_create(n,n);
    EcoReal delta_e, delta_r;


    for(EcoIndex i = 1; i <= n; i++) {
        for(EcoIndex j = 1; j <= n; j++){
            if(i !=j){
                delta_e = tactic->emission[i][j];
                delta_r = tactic->reception[i][j];

                //TODO modifier le condition dans le fichier latex : implication deviens équivalence
                if(delta_e !=0) {
                    non_negociated_price_matrix[i][j] = delta_e * ECO_REAL_SCALE / delta_r;
                }
                else non_negociated_price_matrix[i][j] = 0;
            }
            else non_negociated_price_matrix[i][i] = 0;
        }
    }


    return non_negociated_price_matrix;
}

//IMPLEMENTED eco_price_matrix_after_negotiation
EcoPriceMatrix eco_price_matrix_after_negotiation(
    const EcoActorSet *actors,
    const EcoPriceMatrix prices,
    EcoWeightFunction weight,
    void *weight_context
) {
    EcoIndex n = actors->count;

    EcoPriceMatrix price_matrix = eco_matrix_create(n,n);

    EcoReal p_a, p_b_hat, lambda;

    for(EcoIndex i = 1; i <=n; i++) {
        for(EcoIndex j = i + 1; j <= n; j++){
            price_matrix[i][i] = 0;

            /* Ajout de la sécurité : les acteurs veulent-ils commercer ? */
            if (prices[i][j] > 0 && prices[j][i] > 0) {
                p_a = prices[i][j];
                p_b_hat = ECO_REAL_SCALE * ECO_REAL_SCALE / prices[j][i];

                lambda = weight(p_a, p_b_hat, weight_context);

                price_matrix[i][j] = (lambda * p_a  + (ECO_REAL_SCALE - lambda) * p_b_hat ) / ECO_REAL_SCALE;
                price_matrix[j][i] = ECO_REAL_SCALE * ECO_REAL_SCALE / price_matrix[i][j];
            } else {
                price_matrix[i][j] = 0;
                price_matrix[j][i] = 0;
            }
            

        }
    }

    return price_matrix;
}

//IMPLEMENTED eco_allowed_quantities_for_pai
void eco_allowed_quantities_for_pair(
    const EcoActor *buyer,
    const EcoActor *seller,
    const EcoTactic *tactic,
    EcoReal negotiated_price,
    EcoReal *q_min,
    EcoReal *q_max
) {
    EcoIndex id_a = buyer->id;
    EcoIndex id_b = seller->id;

    if(negotiated_price <= 0) {
        *q_min = 1;
        *q_max = 0;

        return;
    }

    EcoReal delta_e_a, delta_r_a, delta_e_b, delta_r_b;
    delta_e_a = tactic->emission[id_a][id_b];
    delta_r_a = tactic->reception[id_a][id_b];
    delta_e_b = tactic->emission[id_b][id_a];
    delta_r_b = tactic->reception[id_b][id_a];

    *q_min = max(delta_r_a, delta_r_b * ECO_REAL_SCALE / negotiated_price);
    *q_max = min(delta_e_a * ECO_REAL_SCALE/ negotiated_price, delta_e_b);

    return;
}

//DONE modification de la méthode de selection des dépense dans le Latex
//IMPLEMENTED eco_unilateral_selection
EcoVector eco_unilateral_selection(
    const EcoActorSet *actors,
    const EcoActor *actor,
    const EcoVector quantities,
    const EcoVector prices,
    EcoReal *out_cost /* NOUVEAU : Paramètre d'export pour retourner c_a proprement */
) {
    EcoIndex n = actors->count;
    EcoVector selected = eco_vector_create(n);
    EcoIndex m = actor->resources[0];
    EcoVector virtual_resources = eco_vector_create(m);

    EcoReal budget = actor->resources[actor->produced_resource];
    EcoReal total_cost = 0;

    /* Initialisation des ressources virtuelles */
    for (EcoIndex i = 1; i <= m; i++) {
        virtual_resources[i] = actor->resources[i];
    }

    bool *v = malloc(sizeof(bool) * (n + 1));
    EcoIndex card_v = 0;
    for (EcoIndex i = 1; i <= n; i++) { 
        v[i] = (i != actor->id) && (quantities[i] > 0) && (prices[i] > 0);
        if (v[i]) card_v++;
    }

    bool *v_util = calloc((n + 1), sizeof(bool));
    bool *k_crit = calloc((m + 1), sizeof(bool));
    EcoReal *alpha = calloc((m + 1), sizeof(EcoReal));
    
    bool running = true;
    
    while (running && card_v > 0) {
        EcoReal alpha_max = LLONG_MAX;
        
        /* 1. Identification de l'intensité de production et du goulet d'étranglement */
        for (EcoIndex k = 1; k <= m; k++) {
            if (actor->consumption_pattern[k] > 0) { /* Sécurité : division par zéro */
                alpha[k] = (virtual_resources[k] * ECO_REAL_SCALE) / actor->consumption_pattern[k];
                if (alpha[k] < alpha_max) {
                    alpha_max = alpha[k];
                }
            } else {
                alpha[k] = LLONG_MAX; 
            }
        }

        for (EcoIndex k = 1; k <= m; k++) {
            k_crit[k] = (actor->consumption_pattern[k] > 0) && (alpha[k] == alpha_max);
        }

        /* 2. Filtrage des vendeurs utiles et accessibles */
        EcoIndex card_v_util = 0;
        for (EcoIndex i = 1; i <= n; i++) {
            EcoReal cost_of_exchange = (prices[i] * quantities[i]) / ECO_REAL_SCALE;
            v_util[i] = v[i] && k_crit[actors->actors[i]->produced_resource] && (cost_of_exchange <= budget);
            if (v_util[i]) card_v_util++;
        }

        if (card_v_util == 0) {
            running = false;
            break;
        }

        EcoReal cost_min = LLONG_MAX;
        EcoIndex selected_actor = 0;
        
        /* 3. Sélection de l'offre la moins chère */
        for (EcoIndex i = 1; i <= n; i++) {
            if (v_util[i]) {
                EcoReal cost_of_one = (prices[i] * quantities[i]) / ECO_REAL_SCALE;
                if (cost_of_one < cost_min) {
                    selected_actor = i;
                    cost_min = cost_of_one;
                }
            }
        }

        /* 4. Mise à jour des états virtuels */
        selected[selected_actor] = ECO_REAL_ONE;
        budget -= cost_min;
        total_cost += cost_min;
        if (actors->actors[selected_actor]->produced_resource > 0) {
            virtual_resources[actors->actors[selected_actor]->produced_resource] += quantities[selected_actor];
        }
        v[selected_actor] = false;
        card_v--;
    }

    if (out_cost != NULL) {
        *out_cost = total_cost;
    }

    eco_vector_destroy(virtual_resources);
    free(k_crit);
    free(v_util);
    free(alpha); /* Correction syntaxique de frre */
    free(v);

    return selected;
}

//IMPLEMENTED eco_intersect_selections
EcoAcceptanceMatrix eco_intersect_selections(
    const EcoActorSet *actors,
    const EcoSelectionMatrix selections
) {

    EcoIndex n = actors->count;
    EcoAcceptanceMatrix selected_exchanges = eco_matrix_create(n,n);
    for(EcoIndex i = 1 ; i <= n; i ++)
        for(EcoIndex j = 1 ; j <= n ; j ++) 
            selected_exchanges[i][j] = ECO_REAL_ONE*(i != j && selections[i][j] == ECO_REAL_ONE && selections[j][i] == ECO_REAL_ONE);

    return selected_exchanges;
    
}

//IMPLEMENTED eco_build_exchange_table
EcoExchangeTable *eco_build_exchange_table(
    const EcoActorSet *actors,
    const EcoQuantityMatrix quantities,
    const EcoVector gammas, /* NOUVEAU */
    const EcoPriceMatrix prices,
    const EcoAcceptanceMatrix acceptances
) {

    EcoIndex n = actors->count;
    EcoExchangeTable* exchange_table = eco_exchange_table_create(n);

    EcoReal gamma;
    EcoReal quantity;
    EcoReal cost;
    for(EcoIndex i = 1; i <= n; i++){
        exchange_table->emitted[i][i] = 0;
        exchange_table->received[i][i] = 0;
        for(EcoIndex j = i + 1 ; j <= n; j++){
            if(acceptances[i][j] == ECO_REAL_ONE) {
                gamma = gammas[i] < gammas[j] ? gammas[i] : gammas[j];
                quantity = gamma * quantities[i][j] / ECO_REAL_SCALE;
                cost = prices[i][j] * quantity / ECO_REAL_SCALE;
                exchange_table->emitted[i][j] = cost;
                exchange_table->received[i][j] = quantity;
                exchange_table->emitted[j][i] = quantity;
                exchange_table->received[j][i] = cost;
            }
        }
    }

    return exchange_table;
}

//IMPLEMENTED eco_exchange_result_destroy
void eco_exchange_result_destroy(EcoExchangeResult *result) {
    if (result == NULL) {
        return;
    }
    
    /* Libération de l'ensemble d'acteurs mis à jour */
    if (result->updated_actors != NULL) {
        eco_actor_set_destroy(result->updated_actors);
    }
    
    /* Libération du tableau d'échanges */
    if (result->exchange_table != NULL) {
        eco_exchange_table_destroy(result->exchange_table);
    }
    
    /* Libération de la structure englobante elle-même */
    free(result);
}

//IMPLEMENTED eco_update_actors_by_exchange
EcoExchangeResult *eco_update_actors_by_exchange(
    const EcoActorSet *actors,
    const EcoTactic *tactic,
    const EcoPriceMatrix prices
) {
    EcoIndex n = actors->count;
    EcoQuantityMatrix minimal_quantities = eco_matrix_create(n, n);
    EcoVector gammas_multiplier = eco_vector_create(n);

    /* Construction manuelle de will_matrix pour stocker les sélections */
    EcoVector *will_matrix = malloc(sizeof(EcoVector) * (n + 1));
    will_matrix[0] = malloc(sizeof(EcoReal));
    will_matrix[0][0] = n;

    // 1. Calcul des quantités minimum échangeables
    EcoReal q_min;
    EcoReal q_max;

    for(EcoIndex i = 1; i <= n; i++) {
        minimal_quantities[i][i] = 0; // Mis hors de la boucle j pour l'efficacité
        for(EcoIndex j = 1; j <= n; j++) {
            if(i != j) {
                eco_allowed_quantities_for_pair(
                    actors->actors[i], actors->actors[j],
                    tactic, prices[i][j],
                    &q_min, &q_max
                );
                minimal_quantities[i][j] = q_min <= q_max ? q_min : 0;
            }
        }
    }

    // 2. Sélection unilatérale et calcul du facteur multiplicateur
    EcoVector resources;
    EcoIndex produced;
    EcoReal out_cost;

    for(EcoIndex i = 1; i <= n; i ++) {
        will_matrix[i] = eco_unilateral_selection(
            actors, actors->actors[i],
            minimal_quantities[i], prices[i],
            &out_cost
        );

        if(0 < out_cost) {
            resources = actors->actors[i]->resources;
            produced = actors->actors[i]->produced_resource;
            gammas_multiplier[i] = (resources[produced] * ECO_REAL_SCALE) / out_cost;
        } else {
            gammas_multiplier[i] = 0;
        }
    }

    // 3. Intersection bilatérale des sélections
    EcoMatrix commerce = eco_intersect_selections(actors, will_matrix);

    // 4. Construction des échanges effectifs
    EcoExchangeTable* effectives_exchanges = eco_build_exchange_table(
        actors, minimal_quantities, gammas_multiplier, prices, commerce
    );

    // 5. Copie et transformation de actors_hat
    EcoActorSet* actors_hat = eco_actor_set_create(n);
    EcoReal cost, quantity_brought;
    EcoActor *a, *a_hat, *b;

    for(EcoIndex i = 1; i <= n; i++) {
        a = actors->actors[i];
        
        /* CORRECTION : Instanciation complète de l'acteur mis à jour */
        actors_hat->actors[i] = eco_actor_create(a->id);
        a_hat = actors_hat->actors[i];
        
        a_hat->produced_resource = a->produced_resource;
        a_hat->production = a->production;
        a_hat->production_context = a->production_context;

        EcoIndex nb_res = ECO_VEC_LENGTH(a->resources);
        EcoIndex nb_cons = ECO_VEC_LENGTH(a->consumption_pattern);
        
        a_hat->resources = eco_vector_create(nb_res);
        a_hat->consumption_pattern = eco_vector_create(nb_cons);

        /* Copie de l'état initial pour R_a et C_a */
        for(EcoIndex k = 1; k <= nb_res; k++) a_hat->resources[k] = a->resources[k];
        for(EcoIndex k = 1; k <= nb_cons; k++) a_hat->consumption_pattern[k] = a->consumption_pattern[k];

        /* Application des échanges */
        for(EcoIndex j = 1; j <= n; j++) {
            if(i != j) {
                b = actors->actors[j];
                cost = effectives_exchanges->emitted[i][j];
                quantity_brought = effectives_exchanges->received[i][j];
                
                if (a_hat->produced_resource > 0 && a_hat->produced_resource <= ECO_VEC_LENGTH(a_hat->resources)) {
                    a_hat->resources[a_hat->produced_resource] -= cost;
                }
                if (b->produced_resource > 0 && b->produced_resource <= ECO_VEC_LENGTH(a_hat->resources)) {
                    a_hat->resources[b->produced_resource] += quantity_brought;
                }
            }
        }
    }

    // 6. Nettoyage de la mémoire temporaire (LEAKS FIXÉS)
    eco_matrix_destroy(minimal_quantities);
    eco_vector_destroy(gammas_multiplier);
    eco_matrix_destroy(commerce);

    for(EcoIndex i = 1; i <= n; i++) {
        eco_vector_destroy(will_matrix[i]); /* Détruit chaque vecteur renvoyé par eco_unilateral_selection */
    }
    free(will_matrix[0]);
    free(will_matrix);

    // 7. Retour du résultat
    EcoExchangeResult* result = malloc(sizeof(EcoExchangeResult));
    result->updated_actors = actors_hat;
    result->exchange_table = effectives_exchanges;

    return result;
}

//IMPLEMENTED eco_update_actors_by_production
EcoActorSet *eco_update_actors_by_production(const EcoActorSet *actors) {
    /* Sécurité d'entrée */
    if (actors == NULL || actors->count == 0) {
        return NULL;
    }

    EcoIndex n = actors->count;
    EcoActorSet* actors_hat = eco_actor_set_create(n);

    for (EcoIndex i = 1; i <= n; i++) {
        /* --- VARIABLES INTERMÉDIAIRES DE LISIBILITÉ --- */
        EcoActor *a = actors->actors[i];
        
        /* 1. INSTANCIATION COMPLÈTE DU NOUVEL ACTEUR (Évite le Segfault) */
        EcoActor *a_hat = eco_actor_create(a->id);
        actors_hat->actors[i] = a_hat;
        
        a_hat->produced_resource = a->produced_resource;
        a_hat->production = a->production;
        a_hat->production_context = a->production_context; /* Ne pas oublier le contexte ! */

        EcoIndex m = ECO_VEC_LENGTH(a->resources);
        a_hat->resources = eco_vector_create(m);
        a_hat->consumption_pattern = eco_vector_create(m);

        EcoReal alpha_max = LLONG_MAX;

        /* 2. RECHERCHE DE L'INTENSITÉ MAXIMALE (Goulet d'étranglement) */
        for (EcoIndex k = 1; k <= m; k++) {
            EcoReal current_res = a->resources[k];
            EcoReal current_cons = a->consumption_pattern[k];

            /* Copie de l'état initial dans a_hat */
            a_hat->resources[k] = current_res;
            a_hat->consumption_pattern[k] = current_cons;

            /* Calcul du ratio seulement si la ressource est consommée */
            if (current_cons > 0) {
                EcoReal alpha = (current_res * ECO_REAL_SCALE) / current_cons;
                if (alpha < alpha_max) {
                    alpha_max = alpha;
                }
            }
        }

        /* Si l'acteur ne consomme rien, il ne produit rien */
        if (alpha_max == LLONG_MAX) {
            alpha_max = ECO_REAL_ZERO;
        }

        /* 3. CALCUL DE LA CONSOMMATION EFFECTIVE ET DÉDUCTION DES STOCKS */
        EcoVector actual_consumption = eco_vector_create(m);
        
        for (EcoIndex k = 1; k <= m; k++) {
            EcoReal current_cons = a->consumption_pattern[k];
            
            /* Multiplication en virgule fixe : (A * B) / SCALE */
            actual_consumption[k] = (alpha_max * current_cons) / ECO_REAL_SCALE;
            
            /* L'acteur consomme ces ressources de son nouveau stock */
            a_hat->resources[k] -= actual_consumption[k];
        }

        /* 4. PRODUCTION ET AJOUT AU STOCK */
        /* La fonction de production prend la consommation effective en paramètre */
        EcoReal prod = a->production(actual_consumption, a->production_context);
        
        if (a_hat->produced_resource > 0) {
            a_hat->resources[a_hat->produced_resource] += prod;
        }

        /* Nettoyage du vecteur temporaire */
        eco_vector_destroy(actual_consumption);
    }

    /* 5. RETOUR DE L'ENSEMBLE MIS À JOUR */
    return actors_hat;
}



EcoMarket *deep_copy(EcoMarket* market) {
    if (market == NULL) return NULL;

    EcoMarket *copy = calloc(1, sizeof(EcoMarket));
    if (copy == NULL) return NULL;

    /* 1. Fonctions globales et contextes */
    copy->update = market->update;
    copy->update_context = market->update_context;
    copy->weight = market->weight;
    copy->weight_context = market->weight_context;

    /* 2. Stratégie */
    copy->strategy.tactic_count = market->strategy.tactic_count;
    copy->strategy.tactic_id = market->strategy.tactic_id;
    copy->strategy.generator = market->strategy.generator;
    copy->strategy.generator_context = market->strategy.generator_context;

    if (market->strategy.tactic_count > 0 && market->strategy.tactics != NULL) {
        copy->strategy.tactics = calloc(market->strategy.tactic_count + 1, sizeof(EcoTactic*));
        for (EcoIndex i = 1; i <= market->strategy.tactic_count; i++) {
            if (market->strategy.tactics[i] != NULL) {
                EcoTactic *tac_orig = market->strategy.tactics[i];
                EcoIndex n = ECO_MAT_ROWS(tac_orig->emission); 
                copy->strategy.tactics[i] = eco_tactic_create(n);
                for(EcoIndex r = 1; r <= n; r++) {
                    for(EcoIndex c = 1; c <= n; c++) {
                        copy->strategy.tactics[i]->emission[r][c] = tac_orig->emission[r][c];
                        copy->strategy.tactics[i]->reception[r][c] = tac_orig->reception[r][c];
                    }
                }
            }
        }
    } else {
        copy->strategy.tactics = NULL;
    }

    /* 3. Ensemble des acteurs */
    if (market->initial_actors != NULL) {
        EcoIndex n = market->initial_actors->count;
        copy->initial_actors = eco_actor_set_create(n); 
        
        for (EcoIndex i = 1; i <= n; i++) {
            EcoActor *orig_a = market->initial_actors->actors[i];
            
            // CORRECTION MAJEURE : Instanciation de l'acteur !
            copy->initial_actors->actors[i] = eco_actor_create(orig_a->id);
            EcoActor *copy_a = copy->initial_actors->actors[i]; 
            
            copy_a->produced_resource = orig_a->produced_resource;
            copy_a->production = orig_a->production;
            copy_a->production_context = orig_a->production_context;

            EcoIndex nb_res = ECO_VEC_LENGTH(orig_a->resources);
            EcoIndex nb_cons = ECO_VEC_LENGTH(orig_a->consumption_pattern);
            
            copy_a->resources = eco_vector_create(nb_res);
            copy_a->consumption_pattern = eco_vector_create(nb_cons);
            
            for(EcoIndex k = 1; k <= nb_res; k++) {
                copy_a->resources[k] = orig_a->resources[k];
            }
            for(EcoIndex k = 1; k <= nb_cons; k++) {
                copy_a->consumption_pattern[k] = orig_a->consumption_pattern[k];
            }
        }
    } else {
        copy->initial_actors = NULL;
    }

    return copy;
}