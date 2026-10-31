#include "eco_test_framework.h"
#include "eco_test_utils.h"
#include "eco_test_weak_symbols.h"

#include <string.h>

static EcoTestResult test_display_options_default(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_display_options_default)) {
        ECO_TEST_TODO("eco_display_options_default non implémentée");
    }

    EcoDisplayOptions options;
    eco_display_options_default(&options);

    ECO_TEST_ASSERT(options.decimals <= ECO_DISPLAY_MAX_DECIMALS || options.decimals == ECO_DISPLAY_DECIMALS_AUTO,
                    "postcondition : nombre de décimales admissible");
    ECO_TEST_ASSERT(options.max_vector_entries == ECO_DISPLAY_NO_LIMIT || options.max_vector_entries > 0,
                    "postcondition : limite vectorielle valide");
    ECO_TEST_PASS("options d'affichage par défaut vérifiées");
}

static EcoTestResult test_real_to_string_postconditions(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_real_to_string)) {
        ECO_TEST_TODO("eco_real_to_string non implémentée");
    }

    char buffer[64];
    int written = eco_real_to_string(ECO_REAL_FROM_INT(1) + eco_test_real_from_ratio(234, 1000), 3, buffer, sizeof(buffer));
    ECO_TEST_ASSERT(written > 0, "eco_real_to_string doit écrire au moins un caractère");
    ECO_TEST_ASSERT(buffer[sizeof(buffer) - 1] == '\0' || strlen(buffer) < sizeof(buffer), "chaîne terminée par nul");
    ECO_TEST_ASSERT(strcmp(buffer, "1.234") == 0, "1.234 attendu pour trois décimales");

    written = eco_real_to_string(-eco_test_real_from_ratio(5, 2), 2, buffer, sizeof(buffer));
    ECO_TEST_ASSERT(written > 0, "valeur négative affichable");
    ECO_TEST_ASSERT(strcmp(buffer, "-2.50") == 0, "-2.50 attendu pour deux décimales");

    ECO_TEST_PASS("conversion EcoReal vers chaîne vérifiée");
}

static EcoTestResult test_vector_fprint_smoke(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_vector_fprint)) {
        ECO_TEST_TODO("eco_vector_fprint non implémentée");
    }

    EcoVector v = eco_test_vector_new(2);
    ECO_TEST_ASSERT_NOT_NULL(v, "vecteur fixture");
    v[1] = ECO_REAL_FROM_INT(1);
    v[2] = eco_test_real_from_ratio(3, 2);

    EcoDisplayOptions options;
    if (!ECO_TEST_FUNCTION_MISSING(eco_display_options_default)) {
        eco_display_options_default(&options);
    } else {
        options.decimals = 2;
        options.show_indices = true;
        options.show_size_cells = false;
        options.compact = true;
        options.trailing_newline = false;
        options.indent = 0;
        options.max_vector_entries = ECO_DISPLAY_NO_LIMIT;
        options.max_matrix_rows = ECO_DISPLAY_NO_LIMIT;
        options.max_matrix_cols = ECO_DISPLAY_NO_LIMIT;
        options.actor_name = NULL;
        options.actor_name_context = NULL;
        options.resource_name = NULL;
        options.resource_name_context = NULL;
    }

    int code = eco_vector_fprint(stdout, v, &options);
    eco_test_vector_free(v);
    ECO_TEST_ASSERT(code >= 0, "eco_vector_fprint doit réussir sur stdout avec un vecteur valide");
    ECO_TEST_PASS("affichage d'un vecteur : smoke test vérifié");
}

static EcoTestResult test_matrix_row_column_smoke(void)
{
    if (ECO_TEST_FUNCTION_MISSING(eco_matrix_row_fprint) || ECO_TEST_FUNCTION_MISSING(eco_matrix_column_fprint)) {
        ECO_TEST_TODO("eco_matrix_row_fprint / eco_matrix_column_fprint non implémentées");
    }

    EcoMatrix m = eco_test_matrix_new(2, 2);
    ECO_TEST_ASSERT_NOT_NULL(m, "matrice fixture");
    ECO_MAT_AT(m, 1, 1) = ECO_REAL_FROM_INT(1);
    ECO_MAT_AT(m, 1, 2) = ECO_REAL_FROM_INT(2);
    ECO_MAT_AT(m, 2, 1) = ECO_REAL_FROM_INT(3);
    ECO_MAT_AT(m, 2, 2) = ECO_REAL_FROM_INT(4);

    EcoDisplayOptions options;
    if (!ECO_TEST_FUNCTION_MISSING(eco_display_options_default)) {
        eco_display_options_default(&options);
    } else {
        options.decimals = 0;
        options.show_indices = true;
        options.show_size_cells = false;
        options.compact = true;
        options.trailing_newline = false;
        options.indent = 0;
        options.max_vector_entries = ECO_DISPLAY_NO_LIMIT;
        options.max_matrix_rows = ECO_DISPLAY_NO_LIMIT;
        options.max_matrix_cols = ECO_DISPLAY_NO_LIMIT;
        options.actor_name = NULL;
        options.actor_name_context = NULL;
        options.resource_name = NULL;
        options.resource_name_context = NULL;
    }

    int row_code = eco_matrix_row_fprint(stdout, m, 1, &options);
    int col_code = eco_matrix_column_fprint(stdout, m, 2, &options);
    eco_test_matrix_free(m);

    ECO_TEST_ASSERT(row_code >= 0, "affichage d'une ligne valide");
    ECO_TEST_ASSERT(col_code >= 0, "affichage d'une colonne valide");
    ECO_TEST_PASS("affichage ligne/colonne : smoke test vérifié");
}

void eco_register_display_tests(EcoTestCase *tests, size_t *count)
{
    tests[(*count)++] = (EcoTestCase){"affichage", "options par défaut", test_display_options_default};
    tests[(*count)++] = (EcoTestCase){"affichage", "EcoReal vers chaîne", test_real_to_string_postconditions};
    tests[(*count)++] = (EcoTestCase){"affichage", "afficher un vecteur", test_vector_fprint_smoke};
    tests[(*count)++] = (EcoTestCase){"affichage", "afficher ligne et colonne", test_matrix_row_column_smoke};
}
