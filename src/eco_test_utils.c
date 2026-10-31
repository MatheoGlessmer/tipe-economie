#include "eco_test_utils.h"

#include <stdlib.h>
#include <string.h>

EcoReal eco_test_real_from_ratio(int64_t numerator, int64_t denominator)
{
    if (denominator == 0) {
        return 0;
    }
return (EcoReal)((numerator * ECO_REAL_SCALE) / denominator);
}

EcoReal eco_test_real_mul(EcoReal a, EcoReal b)
{
return (EcoReal)((a * b) / ECO_REAL_SCALE);
}

EcoReal eco_test_real_div(EcoReal a, EcoReal b)
{
    if (b == 0) {
        return 0;
    }
return (EcoReal)((a * ECO_REAL_SCALE) / b);
}

EcoReal eco_test_real_abs(EcoReal x)
{
    return x < 0 ? -x : x;
}

bool eco_test_real_close(EcoReal a, EcoReal b, EcoReal tolerance)
{
    return eco_test_real_abs(a - b) <= tolerance;
}

EcoVector eco_test_vector_new(EcoIndex length)
{
    EcoVector vector = calloc((size_t)length + 1u, sizeof(EcoReal));
    if (vector == NULL) {
        return NULL;
    }
    vector[0] = (EcoReal)length;
    return vector;
}

void eco_test_vector_free(EcoVector vector)
{
    free(vector);
}

EcoVector eco_test_vector_clone(EcoVector vector)
{
    if (vector == NULL) {
        return NULL;
    }

    EcoIndex length = ECO_VEC_LENGTH(vector);
    EcoVector clone = eco_test_vector_new(length);
    if (clone == NULL) {
        return NULL;
    }

    for (EcoIndex i = 1; i <= length; ++i) {
        clone[i] = vector[i];
    }

    return clone;
}

EcoMatrix eco_test_matrix_new(EcoIndex rows, EcoIndex cols)
{
    EcoMatrix matrix = calloc((size_t)rows + 1u, sizeof(EcoReal *));
    if (matrix == NULL) {
        return NULL;
    }

    matrix[0] = calloc(1u, sizeof(EcoReal));
    if (matrix[0] == NULL) {
        free(matrix);
        return NULL;
    }
    matrix[0][0] = (EcoReal)rows;

    for (EcoIndex i = 1; i <= rows; ++i) {
        matrix[i] = calloc((size_t)cols + 1u, sizeof(EcoReal));
        if (matrix[i] == NULL) {
            for (EcoIndex k = 0; k < i; ++k) {
                free(matrix[k]);
            }
            free(matrix);
            return NULL;
        }
        matrix[i][0] = (EcoReal)cols;
    }

    return matrix;
}

void eco_test_matrix_free(EcoMatrix matrix)
{
    if (matrix == NULL) {
        return;
    }

    EcoIndex rows = ECO_MAT_ROWS(matrix);
    for (EcoIndex i = 0; i <= rows; ++i) {
        free(matrix[i]);
    }
    free(matrix);
}

EcoActor *eco_test_actor_new(EcoActorId id,
                             EcoIndex resource_count,
                             EcoResourceId produced_resource,
                             EcoProductionFunction production,
                             void *production_context)
{
    EcoActor *actor = calloc(1u, sizeof(*actor));
    if (actor == NULL) {
        return NULL;
    }

    actor->resources = eco_test_vector_new(resource_count);
    actor->consumption_pattern = eco_test_vector_new(resource_count);
    if (actor->resources == NULL || actor->consumption_pattern == NULL) {
        eco_test_actor_free(actor);
        return NULL;
    }

    actor->id = id;
    actor->produced_resource = produced_resource;
    actor->production = production;
    actor->production_context = production_context;

    return actor;
}

void eco_test_actor_free(EcoActor *actor)
{
    if (actor == NULL) {
        return;
    }
    eco_test_vector_free(actor->resources);
    eco_test_vector_free(actor->consumption_pattern);
    free(actor);
}

EcoActorSet *eco_test_actor_set_new(EcoIndex count)
{
    EcoActorSet *set = calloc(1u, sizeof(*set));
    if (set == NULL) {
        return NULL;
    }

    set->actors = calloc((size_t)count + 1u, sizeof(EcoActor *));
    if (set->actors == NULL) {
        free(set);
        return NULL;
    }

    set->count = count;
    return set;
}

void eco_test_actor_set_free(EcoActorSet *set)
{
    if (set == NULL) {
        return;
    }

    if (set->actors != NULL) {
        for (EcoIndex i = 1; i <= set->count; ++i) {
            eco_test_actor_free(set->actors[i]);
        }
    }

    free(set->actors);
    free(set);
}

EcoTactic *eco_test_tactic_new(EcoIndex actor_count)
{
    EcoTactic *tactic = calloc(1u, sizeof(*tactic));
    if (tactic == NULL) {
        return NULL;
    }

    tactic->emission = eco_test_matrix_new(actor_count, actor_count);
    tactic->reception = eco_test_matrix_new(actor_count, actor_count);
    if (tactic->emission == NULL || tactic->reception == NULL) {
        eco_test_tactic_free(tactic);
        return NULL;
    }

    return tactic;
}

void eco_test_tactic_free(EcoTactic *tactic)
{
    if (tactic == NULL) {
        return;
    }
    eco_test_matrix_free(tactic->emission);
    eco_test_matrix_free(tactic->reception);
    free(tactic);
}

EcoExchangeTable *eco_test_exchange_table_new(EcoIndex actor_count)
{
    EcoExchangeTable *table = calloc(1u, sizeof(*table));
    if (table == NULL) {
        return NULL;
    }

    table->emitted = eco_test_matrix_new(actor_count, actor_count);
    table->received = eco_test_matrix_new(actor_count, actor_count);
    if (table->emitted == NULL || table->received == NULL) {
        eco_test_exchange_table_free(table);
        return NULL;
    }

    return table;
}

void eco_test_exchange_table_free(EcoExchangeTable *table)
{
    if (table == NULL) {
        return;
    }
    eco_test_matrix_free(table->emitted);
    eco_test_matrix_free(table->received);
    free(table);
}

bool eco_test_vector_has_length(EcoVector vector, EcoIndex length)
{
    return vector != NULL && ECO_VEC_LENGTH(vector) == length;
}

bool eco_test_matrix_has_shape(EcoMatrix matrix, EcoIndex rows, EcoIndex cols)
{
    if (matrix == NULL || matrix[0] == NULL || ECO_MAT_ROWS(matrix) != rows) {
        return false;
    }

    for (EcoIndex i = 1; i <= rows; ++i) {
        if (matrix[i] == NULL || ECO_MAT_COLS(matrix) != cols) {
            return false;
        }
    }

    return true;
}

bool eco_test_flag_matrix_is_boolean(EcoFlagMatrix matrix)
{
    if (matrix == NULL) {
        return false;
    }

    EcoIndex rows = ECO_MAT_ROWS(matrix);
    for (EcoIndex i = 1; i <= rows; ++i) {
        EcoIndex cols = ECO_MAT_COLS(matrix);
        for (EcoIndex j = 1; j <= cols; ++j) {
            EcoReal value = ECO_MAT_AT(matrix, i, j);
            if (value != ECO_REAL_ZERO && value != ECO_REAL_ONE) {
                return false;
            }
        }
    }

    return true;
}

bool eco_test_matrix_is_non_negative(EcoMatrix matrix)
{
    if (matrix == NULL) {
        return false;
    }

    EcoIndex rows = ECO_MAT_ROWS(matrix);
    for (EcoIndex i = 1; i <= rows; ++i) {
        EcoIndex cols = ECO_MAT_COLS(matrix);
        for (EcoIndex j = 1; j <= cols; ++j) {
            if (ECO_MAT_AT(matrix, i, j) < ECO_REAL_ZERO) {
                return false;
            }
        }
    }

    return true;
}

EcoReal eco_test_production_identity(EcoVector consumption, void *context)
{
    (void)context;
    if (consumption == NULL || ECO_VEC_LENGTH(consumption) < 1u) {
        return 0;
    }
    return consumption[1];
}

EcoReal eco_test_production_linear_from_resource_1(EcoVector consumption, void *context)
{
    EcoReal coefficient = context != NULL ? *((EcoReal *)context) : ECO_REAL_ONE;
    if (consumption == NULL || ECO_VEC_LENGTH(consumption) < 1u) {
        return 0;
    }
    return eco_test_real_mul(coefficient, consumption[1]);
}

EcoReal eco_test_weight_half(EcoReal p_a, EcoReal p_b_hat, void *context)
{
    (void)p_a;
    (void)p_b_hat;
    (void)context;
    return ECO_REAL_ONE / 2;
}
