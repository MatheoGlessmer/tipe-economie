#ifndef ECO_TEST_WEAK_SYMBOLS_H
#define ECO_TEST_WEAK_SYMBOLS_H

/*
 * Les fonctions du modèle ne sont pas encore forcément implémentées.
 * Avec GCC/Clang, les symboles faibles permettent de compiler la suite de
 * tests quand même : si une fonction manque, son adresse vaut NULL et le test
 * correspondant est marqué TODO.
 */

#include "economie_model.h"
#include "economie_display.h"

#if defined(__GNUC__) || defined(__clang__)
#define ECO_TEST_HAS_WEAK_SYMBOLS 1
#else
#define ECO_TEST_HAS_WEAK_SYMBOLS 0
#endif

#if ECO_TEST_HAS_WEAK_SYMBOLS
#pragma weak eco_vector_create
#pragma weak eco_vector_destroy
#pragma weak eco_matrix_create
#pragma weak eco_matrix_destroy
#pragma weak eco_actor_create
#pragma weak eco_actor_destroy
#pragma weak eco_actor_set_create
#pragma weak eco_actor_set_destroy
#pragma weak eco_tactic_create
#pragma weak eco_tactic_destroy
#pragma weak eco_exchange_table_create
#pragma weak eco_exchange_table_destroy
#pragma weak eco_price_matrix_before_negotiation
#pragma weak eco_price_matrix_after_negotiation
#pragma weak eco_allowed_quantities_for_pair
#pragma weak eco_unilateral_selection
#pragma weak eco_intersect_selections
#pragma weak eco_build_exchange_table
#pragma weak eco_update_actors_by_exchange
#pragma weak eco_update_actors_by_production
#pragma weak eco_update_actors_global

#pragma weak eco_display_options_default
#pragma weak eco_real_to_string
#pragma weak eco_real_fprint
#pragma weak eco_real_print
#pragma weak eco_vector_fprint
#pragma weak eco_vector_print
#pragma weak eco_matrix_fprint
#pragma weak eco_matrix_print
#pragma weak eco_flag_matrix_fprint
#pragma weak eco_flag_matrix_print
#pragma weak eco_actor_fprint
#pragma weak eco_actor_print
#pragma weak eco_actor_set_fprint
#pragma weak eco_actor_set_print
#pragma weak eco_tactic_fprint
#pragma weak eco_tactic_print
#pragma weak eco_exchange_table_fprint
#pragma weak eco_exchange_table_print
#pragma weak eco_market_fprint
#pragma weak eco_market_print
#pragma weak eco_state_sequence_fprint
#pragma weak eco_state_sequence_print
#pragma weak eco_actor_resources_fprint
#pragma weak eco_actor_consumption_pattern_fprint
#pragma weak eco_matrix_row_fprint
#pragma weak eco_matrix_column_fprint
#endif

#define ECO_TEST_FUNCTION_MISSING(function_name) ((function_name) == 0)

#endif /* ECO_TEST_WEAK_SYMBOLS_H */
