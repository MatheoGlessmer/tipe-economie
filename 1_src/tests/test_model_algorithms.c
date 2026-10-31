#include "eco_test_framework.h"
#include "eco_test_utils.h"
#include "eco_test_weak_symbols.h"

#include "economie_model.h"

static EcoActorSet *fixture_two_actors(void)
{
    EcoActorSet *actors = eco_test_actor_set_new(2);
    if (actors == NULL) {
        return NULL;
    }

    actors->actors[1] = eco_test_actor_new(1, 3, 1, eco_test_production_identity, NULL);
    actors->actors[2] = eco_test_actor_new(2, 3, 2, eco_test_production_identity, NULL);

    if (actors->actors[1] == NULL || actors->actors[2] == NULL) {
        eco_test_actor_set_free(actors);
        return NULL;
    }

    actors->actors[1]->resources[1] = ECO_REAL_FROM_INT(10);
    actors->actors[2]->resources[2] = ECO_REAL_FROM_INT(20);

    return actors;
}

static EcoTestResult test_price_matrix_before_negotiation(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_price_matrix_before_negotiation)) {
        ECO_TEST_TODO("eco_price_matrix_before_negotiation non implémentée");
    }

    EcoActorSet *actors = fixture_two_actors();
    EcoTactic *tactic = eco_test_tactic_new(2);
    ECO_TEST_ASSERT_NOT_NULL(actors, "fixture acteurs");
    ECO_TEST_ASSERT_NOT_NULL(tactic, "fixture tactique");

    ECO_MAT_AT(tactic->emission, 1, 2) = ECO_REAL_FROM_INT(3);
    ECO_MAT_AT(tactic->reception, 1, 2) = ECO_REAL_FROM_INT(8);
    ECO_MAT_AT(tactic->emission, 2, 1) = ECO_REAL_FROM_INT(8);
    ECO_MAT_AT(tactic->reception, 2, 1) = ECO_REAL_FROM_INT(3);

    EcoPriceMatrix prices = eco_price_matrix_before_negotiation(actors, tactic);
    ECO_TEST_ASSERT_NOT_NULL(prices, "la matrice des prix avant négociation ne doit pas être NULL");
    ECO_TEST_ASSERT(eco_test_matrix_has_shape(prices, 2, 2), "postcondition : matrice 2x2");
    ECO_TEST_ASSERT_EQ_REAL(eco_test_real_from_ratio(3, 8), ECO_MAT_AT(prices, 1, 2), "P[1][2] = delta_e / delta_r");
    ECO_TEST_ASSERT_EQ_REAL(eco_test_real_from_ratio(8, 3), ECO_MAT_AT(prices, 2, 1), "P[2][1] = delta_e / delta_r");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_ZERO, ECO_MAT_AT(prices, 1, 1), "diagonale nulle");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_ZERO, ECO_MAT_AT(prices, 2, 2), "diagonale nulle");

    if (!ECO_TEST_FUNCTION_MISSING(eco_matrix_destroy)) {
        eco_matrix_destroy(prices);
    }
    eco_test_tactic_free(tactic);
    eco_test_actor_set_free(actors);
    ECO_TEST_PASS("prix maximaux conseillés vérifiés");
}

static EcoTestResult test_price_matrix_after_negotiation(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_price_matrix_after_negotiation)) {
        ECO_TEST_TODO("eco_price_matrix_after_negotiation non implémentée");
    }

    EcoActorSet *actors = fixture_two_actors();
    EcoMatrix prices = eco_test_matrix_new(2, 2);
    ECO_TEST_ASSERT_NOT_NULL(actors, "fixture acteurs");
    ECO_TEST_ASSERT_NOT_NULL(prices, "fixture prix");

    ECO_MAT_AT(prices, 1, 2) = eco_test_real_from_ratio(3, 8); /* 0.375 */
    ECO_MAT_AT(prices, 2, 1) = ECO_REAL_FROM_INT(6);           /* inverse : 0.166666 */

    EcoPriceMatrix negotiated = eco_price_matrix_after_negotiation(actors, prices, eco_test_weight_half, NULL);
    ECO_TEST_ASSERT_NOT_NULL(negotiated, "la matrice des prix négociés ne doit pas être NULL");
    ECO_TEST_ASSERT(eco_test_matrix_has_shape(negotiated, 2, 2), "postcondition : matrice 2x2");

    EcoReal expected = (eco_test_real_from_ratio(3, 8) + eco_test_real_from_ratio(1, 6)) / 2;
    ECO_TEST_ASSERT_APPROX_REAL(expected, ECO_MAT_AT(negotiated, 1, 2), 2, "barycentre à poids 1/2");
    ECO_TEST_ASSERT_APPROX_REAL(ECO_REAL_ONE,
                                eco_test_real_mul(ECO_MAT_AT(negotiated, 1, 2), ECO_MAT_AT(negotiated, 2, 1)),
                                4,
                                "postcondition : prix inverses entre les deux sens");

    if (!ECO_TEST_FUNCTION_MISSING(eco_matrix_destroy)) {
        eco_matrix_destroy(negotiated);
    }
    eco_test_matrix_free(prices);
    eco_test_actor_set_free(actors);
    ECO_TEST_PASS("prix négociés vérifiés");
}

static EcoTestResult test_allowed_quantities_for_pair(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_allowed_quantities_for_pair)) {
        ECO_TEST_TODO("eco_allowed_quantities_for_pair non implémentée");
    }

    EcoActorSet *actors = fixture_two_actors();
    EcoTactic *tactic = eco_test_tactic_new(2);
    ECO_TEST_ASSERT_NOT_NULL(actors, "fixture acteurs");
    ECO_TEST_ASSERT_NOT_NULL(tactic, "fixture tactique");

    ECO_MAT_AT(tactic->emission, 1, 2) = ECO_REAL_FROM_INT(10);
    ECO_MAT_AT(tactic->reception, 1, 2) = ECO_REAL_FROM_INT(3);
    ECO_MAT_AT(tactic->emission, 2, 1) = ECO_REAL_FROM_INT(8);
    ECO_MAT_AT(tactic->reception, 2, 1) = ECO_REAL_FROM_INT(4);

    EcoReal q_min = 0;
    EcoReal q_max = 0;
    eco_allowed_quantities_for_pair(actors->actors[1], actors->actors[2], tactic, ECO_REAL_ONE, &q_min, &q_max);

    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(4), q_min, "q_min = max(delta_r^a, delta_r^b / p)");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(8), q_max, "q_max = min(delta_e^a / p, delta_e^b)");
    ECO_TEST_ASSERT(q_min <= q_max, "postcondition : intervalle non vide sous cette fixture");

    eco_test_tactic_free(tactic);
    eco_test_actor_set_free(actors);
    ECO_TEST_PASS("quantités permises vérifiées");
}

static EcoTestResult test_unilateral_selection_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_unilateral_selection)) {
        ECO_TEST_TODO("eco_unilateral_selection non implémentée");
    }

    EcoActorSet *actors = eco_test_actor_set_new(3);
    
    /* Les quantités et prix sont maintenant des vecteurs 1D du point de vue d'un acteur */
    EcoVector quantities = eco_vector_create(3);
    EcoVector prices = eco_vector_create(3);
    
    ECO_TEST_ASSERT_NOT_NULL(actors, "fixture acteurs");
    ECO_TEST_ASSERT_NOT_NULL(quantities, "fixture quantités");
    ECO_TEST_ASSERT_NOT_NULL(prices, "fixture prix");

    /* INITIALISATION COMPLÈTE des acteurs */
    for (EcoIndex i = 1; i <= 3; ++i) {
        actors->actors[i] = eco_actor_create(i);
        ECO_TEST_ASSERT_NOT_NULL(actors->actors[i], "acteur fixture");
        
        actors->actors[i]->produced_resource = i;
        actors->actors[i]->resources = eco_vector_create(3);
        actors->actors[i]->resources[i] = ECO_REAL_FROM_INT(10); /* Budget */
        
        actors->actors[i]->consumption_pattern = eco_vector_create(3);
        for(EcoIndex k = 1; k <= 3; ++k) {
            if (k != i) {
                ECO_VEC_AT(actors->actors[i]->consumption_pattern, k) = ECO_REAL_ONE;
                actors->actors[i]->resources[k] = ECO_REAL_FROM_INT(1); /* Stock initial */
            } else {
                ECO_VEC_AT(actors->actors[i]->consumption_pattern, k) = ECO_REAL_ZERO;
            }
        }
    }

    /* On teste la sélection du point de vue de l'acteur 1 */
    ECO_VEC_AT(quantities, 1) = ECO_REAL_ZERO;
    ECO_VEC_AT(prices, 1) = ECO_REAL_ZERO;
    
    /* Offre de l'acteur 2 */
    ECO_VEC_AT(quantities, 2) = ECO_REAL_FROM_INT(4);
    ECO_VEC_AT(prices, 2) = ECO_REAL_FROM_INT(2);
    
    /* Offre de l'acteur 3 */
    ECO_VEC_AT(quantities, 3) = ECO_REAL_FROM_INT(2);
    ECO_VEC_AT(prices, 3) = ECO_REAL_FROM_INT(4);

    EcoReal cost = 0;
    /* APPEL AVEC LA NOUVELLE SIGNATURE */
    EcoVector selection = eco_unilateral_selection(actors, actors->actors[1], quantities, prices, &cost);
    
    ECO_TEST_ASSERT_NOT_NULL(selection, "vecteur de sélection non NULL");
    ECO_TEST_ASSERT_EQ_INT(3, ECO_VEC_LENGTH(selection), "postcondition : vecteur de taille 3");

    /* VÉRIFICATION DES POSTCONDITIONS */
    EcoReal manual_cost = 0;
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_ZERO, ECO_VEC_AT(selection, 1), "postcondition : auto-sélection nulle");
    
    for (EcoIndex b = 1; b <= 3; ++b) {
        EcoReal s = ECO_VEC_AT(selection, b);
        ECO_TEST_ASSERT(s == ECO_REAL_ZERO || s == ECO_REAL_ONE, "postcondition : valeurs booléennes 0 ou 1");
        
        if (s == ECO_REAL_ONE) {
            ECO_TEST_ASSERT(ECO_VEC_AT(quantities, b) > 0, "quantité > 0 si sélectionné");
            ECO_TEST_ASSERT(ECO_VEC_AT(prices, b) > 0, "prix > 0 si sélectionné");
            manual_cost += eco_test_real_mul(ECO_VEC_AT(quantities, b), ECO_VEC_AT(prices, b));
        }
    }
    
    ECO_TEST_ASSERT_EQ_REAL(manual_cost, cost, "postcondition : calcul du coût exact correctement exporté");
    ECO_TEST_ASSERT(cost <= actors->actors[1]->resources[actors->actors[1]->produced_resource], "postcondition : budget non dépassé");

    /* NETTOYAGE */
    eco_vector_destroy(selection);
    eco_vector_destroy(prices);
    eco_vector_destroy(quantities);
    

    eco_test_actor_set_free(actors);
    
    ECO_TEST_PASS("postconditions de sélection unilatérale vérifiées");
}

static EcoTestResult test_intersect_selections_exact(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_intersect_selections)) {
        ECO_TEST_TODO("eco_intersect_selections non implémentée");
    }

    EcoActorSet *actors = eco_test_actor_set_new(2);
    EcoMatrix selections = eco_test_matrix_new(2, 2);
    ECO_TEST_ASSERT_NOT_NULL(actors, "fixture acteurs");
    ECO_TEST_ASSERT_NOT_NULL(selections, "fixture sélections");

    /* Remplacement de fixture_two_actors() par une initialisation propre */
    for (EcoIndex i = 1; i <= 2; ++i) {
        actors->actors[i] = eco_actor_create(i);
        /* L'Algo 1.5 n'a pas besoin de R_a ou C_a car il ne fait qu'une intersection booléenne, 
         * mais on initialise pour éviter tout problème de framework sous-jacent. */
    }

    ECO_MAT_AT(selections, 1, 2) = ECO_REAL_ONE;
    ECO_MAT_AT(selections, 2, 1) = ECO_REAL_ZERO;

    EcoAcceptanceMatrix acceptances = eco_intersect_selections(actors, selections);
    ECO_TEST_ASSERT_NOT_NULL(acceptances, "matrice d'acceptation non NULL");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_ZERO, ECO_MAT_AT(acceptances, 1, 2), "pas d'acceptation sans réciprocité");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_ZERO, ECO_MAT_AT(acceptances, 2, 1), "symétrie");

    ECO_MAT_AT(selections, 2, 1) = ECO_REAL_ONE;
    if (!ECO_TEST_FUNCTION_MISSING(eco_matrix_destroy)) {
        eco_matrix_destroy(acceptances);
    }
    acceptances = eco_intersect_selections(actors, selections);
    ECO_TEST_ASSERT_NOT_NULL(acceptances, "matrice d'acceptation non NULL");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_ONE, ECO_MAT_AT(acceptances, 1, 2), "acceptation mutuelle valide");

    /* NETTOYAGE */
    if (!ECO_TEST_FUNCTION_MISSING(eco_matrix_destroy)) {
        eco_matrix_destroy(acceptances);
    }

    eco_test_actor_set_free(actors);
    ECO_TEST_PASS("intersection des volontés de commerce vérifiée");
}

static EcoTestResult test_build_exchange_table_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_build_exchange_table)) {
        ECO_TEST_TODO("eco_build_exchange_table non implémentée");
    }

    EcoActorSet *actors = eco_test_actor_set_new(2);
    EcoMatrix quantities = eco_test_matrix_new(2, 2);
    EcoVector gammas = eco_vector_create(2); /* VECTEUR DES FACTEURS */
    EcoMatrix prices = eco_test_matrix_new(2, 2);
    EcoMatrix acceptances = eco_test_matrix_new(2, 2);
    
    ECO_TEST_ASSERT_NOT_NULL(actors, "fixture acteurs");
    ECO_TEST_ASSERT_NOT_NULL(quantities, "fixture quantités");
    ECO_TEST_ASSERT_NOT_NULL(gammas, "fixture gammas");
    ECO_TEST_ASSERT_NOT_NULL(prices, "fixture prix");
    ECO_TEST_ASSERT_NOT_NULL(acceptances, "fixture acceptations");

    /* Remplacement de fixture_two_actors() par une initialisation propre */
    for (EcoIndex i = 1; i <= 2; ++i) {
        actors->actors[i] = eco_actor_create(i);
        actors->actors[i]->produced_resource = i;
    }

    /* Paramétrage de l'échange de base */
    ECO_MAT_AT(quantities, 1, 2) = ECO_REAL_FROM_INT(5);
    ECO_MAT_AT(prices, 1, 2) = ECO_REAL_FROM_INT(2);
    ECO_MAT_AT(acceptances, 1, 2) = ECO_REAL_ONE;
    ECO_MAT_AT(acceptances, 2, 1) = ECO_REAL_ONE;

    /* FACTEURS GAMMA : L'acteur 1 a 3, l'acteur 2 a 2. Le min sera 2. */
    ECO_VEC_AT(gammas, 1) = ECO_REAL_FROM_INT(3);
    ECO_VEC_AT(gammas, 2) = ECO_REAL_FROM_INT(2);

    /* APPEL AVEC NOUVELLE SIGNATURE */
    EcoExchangeTable *table = eco_build_exchange_table(actors, quantities, gammas, prices, acceptances);
    ECO_TEST_ASSERT_NOT_NULL(table, "tableau d'échanges non NULL");
    
    /* VÉRIFICATION DU RÉSULTAT
     * min(3, 2) = 2
     * q_effectif = 2 * 5 = 10
     * m_effectif = 2 (prix) * 10 = 20
     */
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(20), ECO_MAT_AT(table->emitted, 1, 2), "m effectif = P * min(gamma_a, gamma_b) * Q");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(10), ECO_MAT_AT(table->received, 1, 2), "q effectif = min(gamma_a, gamma_b) * Q");

    /* VÉRIFICATION DE LA SYMÉTRIE */
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(10), ECO_MAT_AT(table->emitted, 2, 1), "symétrie émission/réception");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(20), ECO_MAT_AT(table->received, 2, 1), "symétrie réception/émission");

    /* NETTOYAGE */
    if (!ECO_TEST_FUNCTION_MISSING(eco_exchange_table_destroy)) {
        eco_exchange_table_destroy(table);
    }
    eco_vector_destroy(gammas);
    eco_test_matrix_free(acceptances);
    eco_test_matrix_free(prices);
    eco_test_matrix_free(quantities);

    eco_test_actor_set_free(actors);
    ECO_TEST_PASS("construction du tableau d'échanges avec facteur multiplicateur vérifiée");
}

static EcoTestResult test_update_by_exchange_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_update_actors_by_exchange)) {
        ECO_TEST_TODO("eco_update_actors_by_exchange non implémentée");
    }

    /* 1. INITIALISATION DU MARCHÉ VIA L'API STANDARD DU HEADER */
    EcoActorSet *actors = eco_actor_set_create(2);
    ECO_TEST_ASSERT_NOT_NULL(actors, "allocation ensemble d'acteurs");

    for (EcoIndex i = 1; i <= 2; ++i) {
        actors->actors[i] = eco_actor_create(i);
        ECO_TEST_ASSERT_NOT_NULL(actors->actors[i], "allocation acteur");
        
        actors->actors[i]->produced_resource = i;
        
        /* Allocation manuelle des vecteurs comme attendu par le modèle */
        actors->actors[i]->resources = eco_vector_create(2);
        actors->actors[i]->consumption_pattern = eco_vector_create(2);
    }

    /* Acteur 1 : possède 10 unités de Res 1, veut consommer de la Res 2 */
    ECO_VEC_AT(actors->actors[1]->resources, 1) = ECO_REAL_FROM_INT(10); 
    ECO_VEC_AT(actors->actors[1]->resources, 2) = ECO_REAL_ZERO;
    ECO_VEC_AT(actors->actors[1]->consumption_pattern, 1) = ECO_REAL_ZERO;
    ECO_VEC_AT(actors->actors[1]->consumption_pattern, 2) = ECO_REAL_ONE;

    /* Acteur 2 : possède 10 unités de Res 2, veut consommer de la Res 1 */
    ECO_VEC_AT(actors->actors[2]->resources, 1) = ECO_REAL_ZERO;
    ECO_VEC_AT(actors->actors[2]->resources, 2) = ECO_REAL_FROM_INT(10); 
    ECO_VEC_AT(actors->actors[2]->consumption_pattern, 1) = ECO_REAL_ONE;
    ECO_VEC_AT(actors->actors[2]->consumption_pattern, 2) = ECO_REAL_ZERO;

    /* 2. PRÉPARATION DE LA TACTIQUE ET DES PRIX VIA LES STRUCTURES OFFICIELLES */
    EcoTactic *tactic = eco_tactic_create(2);
    ECO_TEST_ASSERT_NOT_NULL(tactic, "allocation tactique");
    
    /* A1 veut donner max 5 unités pour recevoir min 2 unités */
    ECO_MAT_AT(tactic->emission, 1, 2) = ECO_REAL_FROM_INT(5);
    ECO_MAT_AT(tactic->reception, 1, 2) = ECO_REAL_FROM_INT(2);
    
    /* A2 veut donner max 5 unités pour recevoir min 2 unités */
    ECO_MAT_AT(tactic->emission, 2, 1) = ECO_REAL_FROM_INT(5);
    ECO_MAT_AT(tactic->reception, 2, 1) = ECO_REAL_FROM_INT(2);

    EcoPriceMatrix prices = eco_matrix_create(2, 2);
    ECO_TEST_ASSERT_NOT_NULL(prices, "allocation matrice prix");
    
    /* Prix négocié de 1 pour 1 (l'inverse de 1 est 1, la symétrie est respectée) */
    ECO_MAT_AT(prices, 1, 2) = ECO_REAL_ONE;
    ECO_MAT_AT(prices, 2, 1) = ECO_REAL_ONE; 

    /* 3. EXÉCUTION DE LA BRIQUE 7 */
    EcoExchangeResult *result = eco_update_actors_by_exchange(actors, tactic, prices);
    
    ECO_TEST_ASSERT_NOT_NULL(result, "le résultat de l'échange ne doit pas être NULL");
    ECO_TEST_ASSERT_NOT_NULL(result->updated_actors, "l'ensemble des acteurs mis à jour ne doit pas être NULL");
    ECO_TEST_ASSERT_NOT_NULL(result->exchange_table, "le tableau d'échanges ne doit pas être NULL");

    /* 4. VÉRIFICATION DES POSTCONDITIONS MATHÉMATIQUES */
    /* Logique attendue : Q_base = 2. Coût_base = 2*1 = 2. Gamma = 10/2 = 5. Q_eff = 5*2 = 10. */
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(10), ECO_MAT_AT(result->exchange_table->emitted, 1, 2), "A1 émet 10 unités");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(10), ECO_MAT_AT(result->exchange_table->received, 1, 2), "A1 reçoit 10 unités");

    /* Vérification de la mise à jour effective des stocks des nouveaux acteurs */
    EcoActor *a1_new = result->updated_actors->actors[1];
    EcoActor *a2_new = result->updated_actors->actors[2];
    
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_ZERO, ECO_VEC_AT(a1_new->resources, 1), "A1 a dépensé tout son budget");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(10), ECO_VEC_AT(a1_new->resources, 2), "A1 a stocké les ressources achetées");
    
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(10), ECO_VEC_AT(a2_new->resources, 1), "A2 a stocké les ressources achetées");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_ZERO, ECO_VEC_AT(a2_new->resources, 2), "A2 a dépensé tout son budget");

    /* 5. NETTOYAGE MÉMOIRE CONFORME À L'API STANDARD */
    if (!ECO_TEST_FUNCTION_MISSING(eco_exchange_result_destroy)) {
        eco_exchange_result_destroy(result);
    }
    
    eco_matrix_destroy(prices);
    eco_tactic_destroy(tactic);
    
    /* * En utilisant eco_actor_set_destroy() défini dans ton header,
     * la libération des vecteurs internes de l'acteur (resources, consumption_pattern) 
     * est censée être gérée proprement par l'implémentation de ton modèle.
     */
    eco_actor_set_destroy(actors);

    ECO_TEST_PASS("mise à jour globale par échange vérifiée avec succès");
}

static EcoTestResult test_update_by_production_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_update_actors_by_production)) {
        ECO_TEST_TODO("eco_update_actors_by_production non implémentée");
    }

    EcoReal coefficient = ECO_REAL_FROM_INT(5);
    EcoActorSet *actors = eco_test_actor_set_new(1);
    ECO_TEST_ASSERT_NOT_NULL(actors, "fixture acteurs");
    
    actors->actors[1] = eco_test_actor_new(1, 3, 3, eco_test_production_linear_from_resource_1, &coefficient);
    ECO_TEST_ASSERT_NOT_NULL(actors->actors[1], "acteur fixture");

    /* Initialisation mémoire des vecteurs */
    actors->actors[1]->resources = eco_vector_create(3);
    actors->actors[1]->consumption_pattern = eco_vector_create(3);

    actors->actors[1]->resources[1] = ECO_REAL_FROM_INT(10);
    actors->actors[1]->resources[2] = ECO_REAL_FROM_INT(4);
    actors->actors[1]->resources[3] = ECO_REAL_ZERO;
    
    actors->actors[1]->consumption_pattern[1] = ECO_REAL_FROM_INT(2);
    actors->actors[1]->consumption_pattern[2] = ECO_REAL_FROM_INT(1);
    actors->actors[1]->consumption_pattern[3] = ECO_REAL_ZERO;

    EcoActorSet *updated = eco_update_actors_by_production(actors);
    
    ECO_TEST_ASSERT_NOT_NULL(updated, "ensemble produit non NULL");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(2), updated->actors[1]->resources[1], "alpha_max = 4 donc consommation de 8 unités de ressource 1");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_ZERO, updated->actors[1]->resources[2], "alpha_max = 4 donc consommation de 4 unités de ressource 2");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(40), updated->actors[1]->resources[3], "production linéaire sur consommation[1] : 5*8 = 40");

    if (!ECO_TEST_FUNCTION_MISSING(eco_actor_set_destroy)) {
        eco_actor_set_destroy(updated);
    }
    
    /* Libération de la mémoire locale instanciée */
    eco_vector_destroy(actors->actors[1]->resources);
    eco_vector_destroy(actors->actors[1]->consumption_pattern);
    eco_test_actor_set_free(actors);
    
    ECO_TEST_PASS("mise à jour par production vérifiée");
}



/* Fonctions de production bouchons (mocks) pour contrôler le test à 100% */
static EcoReal mock_production_a1(const EcoVector consumption, void *context) {
    (void)context;
    /* Produit 2 unités pour chaque unité de ressource 2 consommée */
    return consumption[2] * 2; 
}

static EcoReal mock_production_a2(const EcoVector consumption, void *context) {
    (void)context;
    /* Produit 3 unités pour chaque unité de ressource 1 consommée */
    return consumption[1] * 3; 
}

static EcoTestResult test_global_update_exact_match(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_update_actors_global)) {
        ECO_TEST_TODO("eco_update_actors_global non implémentée");
    }

    /* 1. INITIALISATION DU MARCHÉ (A_initial) */
    EcoActorSet *actors = eco_actor_set_create(2);
    ECO_TEST_ASSERT_NOT_NULL(actors, "allocation ensemble d'acteurs");

    for (EcoIndex i = 1; i <= 2; ++i) {
        actors->actors[i] = eco_actor_create(i);
        actors->actors[i]->produced_resource = i;
        actors->actors[i]->resources = eco_vector_create(2);
        actors->actors[i]->consumption_pattern = eco_vector_create(2);
    }

    /* A1 : budget 10 de R1, consomme R2 */
    ECO_VEC_AT(actors->actors[1]->resources, 1) = ECO_REAL_FROM_INT(10);
    ECO_VEC_AT(actors->actors[1]->resources, 2) = ECO_REAL_ZERO;
    ECO_VEC_AT(actors->actors[1]->consumption_pattern, 1) = ECO_REAL_ZERO;
    ECO_VEC_AT(actors->actors[1]->consumption_pattern, 2) = ECO_REAL_ONE;
    actors->actors[1]->production = mock_production_a1;

    /* A2 : budget 10 de R2, consomme R1 */
    ECO_VEC_AT(actors->actors[2]->resources, 1) = ECO_REAL_ZERO;
    ECO_VEC_AT(actors->actors[2]->resources, 2) = ECO_REAL_FROM_INT(10);
    ECO_VEC_AT(actors->actors[2]->consumption_pattern, 1) = ECO_REAL_ONE;
    ECO_VEC_AT(actors->actors[2]->consumption_pattern, 2) = ECO_REAL_ZERO;
    actors->actors[2]->production = mock_production_a2;

    /* 2. PRÉPARATION DE LA TACTIQUE (Delta) */
    /* Émission 2, Réception 2 -> Prix 1, Base 2, Budget 10 -> Gamma 5 -> Échange total = 10 */
    EcoTactic *tactic = eco_tactic_create(2);
    ECO_MAT_AT(tactic->emission, 1, 2) = ECO_REAL_FROM_INT(2);
    ECO_MAT_AT(tactic->reception, 1, 2) = ECO_REAL_FROM_INT(2);
    ECO_MAT_AT(tactic->emission, 2, 1) = ECO_REAL_FROM_INT(2);
    ECO_MAT_AT(tactic->reception, 2, 1) = ECO_REAL_FROM_INT(2);

    /* 3. CRÉATION MANUELLE DE L'ÉTAT ATTENDU (A_hat attendu) */
    EcoActorSet *expected_a_hat = eco_actor_set_create(2);
    for (EcoIndex i = 1; i <= 2; ++i) {
        expected_a_hat->actors[i] = eco_actor_create(i);
        expected_a_hat->actors[i]->resources = eco_vector_create(2);
    }
    
    /* État de A1 après l'échange (R1=0, R2=10) puis la production (R2 consommée, R1 produit 10*2=20) */
    ECO_VEC_AT(expected_a_hat->actors[1]->resources, 1) = ECO_REAL_FROM_INT(20);
    ECO_VEC_AT(expected_a_hat->actors[1]->resources, 2) = ECO_REAL_ZERO;

    /* État de A2 après l'échange (R1=10, R2=0) puis la production (R1 consommée, R2 produit 10*3=30) */
    ECO_VEC_AT(expected_a_hat->actors[2]->resources, 1) = ECO_REAL_ZERO;
    ECO_VEC_AT(expected_a_hat->actors[2]->resources, 2) = ECO_REAL_FROM_INT(30);

    /* 4. EXÉCUTION DE LA FONCTION GLOBALE (A_hat calculé) */
    /* On suppose que eco_test_weight_half est défini dans ton framework (pondération 50/50) */
    EcoActorSet *actual_a_hat = eco_update_actors_global(actors, tactic, eco_test_weight_half, NULL, NULL);

    ECO_TEST_ASSERT_NOT_NULL(actual_a_hat, "fonction globale : résultat non NULL");
    ECO_TEST_ASSERT_EQ_INT(expected_a_hat->count, actual_a_hat->count, "fonction globale : même nombre d'acteurs");

    /* 5. TEST STRICT D'ÉGALITÉ ENTRE L'ÉTAT MANUEL ET L'ÉTAT CALCULÉ */
    for (EcoIndex i = 1; i <= 2; ++i) {
        EcoVector expected_res = expected_a_hat->actors[i]->resources;
        EcoVector actual_res = actual_a_hat->actors[i]->resources;
        
        ECO_TEST_ASSERT_EQ_INT(ECO_VEC_LENGTH(expected_res), ECO_VEC_LENGTH(actual_res), "même taille de vecteur de ressources");
        
        for (EcoIndex k = 1; k <= ECO_VEC_LENGTH(expected_res); ++k) {
            ECO_TEST_ASSERT_EQ_REAL(
                ECO_VEC_AT(expected_res, k), 
                ECO_VEC_AT(actual_res, k), 
                "égalité stricte des ressources après un cycle global complet (échange + production)"
            );
        }
    }

    /* 6. NETTOYAGE MÉMOIRE */
    /* Destruction des vecteurs ressources de expected_a_hat */
    for (EcoIndex i = 1; i <= 2; ++i) {
        eco_vector_destroy(expected_a_hat->actors[i]->resources);
    }
    eco_actor_set_destroy(expected_a_hat);
    
    if (!ECO_TEST_FUNCTION_MISSING(eco_actor_set_destroy)) {
        eco_actor_set_destroy(actual_a_hat);
    }
    
    eco_tactic_destroy(tactic);
    eco_actor_set_destroy(actors);

    ECO_TEST_PASS("test de la fonction globale vérifié par comparaison stricte d'état (A_hat)");
}

void eco_register_model_algorithm_tests(EcoTestCase *tests, size_t *count)
{
    tests[(*count)++] = (EcoTestCase){"algorithmes modèle", "matrice prix avant négociation", test_price_matrix_before_negotiation};
    tests[(*count)++] = (EcoTestCase){"algorithmes modèle", "matrice prix négociés", test_price_matrix_after_negotiation};
    tests[(*count)++] = (EcoTestCase){"algorithmes modèle", "quantités permises", test_allowed_quantities_for_pair};
    tests[(*count)++] = (EcoTestCase){"algorithmes modèle", "sélection unilatérale", test_unilateral_selection_postconditions};
    tests[(*count)++] = (EcoTestCase){"algorithmes modèle", "intersection des sélections", test_intersect_selections_exact};
    tests[(*count)++] = (EcoTestCase){"algorithmes modèle", "construction des échanges", test_build_exchange_table_postconditions};
    tests[(*count)++] = (EcoTestCase){"algorithmes modèle", "mise à jour par échange", test_update_by_exchange_postconditions};
    tests[(*count)++] = (EcoTestCase){"algorithmes modèle", "mise à jour par production", test_update_by_production_postconditions};
    tests[(*count)++] = (EcoTestCase){"algorithme modèle", "mise à jour globale", test_global_update_exact_match};
}
