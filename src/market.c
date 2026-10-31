#include <stdlib.h>
#include "../include/economie_serialize.h"
#include "../include/economie_model.h"
#include "../include/market.h"


/* ========================================================================= */
/*
* Fonction de production et de ponderation pour les differends marcher
*/
/* ========================================================================= */

/* ========================================================================= */
/* 1. MARCHÉ CONVERGENT                                                      */
/* ========================================================================= */

// Acteur 1 : Produit R1 à partir de R2 (Ratio 1:1)
EcoReal production_convergent_a1(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 2);
}

// Acteur 2 : Produit R2 à partir de R1 (Ratio 1:1)
EcoReal production_convergent_a2(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 1);
}

/* ========================================================================= */
/* 2. MARCHÉ DIVERGENT                                                       */
/* ========================================================================= */

// Acteur 1 : Produit R1 à partir de R2 (Efficacité double, on multiplie par 2)
EcoReal production_divergent_a1(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 2) * 2;
}

// Acteur 2 : Produit R2 à partir de R1 (Efficacité double)
EcoReal production_divergent_a2(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 1) * 2;
}

/* ========================================================================= */
/* 3. MARCHÉ OSCILLANT                                                       */
/* ========================================================================= */

// Acteur 1 : Produit R1 à partir de R2 et R3
EcoReal production_oscillant_a1(const EcoVector consumption, void *context) {
    // Il consomme R2 et R3 en quantités égales. Lire R2 suffit.
    return ECO_VEC_AT(consumption, 2); 
}

// Acteur 2 : Produit R2 à partir de R1 et R3
EcoReal production_oscillant_a2(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 1);
}

// Acteur 3 : Produit R3 à partir de R1 et R2
EcoReal production_oscillant_a3(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 1);
}

// Acteur 1 : Produit R1 en consommant R2 (Ratio 1:1)
EcoReal prod_m1_a1(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 2);
}

// Acteur 2 : Produit R2 en consommant R3 (Ratio 1:1)
EcoReal prod_m1_a2(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 3);
}

// Acteur 3 : Produit R3 en consommant R1 (Ratio 1:1)
EcoReal prod_m1_a3(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 1);
}

// Acteur 4 : Produit R2 à partir de R1 (Efficacité accrue)
EcoReal prod_m2_a4(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 1) * 2; 
}

// Acteur 5 : Produit R3 à partir de R2
EcoReal prod_m2_a5(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 2);
}

// Acteur 6 : Produit R1 à partir de R3 (Régénération de la base)
EcoReal prod_m2_a6(const EcoVector consumption, void *context) {
    return ECO_VEC_AT(consumption, 3);
}

/* ========================================================================= */
/* 2. ALGORITHME 1 : MISE À JOUR GLOBALE DES ACTEURS                         */
/* Traduction exacte de l'Algorithme 1 du document mathématique              */
/* ========================================================================= */

EcoActorSet *eco_update_actors_global(const EcoActorSet *actors,
                                      const EcoTactic *tactic,
                                      EcoWeightFunction weight,
                                      void *weight_context,
                                      void *update_context) {
    if (actors == NULL || tactic == NULL) return NULL;

    /* ÉTAPE 1 : P <- Matrice Prix Avant Negociation  */
    EcoPriceMatrix P = eco_price_matrix_before_negotiation(actors, tactic);

    /* ÉTAPE 2 : Pf <- Matrice Prix Negocies  */
    EcoPriceMatrix P_f = eco_price_matrix_after_negotiation(actors, P, weight, weight_context);

    /* ÉTAPE 3 : (A_ech, chi) <- Mise A Jour Echange Des Acteurs  */
    /* Cette brique englobe le calcul des quantités, la sélection et l'intersection */
    EcoExchangeResult *ech_result = eco_update_actors_by_exchange(actors, tactic, P_f);

    /* ÉTAPE 4 : \hat{A} <- Mise A Jour Production  */
    /* Les acteurs produisent au maximum de leurs capacités avec les ressources obtenues[cite: 109]. */
    EcoActorSet *A_hat = eco_update_actors_by_production(ech_result->updated_actors);

    /* --- Nettoyage de la mémoire intermédiaire --- */
    eco_matrix_destroy(P);
    eco_matrix_destroy(P_f);
    
    if (ech_result != NULL) {
        /* L'algorithme 1.8 crée un nouvel ensemble \hat{A} à partir de A_ech[cite: 397]. 
           Il faut donc détruire l'ensemble intermédiaire A_ech pour éviter une fuite. */
        eco_exchange_result_destroy(ech_result);
    }

    /* Output: Un nouvel ensemble d'acteurs \hat{A}  */
    return A_hat;
}

/* ========================================================================= */
/* MAPPING POUR LA SÉRIALISATION DES FICHIERS .ECO                           */
/* ========================================================================= */

EcoProductionFunction eco_market_id_to_production(int id) {
    switch (id) {
        case 11: return production_convergent_a1;
        case 12: return production_convergent_a2;
        case 21: return production_divergent_a1;
        case 22: return production_divergent_a2;
        case 31: return production_oscillant_a1;
        case 32: return production_oscillant_a2;
        case 33: return production_oscillant_a3;
        case 101: return prod_m1_a1;
        case 102: return prod_m1_a2;
        case 103: return prod_m1_a3;
        case 204: return prod_m2_a4;
        case 205: return prod_m2_a5;
        case 206: return prod_m2_a6;
        default: return NULL;
    }
}

int eco_market_production_to_id(EcoProductionFunction func) {
    if (func == production_convergent_a1) return 11;
    if (func == production_convergent_a2) return 12;
    if (func == production_divergent_a1) return 21;
    if (func == production_divergent_a2) return 22;
    if (func == production_oscillant_a1) return 31;
    if (func == production_oscillant_a2) return 32;
    if (func == production_oscillant_a3) return 33;
    if (func == prod_m1_a1) return 101;
    if (func == prod_m1_a2) return 102;
    if (func == prod_m1_a3) return 103;
    if (func == prod_m2_a4) return 204;
    if (func == prod_m2_a5) return 205;
    if (func == prod_m2_a6) return 206;
    return 0; // ID inconnu
}

/* ========================================================================= */
/* FONCTION DE PONDÉRATION (POIDS) PAR DÉFAUT                                */
/* ========================================================================= */

/* Correspond à la fonction constante lambda(x,y) = 1/2 du modèle mathématique */
EcoReal eco_default_weight(EcoReal p_a, EcoReal p_b_hat, void *context) {
    return ECO_REAL_SCALE / 2;
}

/* ========================================================================= */
/* MAPPING POUR LES AUTRES FONCTIONS (UPDATE, WEIGHT, GENERATOR)             */
/* ========================================================================= */

/* --- Update (Mise à jour globale) --- */
EcoUpdateFunction eco_market_id_to_update(int id) {
    if (id == 1) return eco_update_actors_global;
    return NULL;
}

int eco_market_update_to_id(EcoUpdateFunction func) {
    if (func == eco_update_actors_global) return 1;
    return 0;
}

/* --- Weight (Pondération des prix) --- */
EcoWeightFunction eco_market_id_to_weight(int id) {
    if (id == 1) return eco_default_weight;
    return NULL;
}

int eco_market_weight_to_id(EcoWeightFunction func) {
    if (func == eco_default_weight) return 1;
    return 0;
}

/* --- Generator (Générateur dynamique de stratégie) --- */
EcoStrategyGenerator eco_market_id_to_generator(int id) {
    // Pour l'instant nous utilisons des tactiques constantes, donc aucun générateur
    return NULL; 
}

int eco_market_generator_to_id(EcoStrategyGenerator func) {
    return 0;
}

/* ========================================================================= */
/* INITIALISATION DU MAPPER                                                  */
/* ========================================================================= */

/* Cette fonction est appelée par le main.c pour configurer le désérialiseur */
void eco_market_get_default_mapper(EcoMapper *mapper) {
    if (mapper == NULL) return;
    
    mapper->update_to_id = eco_market_update_to_id;
    mapper->id_to_update = eco_market_id_to_update;
    
    mapper->weight_to_id = eco_market_weight_to_id;
    mapper->id_to_weight = eco_market_id_to_weight;
    
    mapper->generator_to_id = eco_market_generator_to_id;
    mapper->id_to_generator = eco_market_id_to_generator;
    
    mapper->production_to_id = eco_market_production_to_id;
    mapper->id_to_production = eco_market_id_to_production;
}

/* ========================================================================= */
/* 3. DYNAMIQUE DU MARCHÉ (Moteur principal)                                 */
/* ========================================================================= */

EcoMarket* maj(EcoMarket* market) {
    if (market == NULL || market->initial_actors == NULL || market->update == NULL) {
        return market; 
    }

    EcoTactic *current_tactic = NULL;
    EcoIndex current_step = 1;

    /* --- Récupération de la tactique \Delta du tour --- */
    if (market->strategy.tactic_count == 0 && market->strategy.generator != NULL) {
        current_tactic = market->strategy.generator((EcoStep)current_step, market->initial_actors, market->strategy.generator_context);
        if (current_tactic == NULL) return market; 
    } else if (market->strategy.tactic_count > 0) {
        if (current_step > market->strategy.tactic_count) {
            return market; /* Fin de la simulation (stratégie épuisée) */
        }
        current_tactic = market->strategy.tactics[current_step];
        if (current_tactic == NULL) return market;
    } else {
        return market;
    }

    /* --- Appel de la fonction de mise à jour globale (\mu) --- */
    EcoActorSet *new_actors = eco_update_actors_global(
        market->initial_actors,
        current_tactic,
        market->weight,
        market->weight_context,
        market->update_context
    );

    /* --- Actualisation de l'état du marché --- */
    if (new_actors != NULL) {
        eco_actor_set_destroy(market->initial_actors);
        market->initial_actors = new_actors;
    }

    /* --- Mise à jour de l'horloge et nettoyage --- */
    market->strategy.tactic_id = current_step;

    if (market->strategy.tactic_count == 0 && market->strategy.generator != NULL) {
        eco_tactic_destroy(current_tactic);
    }

    return market;
}

