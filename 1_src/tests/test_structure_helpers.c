#include "eco_test_framework.h"
#include "eco_test_utils.h"

static EcoTestResult test_local_vector_convention(void)
{
    EcoVector v = eco_test_vector_new(3);
    ECO_TEST_ASSERT_NOT_NULL(v, "allocation du vecteur de test");
    v[1] = ECO_REAL_FROM_INT(10);
    v[2] = ECO_REAL_FROM_INT(20);
    v[3] = ECO_REAL_FROM_INT(30);

    ECO_TEST_ASSERT_EQ_INT(3u, ECO_VEC_LENGTH(v), "v[0] doit contenir la longueur");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(20), ECO_VEC_AT(v, 2), "indice utile à partir de 1");

    eco_test_vector_free(v);
    ECO_TEST_PASS("convention v[0] = longueur vérifiée");
}

static EcoTestResult test_local_matrix_convention(void)
{
    EcoMatrix m = eco_test_matrix_new(2, 3);
    ECO_TEST_ASSERT_NOT_NULL(m, "allocation de la matrice de test");
    ECO_MAT_AT(m, 2, 3) = ECO_REAL_FROM_INT(42);

    ECO_TEST_ASSERT_EQ_INT(2u, ECO_MAT_ROWS(m), "M[0][0] doit contenir le nombre de lignes");
    ECO_TEST_ASSERT_EQ_INT(3u, ECO_MAT_COLS(m), "M[i][0] doit contenir le nombre de colonnes");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(42), ECO_MAT_AT(m, 2, 3), "case utile M[2][3]");

    eco_test_matrix_free(m);
    ECO_TEST_PASS("convention matrice indexée à partir de 1 vérifiée");
}

static EcoTestResult test_local_fixed_point_arithmetic(void)
{
    EcoReal one_half = eco_test_real_from_ratio(1, 2);
    EcoReal three = ECO_REAL_FROM_INT(3);
    EcoReal expected = eco_test_real_from_ratio(3, 2);

    ECO_TEST_ASSERT_EQ_REAL(expected, eco_test_real_mul(three, one_half), "multiplication en virgule fixe");
    ECO_TEST_ASSERT_EQ_REAL(ECO_REAL_FROM_INT(6), eco_test_real_div(three, one_half), "division en virgule fixe");

    ECO_TEST_PASS("arithmétique de test en virgule fixe vérifiée");
}

void eco_register_structure_helper_tests(EcoTestCase *tests, size_t *count)
{
    tests[(*count)++] = (EcoTestCase){"structures locales", "convention des vecteurs", test_local_vector_convention};
    tests[(*count)++] = (EcoTestCase){"structures locales", "convention des matrices", test_local_matrix_convention};
    tests[(*count)++] = (EcoTestCase){"structures locales", "arithmétique virgule fixe", test_local_fixed_point_arithmetic};
}
