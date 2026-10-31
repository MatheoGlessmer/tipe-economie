#include "../include/economie_model.h"
#include "../include/economie_display.h"
#include "../include/economie_serialize.h"
#include "../include/market.h"
#include "../include/ecovect_operation.h"
#include <stdlib.h>

EcoVector market_to_vect(EcoMarket* market) {
    EcoIndex n = market->initial_actors->count;
    EcoIndex m = market->initial_actors->actors[1]->resources[0];
    EcoVector vect = eco_vector_create(n * m);

    for(EcoIndex i = 0; i < n; i++){
        for(EcoIndex j = 0; j < m; j++){
            vect[1 + i*m + j] = market->initial_actors->actors[1+i]->resources[1+j];
        }
    }
    return vect;
}

EcoMatrix jacobien(EcoMarket* market, EcoReal step) {

    EcoDisplayOptions* options = malloc(sizeof(EcoDisplayOptions));
    eco_display_options_default(options);

    EcoIndex n = market->initial_actors->count;
    EcoIndex m = market->initial_actors->actors[1]->resources[0];
    EcoIndex total_dim = n * m;

    EcoMarket *market_plus, *market_minus;

    EcoVector neg_vector;
    EcoVector pos_vector;

    // ALLOCATION SÉCURISÉE : On utilise le constructeur officiel de l'API
    // Le jacobien est une matrice carrée de dimension (n*m) x (n*m)
    EcoMatrix jacob = eco_matrix_create(total_dim, total_dim);
    if (jacob == NULL) return NULL;

    neg_vector = market_to_vect(maj(market));
    for(EcoIndex i = 1; i <= n; i++){
        for(EcoIndex j = 1; j <= m; j++){

            market_plus = deep_copy(market);

            market_plus->initial_actors->actors[i]->resources[j] += step;

            maj(market_plus);

            pos_vector = market_to_vect(market_plus);

            EcoIndex row_index = (i - 1) * m + j;

            // CHAÎNE DE NETTOYAGE : 
            // 1. On détruit la ligne par défaut créée par `eco_matrix_create` pour éviter une fuite
            eco_vector_destroy(jacob[row_index]);

            // 2. On affecte le vrai vecteur calculé
            jacob[row_index] = minus(pos_vector, neg_vector);
            eco_vector_print(jacob[row_index], options);
            scal_prod(ECO_REAL_ONE*ECO_REAL_ONE / step, jacob[row_index]);

            eco_vector_print(jacob[row_index], options);

            eco_vector_destroy(pos_vector);
            
            // maj() modifiant le marché en place, détruire market_maj_plus détruit market_plus
            eco_market_destroy_custom(market_plus);

            
        }
    }
    eco_vector_destroy(neg_vector);

    return jacob;
}