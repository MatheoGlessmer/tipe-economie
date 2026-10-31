#include <stdlib.h>
#include <assert.h>
#include "ecovect_operation.h"
#include "economie_model.h"

// -----------------------------------------------------------------------------
// Addition de deux vecteurs
// -----------------------------------------------------------------------------
EcoVector plus(EcoVector e1, EcoVector e2) {
    if (!e1 || !e2) return NULL;

    // Récupération de la taille (stockée dans la case 0)
    EcoIndex len1 = (EcoIndex)e1[0];
    EcoIndex len2 = (EcoIndex)e2[0];

    // Sécurité : on s'assure que les vecteurs font la même taille
    assert(len1 == len2 && "Erreur : Addition de vecteurs de tailles differentes");

    // Création du vecteur résultat
    EcoVector res = eco_vector_create(len1);
    if (!res) return NULL;

    // Calcul de la somme (l'addition en virgule fixe est directe)
    for (EcoIndex i = 1; i <= len1; i++) {
        res[i] = e1[i] + e2[i];
    }

    return res;
}

// -----------------------------------------------------------------------------
// Soustraction de deux vecteurs (e1 - e2)
// -----------------------------------------------------------------------------
EcoVector minus(EcoVector e1, EcoVector e2) {
    if (!e1 || !e2) return NULL;

    EcoIndex len1 = (EcoIndex)e1[0];
    EcoIndex len2 = (EcoIndex)e2[0];

    assert(len1 == len2 && "Erreur : Soustraction de vecteurs de tailles differentes");

    EcoVector res = eco_vector_create(len1);
    if (!res) return NULL;

    // Calcul de la différence
    for (EcoIndex i = 1; i <= len1; i++) {
        res[i] = e1[i] - e2[i];
    }

    return res;
}

// -----------------------------------------------------------------------------
// Produit scalaire usuel
// -----------------------------------------------------------------------------
EcoReal dot_prod(EcoVector e1, EcoVector e2) {
    if (!e1 || !e2) return ECO_REAL_ZERO;

    EcoIndex len1 = (EcoIndex)e1[0];
    EcoIndex len2 = (EcoIndex)e2[0];

    assert(len1 == len2 && "Erreur : Produit scalaire de tailles differentes");

    EcoReal sum = ECO_REAL_ZERO;

    for (EcoIndex i = 1; i <= len1; i++) {
        /*
         * MULTIPLICATION EN VIRGULE FIXE :
         * (A * SCALE) * (B * SCALE) = (A * B) * SCALE^2
         * Il faut diviser par SCALE pour retomber sur (A * B) * SCALE.
         * * Note technique : On cast en __int128_t pour l'étape de multiplication
         * afin d'éviter un overflow (dépassement de capacité) si les deux 
         * int64_t sont très grands avant la division. (Fonctionne sous GCC/Clang).
         */
        sum += (EcoReal)( ((__int128_t)e1[i] * (__int128_t)e2[i]) / ECO_REAL_SCALE );
    }

    return sum;
}

// -----------------------------------------------------------------------------
// Multiplication d'un vecteur par un scalaire (MODIFICATION EN PLACE)
// -----------------------------------------------------------------------------
void scal_prod(EcoReal lambda, EcoVector v) {
    if (!v) return;

    EcoIndex len = (EcoIndex)v[0];

    for (EcoIndex i = 1; i <= len; i++) {
        // Même logique de virgule fixe que pour le produit scalaire
        v[i] = (EcoReal)( ((__int128_t)v[i] * (__int128_t)lambda) / ECO_REAL_SCALE );
    }
}