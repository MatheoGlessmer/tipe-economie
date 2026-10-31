#ifndef MARKET_H
#define MARKET_H

#include "economie_model.h"
#include "economie_serialize.h"

/* ------------------------------------------------------------------------- */
/* Dynamique et évolution du marché                                         */
/* ------------------------------------------------------------------------- */

/**
 * Met à jour le marché complet (applique une étape de simulation complète).
 * Gère l'horloge interne, extrait la tactique courante (statique ou dynamique)
 * et applique la fonction de mise à jour globale (\mu).
 */
EcoMarket* maj(EcoMarket* market);

/* ------------------------------------------------------------------------- */
/* Fonctions de Conversion (Mapping) pour la sérialisation                  */
/* ------------------------------------------------------------------------- */

/* Couplage pour les fonctions de mise à jour globale (EcoUpdateFunction) */
int eco_market_update_to_id(EcoUpdateFunction func);
EcoUpdateFunction eco_market_id_to_update(int id);

/* Couplage pour les fonctions de pondération/négociation (EcoWeightFunction) */
int eco_market_weight_to_id(EcoWeightFunction func);
EcoWeightFunction eco_market_id_to_weight(int id);

/* Couplage pour les générateurs de stratégies (EcoStrategyGenerator) */
int eco_market_generator_to_id(EcoStrategyGenerator func);
EcoStrategyGenerator eco_market_id_to_generator(int id);

/* Couplage pour les fonctions de production des acteurs (EcoProductionFunction) */
int eco_market_production_to_id(EcoProductionFunction func);
EcoProductionFunction eco_market_id_to_production(int id);

/* ------------------------------------------------------------------------- */
/* Initialisation automatique du Mapper                                      */
/* ------------------------------------------------------------------------- */

/**
 * Remplit une structure EcoMapper avec l'ensemble des fonctions de mapping
 * définies ci-dessus. À utiliser juste avant de sérialiser/désérialiser
 * pour lier correctement tes fonctions C aux identifiants du fichier.
 */
void eco_market_get_default_mapper(EcoMapper *mapper);

#endif /* MARKET_H */