#ifndef ECO_TEST_UTILS_H
#define ECO_TEST_UTILS_H

#include "economie_model.h"

#include <stdbool.h>
#include <stddef.h>

EcoReal eco_test_real_from_ratio(int64_t numerator, int64_t denominator);
EcoReal eco_test_real_mul(EcoReal a, EcoReal b);
EcoReal eco_test_real_div(EcoReal a, EcoReal b);
EcoReal eco_test_real_abs(EcoReal x);
bool eco_test_real_close(EcoReal a, EcoReal b, EcoReal tolerance);

EcoVector eco_test_vector_new(EcoIndex length);
void eco_test_vector_free(EcoVector vector);
EcoVector eco_test_vector_clone(EcoVector vector);

EcoMatrix eco_test_matrix_new(EcoIndex rows, EcoIndex cols);
void eco_test_matrix_free(EcoMatrix matrix);

EcoActor *eco_test_actor_new(EcoActorId id,
                             EcoIndex resource_count,
                             EcoResourceId produced_resource,
                             EcoProductionFunction production,
                             void *production_context);
void eco_test_actor_free(EcoActor *actor);

EcoActorSet *eco_test_actor_set_new(EcoIndex count);
void eco_test_actor_set_free(EcoActorSet *set);

EcoTactic *eco_test_tactic_new(EcoIndex actor_count);
void eco_test_tactic_free(EcoTactic *tactic);

EcoExchangeTable *eco_test_exchange_table_new(EcoIndex actor_count);
void eco_test_exchange_table_free(EcoExchangeTable *table);

bool eco_test_vector_has_length(EcoVector vector, EcoIndex length);
bool eco_test_matrix_has_shape(EcoMatrix matrix, EcoIndex rows, EcoIndex cols);
bool eco_test_flag_matrix_is_boolean(EcoFlagMatrix matrix);
bool eco_test_matrix_is_non_negative(EcoMatrix matrix);

EcoReal eco_test_production_identity(EcoVector consumption, void *context);
EcoReal eco_test_production_linear_from_resource_1(EcoVector consumption, void *context);
EcoReal eco_test_weight_half(EcoReal p_a, EcoReal p_b_hat, void *context);

#endif /* ECO_TEST_UTILS_H */
