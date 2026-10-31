#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "../include/economie_model.h"
#include "../include/economie_serialize.h"
#include "../include/economie_display.h"
#include "../include/market.h"
#include "../include/jacobien.h"

int main(int argc, char** argv) {
    // 1. Vérification des arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fichier_marche.eco>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* fichier_entree = argv[1];
    const char* fichier_sortie = "jacobien_resultat.mat";

    // 2. Initialisation du Mapper de sérialisation
    EcoMapper mapper;
    eco_market_get_default_mapper(&mapper);

    // 3. Lecture et désérialisation du marché
    printf("Ouverture du fichier %s...\n", fichier_entree);
    FILE* file_in = fopen(fichier_entree, "r");
    if (file_in == NULL) {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier d'entree.\n");
        return EXIT_FAILURE;
    }

    EcoMarket* market = eco_deserialize_market(file_in, &mapper);
    fclose(file_in);

    if (market == NULL) {
        fprintf(stderr, "Erreur : La deserialisation du marche a echoue.\n");
        return EXIT_FAILURE;
    }
    
    EcoIndex nb_acteurs = market->initial_actors->count;
    printf("Marche charge avec succes ! (%u acteurs detectes)\n", nb_acteurs);

    // 4. Calcul de la Jacobienne
    // On choisit un pas de perturbation h = 0.01 (en virgule fixe)
    EcoReal step = (EcoReal)(ECO_REAL_ONE*100); 
    
    printf("Calcul de la matrice Jacobienne (Différences finies centrées, step=0.001)...\n");
    EcoMatrix jacob = jacobien(market, step);

    if (jacob == NULL) {
        fprintf(stderr, "Erreur lors du calcul du Jacobien.\n");
        eco_market_destroy_custom(market);
        return EXIT_FAILURE;
    }

    // 5. Sauvegarde du résultat
    printf("Sauvegarde de la matrice...\n");
    FILE* file_out = fopen(fichier_sortie, "w");
    if (file_out != NULL) {
        // Utilisation de ta fonction de sérialisation de matrice !
        eco_serialize_matrix(file_out, jacob);
        fclose(file_out);
        printf("-> Jacobien sauvegarde avec succes dans : %s\n", fichier_sortie);
    } else {
        fprintf(stderr, "Erreur : Impossible de creer le fichier de sortie %s.\n", fichier_sortie);
    }

    // 6. Nettoyage de la mémoire
    eco_matrix_destroy(jacob);
    eco_market_destroy_custom(market);

    printf("Termine.\n");
    return EXIT_SUCCESS;
}