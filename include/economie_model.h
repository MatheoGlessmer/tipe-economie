#ifndef ECONOMIE_MODEL_H
#define ECONOMIE_MODEL_H

/*
 * economie_model.h
 *
 * Structures de base pour le modèle économique du TIPE.
 *
 * Convention principale : les suites mathématiques sont représentées par
 * des tableaux finis. Les vecteurs et matrices numériques sont indexés à
 * partir de 1. La case 0 contient toujours une information de taille.
 *
 *
 *
 * Les dimensions sont stockées dans des cellules EcoReal afin de garder
 * des tableaux homogènes. Conceptuellement, ces cellules doivent être
 * interprétées comme des entiers naturels.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ------------------------------------------------------------------------- */
/* Configuration numérique                                                   */
/* ------------------------------------------------------------------------- */

/*
 * Représentation en virgule fixe.
 *
 * ECO_REAL_SCALE = 1 000 000 signifie que l'entier 1 000 000 représente 1.0
 * Ce choix évite les comparaisons instables entre flottants.
*/

#ifndef ECO_REAL_SCALE
#define ECO_REAL_SCALE 1000000LL
#endif

#define ECO_REAL_ZERO ((EcoReal)0)
#define ECO_REAL_ONE  ((EcoReal)ECO_REAL_SCALE)

typedef int64_t EcoReal;
typedef uint32_t EcoIndex;
typedef uint32_t EcoActorId;
typedef uint32_t EcoResourceId;
typedef uint64_t EcoStep;

#define ECO_INDEX_NONE    ((EcoIndex)0)
#define ECO_ACTOR_NONE    ((EcoActorId)0)
#define ECO_RESOURCE_NONE ((EcoResourceId)0)

/*
 * Les conversions ci-dessous sont des macros de représentation
 */
#define ECO_REAL_FROM_INT(x) ((EcoReal)(x) * (EcoReal)ECO_REAL_SCALE)
#define ECO_REAL_TO_INT(x)   ((int64_t)((x) / (EcoReal)ECO_REAL_SCALE))

/* ------------------------------------------------------------------------- */
/* Tableaux finis, vecteurs et matrices                                      */
/* ------------------------------------------------------------------------- */

/*
 * Suite finie de réels en virgule fixe.
 *
 * Convention :
 *   v[0] = longueur n ;
 *   v[i] = composante i pour 1 <= i <= n.
 *
 * Dans le modèle, EcoVector sert notamment à représenter :
 *   - un stock de ressources R_a ;
 *   - un patron de consommation C_a ;
 *   - une consommation effective alpha C_a.
 */
typedef EcoReal *EcoVector;

/*
 * Matrice numérique représentée comme un tableau de tableaux.
 *
 * Convention :
 *   M[0][0] = nombre de lignes ;
 *   M[i][0] = nombre de colonnes de la ligne i ;
 *   M[i][j] = coefficient de la ligne i et de la colonne j.
 *
 * Les lignes commencent donc à l'indice 1, ce qui rapproche l'implémentation
 * de l'écriture mathématique.
 */
typedef EcoReal **EcoMatrix;

/*
 * Les matrices booléennes sont aussi stockées dans EcoMatrix.
 * Les seules valeurs autorisées sont 0 et 1.
 */
typedef EcoMatrix EcoFlagMatrix;

#define ECO_VEC_LENGTH(v)       ((EcoIndex)((v)[0]))
#define ECO_VEC_AT(v, i)        ((v)[(i)])

#define ECO_MAT_ROWS(M)         ((EcoIndex)((M)[0][0]))
#define ECO_MAT_COLS(M)      ((EcoIndex)((M)[1][0]))
#define ECO_MAT_AT(M, i, j)     ((M)[(i)][(j)])

/* ------------------------------------------------------------------------- */
/* Fonctions mathématiques du modèle                                        */
/* ------------------------------------------------------------------------- */
/*
 * Fonction de production tau_a.
 *
 * Entrée : une consommation représentée par un EcoVector.
 * Sortie : quantité produite de la ressource rho_a, en EcoReal.
 *
 * Hypothèse mathématique prévue dans le modèle : tau_a est linéaire.
 * Cette hypothèse n'est pas vérifiable automatiquement par le type C.
 */
typedef EcoReal (*EcoProductionFunction)(const EcoVector consumption,
                                         void *context);

/*
 * Fonction de pondération lambda utilisée lors de la négociation.
 *
 * Entrées :
 *   p_a     : prix maximal proposé par a, en rho_a / rho_b ;
 *   p_b_hat : inverse du prix maximal proposé par b, ramené dans la même unité.
 *
 * Sortie : poids dans [0, ECO_REAL_ONE].
 */
typedef EcoReal (*EcoWeightFunction)(EcoReal p_a,
                                     EcoReal p_b_hat,
                                     void *context);

/* ------------------------------------------------------------------------- */
/* Acteurs, tactiques, échanges                                              */
/* ------------------------------------------------------------------------- */

/*
 * Acteur économique :
 *   a = (R_a, tau_a, rho_a, C_a)
 *
 * R_a   : stock fini de ressources ;
 * tau_a : fonction de production ;
 * rho_a : indice de l'unique ressource produite ;
 * C_a   : patron de consommation à proportions fixes.
 */
typedef struct EcoActor {
    EcoActorId id;
    EcoVector resources;
    EcoProductionFunction production;
    void *production_context;
    EcoResourceId produced_resource;
    EcoVector consumption_pattern;
} EcoActor;

/*
 * Ensemble fini d'acteurs.
 *
 * Convention :
 *   actors[0] inutile;
 *   actors[1], ..., actors[count] sont les acteurs.
 */
typedef struct EcoActorSet {
    EcoIndex count;
    EcoActor **actors;
} EcoActorSet;

/*
 * Tactique de marché Delta.
 *
 * Plutôt que de stocker une matrice de couples, on stocke deux matrices :
 *   emission[a][b]  = delta_e : quantité maximale de rho_a que a accepte
 *                     d'émettre vers b ;
 *   reception[a][b] = delta_r : quantité minimale de rho_b que a souhaite
 *                     recevoir de b.
 */
typedef struct Ecof {
    EcoMatrix emission;
    EcoMatrix reception;
} EcoTactic;

/*
 * Tableau d'échanges effectifs chi.
 *
 * Si chi_{a,b} = (m_{a,b}, q_{a,b}), alors :
 *   - a émet m_{a,b} unités de rho_a vers b ;
 *   - a reçoit q_{a,b} unités de rho_b depuis b.
 *
 * Comme pour Delta, on sépare les deux composantes en deux matrices.
 */
typedef struct EcoExchangeTable {
    EcoMatrix emitted;
    EcoMatrix received;
} EcoExchangeTable;

/*
 * Matrices spécialisées du modèle.
 */
typedef EcoMatrix EcoPriceMatrix;       /* P ou P_f */
typedef EcoMatrix EcoQuantityMatrix;    /* Q */
typedef EcoFlagMatrix EcoSelectionMatrix; /* S */
typedef EcoFlagMatrix EcoAcceptanceMatrix; /* C_acc */

/* ------------------------------------------------------------------------- */
/* Stratégies, marchés et suites d'états                                     */
/* ------------------------------------------------------------------------- */

/*
 * Une stratégie peut être représentée soit par une suite finie de tactiques,
 * soit par un générateur externe. Les implémentations pourront choisir l'une
 * ou l'autre représentation.
 */
typedef EcoTactic *(*EcoStrategyGenerator)(EcoStep step,
                                           const EcoActorSet *state,
                                           void *context);

typedef struct EcoStrategy {
    EcoIndex tactic_count;
    EcoIndex tactic_id;
    EcoTactic **tactics;              /* tactiques[1..tactic_count]  */
    EcoStrategyGenerator generator;   /* optionnel */
    void *generator_context;
} EcoStrategy;

/*
 * Signature d'une fonction globale de mise à jour.
 *
 * Elle correspond à :
 *   A_{n+1} = mu(A_n, Delta^{(n)}, lambda)
 */
typedef EcoActorSet *(*EcoUpdateFunction)(const EcoActorSet *actors,
                                          const EcoTactic *tactic,
                                          EcoWeightFunction weight,
                                          void *weight_context,
                                          void *update_context);

/*
 * Marché :
 *   M = (A_I, (Delta^{(n)})_{n in N}, mu, lambda)
 */
typedef struct EcoMarket {
    EcoActorSet *initial_actors;
    EcoStrategy strategy;
    EcoUpdateFunction update;
    void *update_context;
    EcoWeightFunction weight;
    void *weight_context;
} EcoMarket;

/*
 * Suite d'états d'un marché, représentée en pratique par une suite finie
 * d'états calculés.
 */
typedef struct EcoStateSequence {
    EcoIndex state_count;
    EcoActorSet **states; /* states[1..state_count] */
} EcoStateSequence;
/*------------------------------------------------------------------------ */
/* Signatures optionnelles pour les futures implémentations                  */
/* ------------------------------------------------------------------------- */

EcoVector eco_vector_create(EcoIndex length);
void eco_vector_destroy(EcoVector vector);

EcoMatrix eco_matrix_create(EcoIndex rows, EcoIndex cols);
void eco_matrix_destroy(EcoMatrix matrix);

EcoActor *eco_actor_create(EcoIndex id);
void eco_actor_destroy(EcoActor *actor);

EcoActorSet *eco_actor_set_create(EcoIndex count);
void eco_actor_set_destroy(EcoActorSet *set);

EcoTactic *eco_tactic_create(EcoIndex actor_count);
void eco_tactic_destroy(EcoTactic *tactic);

EcoExchangeTable *eco_exchange_table_create(EcoIndex actor_count);
void eco_exchange_table_destroy(EcoExchangeTable *exchange_table);

/*
 * Prototypes des briques mathématiques. 
*/
EcoPriceMatrix eco_price_matrix_before_negotiation(const EcoActorSet *actors,
                                                   const EcoTactic *tactic);

EcoPriceMatrix eco_price_matrix_after_negotiation(const EcoActorSet *actors,
                                                  const EcoPriceMatrix prices,
                                                  EcoWeightFunction weight,
                                                  void *weight_context);

void eco_allowed_quantities_for_pair(const EcoActor *buyer,
                                     const EcoActor *seller,
                                     const EcoTactic *tactic,
                                     EcoReal negotiated_price,
                                     EcoReal *q_min,
                                     EcoReal *q_max);

EcoVector eco_unilateral_selection(const EcoActorSet *actors,
                                   const EcoActor *actor,
                                   const EcoVector quantities,
                                   const EcoVector prices,
                                   EcoReal *out_cost);

EcoAcceptanceMatrix eco_intersect_selections(const EcoActorSet *actors,
                                             const EcoSelectionMatrix selections);

EcoExchangeTable *eco_build_exchange_table(const EcoActorSet *actors,
                                           const EcoQuantityMatrix quantities,
                                           const EcoVector gammas, /* NOUVEAU */
                                           const EcoPriceMatrix prices,
                                           const EcoAcceptanceMatrix acceptances);

/*
 * Résultat de l'étape de mise à jour par l'échange (Brique 7).
 * Correspond à la sortie (A_ech, chi) de l'Algorithme 1.7.
 */
typedef struct EcoExchangeResult {
    EcoActorSet *updated_actors;      /* A_ech */
    EcoExchangeTable *exchange_table; /* chi */
} EcoExchangeResult;

/* Fonction utilitaire de libération (si tu respectes la norme de ton framework) */
void eco_exchange_result_destroy(EcoExchangeResult *result);

EcoExchangeResult *eco_update_actors_by_exchange(
    const EcoActorSet *actors,
    const EcoTactic *tactic,
    const EcoPriceMatrix prices
);

EcoActorSet *eco_update_actors_by_production(const EcoActorSet *actors);


EcoMarket *deep_copy(EcoMarket* market);

#endif /* ECONOMIE_MODEL_H */
