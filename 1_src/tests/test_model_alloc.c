#include "eco_test_framework.h"
#include "eco_test_utils.h"
#include "eco_test_weak_symbols.h"

static EcoTestResult test_vector_create_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_vector_create) || ECO_TEST_FUNCTION_MISSING(eco_vector_destroy)) {
        ECO_TEST_TODO("eco_vector_create / eco_vector_destroy non implémentées");
    }

    EcoVector v = eco_vector_create(4);
    ECO_TEST_ASSERT_NOT_NULL(v, "eco_vector_create doit renvoyer un vecteur non NULL sous précondition valide");
    ECO_TEST_ASSERT_EQ_INT(4u, ECO_VEC_LENGTH(v), "postcondition : v[0] vaut la longueur demandée");
    for (EcoIndex i = 1; i <= 4; ++i) {
        ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_ZERO, v[i], "postcondition attendue : composantes initialisées à 0");
    }
    eco_vector_destroy(v);
    ECO_TEST_PASS("postconditions de création d'un vecteur vérifiées");
}

static EcoTestResult test_matrix_create_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_matrix_create) || ECO_TEST_FUNCTION_MISSING(eco_matrix_destroy)) {
        ECO_TEST_TODO("eco_matrix_create / eco_matrix_destroy non implémentées");
    }

    EcoMatrix m = eco_matrix_create(3, 2);
    ECO_TEST_ASSERT_NOT_NULL(m, "eco_matrix_create doit renvoyer une matrice non NULL sous précondition valide");
    ECO_TEST_ASSERT(eco_test_matrix_has_shape(m, 3, 2), "postcondition : dimensions stockées dans M[0][0] et M[i][0]");
    ECO_TEST_ASSERT(eco_test_matrix_is_non_negative(m), "postcondition attendue : matrice initialisée avec des valeurs non négatives");
    eco_matrix_destroy(m);
    ECO_TEST_PASS("postconditions de création d'une matrice vérifiées");
}

static EcoTestResult test_actor_create_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_actor_create) || ECO_TEST_FUNCTION_MISSING(eco_actor_destroy)) {
        ECO_TEST_TODO("eco_actor_create / eco_actor_destroy non implémentées");
    }

    EcoVector resources = eco_test_vector_new(3);
    EcoVector pattern = eco_test_vector_new(3);
    ECO_TEST_ASSERT_NOT_NULL(resources, "ressources de fixture");
    ECO_TEST_ASSERT_NOT_NULL(pattern, "patron de fixture");

    EcoActor *actor = eco_actor_create(7u);
    ECO_TEST_ASSERT_NOT_NULL(actor, "eco_actor_create doit renvoyer un acteur non NULL sous précondition valide");
    ECO_TEST_ASSERT_EQ_INT(7u, actor->id, "postcondition : identifiant conservé");

    eco_actor_destroy(actor);
    ECO_TEST_PASS("postconditions de création d'un acteur vérifiées");
}

static EcoTestResult test_actor_set_create_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_actor_set_create) || ECO_TEST_FUNCTION_MISSING(eco_actor_set_destroy)) {
        ECO_TEST_TODO("eco_actor_set_create / eco_actor_set_destroy non implémentées");
    }

    EcoActorSet *set = eco_actor_set_create(2);
    ECO_TEST_ASSERT_NOT_NULL(set, "eco_actor_set_create doit renvoyer un ensemble non NULL");
    ECO_TEST_ASSERT_EQ_INT(2u, set->count, "postcondition : nombre d'acteurs conservé");
    ECO_TEST_ASSERT_NOT_NULL(set->actors, "postcondition : tableau d'acteurs alloué");
    ECO_TEST_ASSERT_NULL(set->actors[0], "convention : actors[0] inutilisé et nul");
    eco_actor_set_destroy(set);
    ECO_TEST_PASS("postconditions de création d'un ensemble d'acteurs vérifiées");
}

static EcoTestResult test_tactic_create_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_tactic_create) || ECO_TEST_FUNCTION_MISSING(eco_tactic_destroy)) {
        ECO_TEST_TODO("eco_tactic_create / eco_tactic_destroy non implémentées");
    }

    EcoTactic *tactic = eco_tactic_create(3);
    ECO_TEST_ASSERT_NOT_NULL(tactic, "eco_tactic_create doit renvoyer une tactique non NULL");
    ECO_TEST_ASSERT(eco_test_matrix_has_shape(tactic->emission, 3, 3), "postcondition : matrice emission carrée");
    ECO_TEST_ASSERT(eco_test_matrix_has_shape(tactic->reception, 3, 3), "postcondition : matrice reception carrée");
    eco_tactic_destroy(tactic);
    ECO_TEST_PASS("postconditions de création d'une tactique vérifiées");
}

static EcoTestResult test_exchange_table_create_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_exchange_table_create) || ECO_TEST_FUNCTION_MISSING(eco_exchange_table_destroy)) {
        ECO_TEST_TODO("eco_exchange_table_create / eco_exchange_table_destroy non implémentées");
    }

    EcoExchangeTable *table = eco_exchange_table_create(2);
    ECO_TEST_ASSERT_NOT_NULL(table, "eco_exchange_table_create doit renvoyer une table non NULL");
    ECO_TEST_ASSERT(eco_test_matrix_has_shape(table->emitted, 2, 2), "postcondition : matrice emitted carrée");
    ECO_TEST_ASSERT(eco_test_matrix_has_shape(table->received, 2, 2), "postcondition : matrice received carrée");
    eco_exchange_table_destroy(table);
    ECO_TEST_PASS("postconditions de création d'un tableau d'échanges vérifiées");
}

void eco_register_model_alloc_tests(EcoTestCase *tests, size_t *count)
{
    tests[(*count)++] = (EcoTestCase){"allocation modèle", "eco_vector_create", test_vector_create_postconditions};
    tests[(*count)++] = (EcoTestCase){"allocation modèle", "eco_matrix_create", test_matrix_create_postconditions};
    tests[(*count)++] = (EcoTestCase){"allocation modèle", "eco_actor_create", test_actor_create_postconditions};
    tests[(*count)++] = (EcoTestCase){"allocation modèle", "eco_actor_set_create", test_actor_set_create_postconditions};
    tests[(*count)++] = (EcoTestCase){"allocation modèle", "eco_tactic_create", test_tactic_create_postconditions};
    tests[(*count)++] = (EcoTestCase){"allocation modèle", "eco_exchange_table_create", test_exchange_table_create_postconditions};
}
