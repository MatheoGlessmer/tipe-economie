#include "eco_test_framework.h"
#include "eco_test_utils.h"
#include "economie_model.h"
#include "economie_display.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

void eco_register_structure_helper_tests(EcoTestCase *tests, size_t *count);
void eco_register_model_alloc_tests(EcoTestCase *tests, size_t *count);
void eco_register_model_algorithm_tests(EcoTestCase *tests, size_t *count);
void eco_register_display_tests(EcoTestCase *tests, size_t *count);

int main(int argc, char **argv)
{
    if(argc == 1) {
            bool use_color = true;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--no-color") == 0) {
            use_color = false;
        }
    }

    EcoTestCase tests[128];
    size_t count = 0;

    eco_register_structure_helper_tests(tests, &count);
    eco_register_model_alloc_tests(tests, &count);
    eco_register_model_algorithm_tests(tests, &count);
    eco_register_display_tests(tests, &count);

    EcoTestSummary summary;
    return eco_test_run_all(tests, count, use_color, &summary);

    } else if(argc == 2 && strcmp(argv[1],"myTest") == 0) {

        EcoDisplayOptions* options = malloc(sizeof(EcoDisplayOptions));
        eco_display_options_default(options);

        EcoMatrix m = eco_test_matrix_new(2,3);
        
        ECO_MAT_AT(m, 1, 1) = ECO_REAL_FROM_INT(1);
        ECO_MAT_AT(m, 1, 2) = ECO_REAL_FROM_INT(2);
        ECO_MAT_AT(m, 1, 3) = ECO_REAL_FROM_INT(3);
        ECO_MAT_AT(m, 2, 1) = ECO_REAL_FROM_INT(4);
        ECO_MAT_AT(m, 2, 2) = ECO_REAL_FROM_INT(5);
        ECO_MAT_AT(m, 2, 3) = ECO_REAL_FROM_INT(6);

        eco_matrix_print(m, options);

        free(options);

        return 0;

    } else {
        fprintf(stderr, "invalid parameter\n");
        return -1;
    }




}
