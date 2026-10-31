#ifndef ECONOMIE_SERIALIZE_H
#define ECONOMIE_SERIALIZE_H

#include <stdio.h>
#include "economie_model.h"

/* * Définition des types de fonctions de conversion (Mapping)
 * Permet d'associer un ID entier >= 1 à chaque pointeur de fonction.
 */
typedef int (*EcoUpdateToId)(EcoUpdateFunction);
typedef EcoUpdateFunction (*EcoIdToUpdate)(int);

typedef int (*EcoWeightToId)(EcoWeightFunction);
typedef EcoWeightFunction (*EcoIdToWeight)(int);

typedef int (*EcoGeneratorToId)(EcoStrategyGenerator);
typedef EcoStrategyGenerator (*EcoIdToGenerator)(int);

typedef int (*EcoProductionToId)(EcoProductionFunction);
typedef EcoProductionFunction (*EcoIdToProduction)(int);

/* Structure regroupant les fonctions de traduction pour la sérialisation */
typedef struct EcoMapper {
    EcoUpdateToId update_to_id;
    EcoIdToUpdate id_to_update;
    EcoWeightToId weight_to_id;
    EcoIdToWeight id_to_weight;
    EcoGeneratorToId generator_to_id;
    EcoIdToGenerator id_to_generator;
    EcoProductionToId production_to_id;
    EcoIdToProduction id_to_production;
} EcoMapper;

/* --- PROTOTYPES DES FONCTIONS GLOBALES (MARCHÉ) --- */

/**
 * Sérialise un marché complet dans un flux (fichier .eco).
 * Retourne 0 en cas de succès, -1 en cas d'erreur.
 */
int eco_serialize_market(FILE *stream, const EcoMarket *market, const EcoMapper *mapper);

/**
 * Désérialise un marché complet depuis un flux (fichier .eco).
 * Alloue dynamiquement le marché. Retourne le pointeur ou NULL en cas d'échec.
 */
EcoMarket *eco_deserialize_market(FILE *stream, const EcoMapper *mapper);

/**
 * Libère proprement toute la mémoire allouée à un EcoMarket.
 */
void eco_market_destroy_custom(EcoMarket *market);


/* --- PROTOTYPES POUR VECTEURS ET MATRICES (ex: Jacobien) --- */

/**
 * Sérialise une matrice (ex: Jacobien) dans un flux.
 * Retourne 0 en cas de succès, -1 en cas d'erreur.
 */
int eco_serialize_matrix(FILE *stream, const EcoMatrix matrix);

/**
 * Désérialise une matrice (ex: Jacobien) depuis un flux.
 * Alloue la matrice dynamiquement. Retourne NULL en cas d'échec.
 */
EcoMatrix eco_deserialize_matrix(FILE *stream);

/**
 * Sérialise un vecteur dans un flux.
 * Retourne 0 en cas de succès, -1 en cas d'erreur.
 */
int eco_serialize_vector(FILE *stream, const EcoVector vector);

/**
 * Désérialise un vecteur depuis un flux.
 * Alloue le vecteur dynamiquement. Retourne NULL en cas d'échec.
 */
EcoVector eco_deserialize_vector(FILE *stream);

#endif /* ECONOMIE_SERIALIZE_H */