# Simulation économique — TIPE 2026

Modèle jouet de flux monétaires entre acteurs économiques, implémenté en C,
pour étudier dans quelles conditions un système économique simplifié se
stabilise, oscille ou diverge.

> **État du projet** : modèle, simulation et suite de tests implémentés ;
> plusieurs marchés d'exemple simulés et comparés. L'analyse détaillée des
> résultats et le rapport sont en cours. Projet de TIPE 2026 (candidat libre),
> thème « Cycle et boucle ». Il s'agit d'une première modélisation, appelée à
> évoluer.

## Question étudiée

> Dans quelles conditions un système économique simplifié présente-t-il un
> comportement **stable**, **oscillatoire** ou **divergent** ?

L'objectif n'est pas de reproduire une économie réelle, mais de construire un
modèle suffisamment rigoureux pour observer expérimentalement différentes
dynamiques. Le modèle est un système dynamique discret : à chaque étape, les
acteurs échangent puis produisent, et l'état obtenu est réinjecté dans l'étape
suivante. La spécification mathématique complète (définitions, algorithmes avec
préconditions et postconditions) est dans [`docs/Tipe_2026.pdf`](docs/Tipe_2026.pdf).

## En bref : ce que fait le programme

Le programme lit un marché décrit dans un fichier **`.eco`** et écrit le
résultat dans un fichier **`.mat`**, selon les spécifications mathématiques du
modèle. Par exemple, `m1.eco` donne `jac_m1.mat`, le jacobien du marché `m1`
`[à confirmer]`.

```bash
# [commande exacte à compléter]
./build/simulation examples/m1.eco results/jac_m1.mat
```

Marchés d'exemple fournis dans [`examples/`](examples/) : `m1` à `m4` et un
marché `divergent` `[à confirmer : un marché au comportement divergent]`. Les
sorties correspondantes sont dans [`results/`](results/).

## Le modèle en bref

- **Acteur** `a = (R_a, τ_a, ρ_a, C_a)` : un stock de ressources, une fonction
  de production linéaire, l'unique ressource qu'il produit et un patron de
  consommation (proportions fixes).
- **Tactique de marché** `Δ` : pour chaque couple d'acteurs, la quantité
  maximale qu'on accepte d'émettre et la quantité minimale qu'on veut recevoir.
- **Marché** `M = (A_I, (Δ^(n)), μ, λ)` : acteurs initiaux, stratégie,
  fonction de mise à jour `μ` et fonction de pondération `λ` des prix.
- **Dynamique** : `A_{n+1} = μ(A_n, Δ^(n), λ)`.

Une étape de simulation enchaîne huit briques : négociation des prix, calcul des
quantités échangeables, sélection unilatérale, **échange seulement si les deux
parties sont d'accord**, puis production maximale avec les ressources obtenues.

| Brique | Rôle | Fonction C |
|---|---|---|
| 1 | prix avant négociation | `eco_price_matrix_before_negotiation` |
| 2 | négociation bilatérale des prix | `eco_price_matrix_after_negotiation` |
| 3 | quantités échangeables | `eco_allowed_quantities_for_pair` |
| 4 | sélection unilatérale | `eco_unilateral_selection` |
| 5 | intersection des volontés | `eco_intersect_selections` |
| 6 | tableau des échanges | `eco_build_exchange_table` |
| 7 | mise à jour après échanges | `eco_update_actors_by_exchange` |
| 8 | production maximale | `eco_update_actors_by_production` |
| étape complète | horloge, tactique courante, `μ` | `maj` (`market.h`) |

## Choix d'implémentation

- **Virgule fixe** : `EcoReal` est un `int64_t` d'échelle 10⁶ (l'entier
  1 000 000 représente 1.0), pour éviter les comparaisons instables entre
  flottants.
- **Vecteurs et matrices indexés à partir de 1**, la taille étant stockée dans
  la case 0, pour rester proche de l'écriture mathématique.
- **Pointeurs de fonction** pour la production, la pondération `λ`, les
  générateurs de stratégies et la mise à jour `μ`, chacun avec un contexte
  `void *`.
- **Sérialisation** d'un marché, d'un vecteur ou d'une matrice
  (`economie_serialize`), avec une table de correspondance entre fonctions C et
  identifiants dans le fichier.
- **Jacobien** (`jacobien`) : conversion de l'état d'un marché en vecteur et
  calcul du jacobien d'une étape avec un pas `step`. `[à compléter]`
- **Analyse de matrices** (`matrix_analysis`) : outil séparé de la simulation, qui lit
  une matrice `.mat` produite par le programme et calcule ses valeurs propres avec
  LAPACK (`LAPACKE_dgeev`), pour juger de la stabilité locale du système
  `[à confirmer]`.
- **Affichage** configurable (`economie_display`) : décimales, indices, noms
  d'acteurs et de ressources.
- Opérations sur les vecteurs (`ecovect_operation`) : somme, différence,
  produit scalaire, mise à l'échelle.

## Structure du dépôt

```text
.
├── README.md
├── Makefile
├── include/      en-têtes
├── src/          implémentation du modèle, de la simulation et de l'analyse
├── tests/        suite de tests unitaires (voir tests/README.md)
├── examples/     marchés d'exemple au format .eco
├── results/      sorties au format .mat
└── docs/         spécification mathématique du TIPE (Tipe_2026.pdf), rapport
```

## Compilation et exécution

Prérequis : `gcc` (ou `clang`) et `make` ; pour l'outil d'analyse,
LAPACKE (`sudo apt install liblapacke-dev` sous Debian/Ubuntu).

```bash
make run          # compile et lance la suite de tests
make              # compile le programme de simulation (build/simulation)
make analysis     # compile l'outil d'analyse des matrices (build/matrix_analysis)
```

## Tests

La suite de tests est écrite sans dépendance externe, avec un mini-framework à
sorties colorées (`PASS`, `FAIL`, `CRASH`, `TODO`). Chaque test construit une
entrée qui respecte les préconditions, appelle la fonction, puis vérifie les
postconditions. Détails dans [`tests/README.md`](tests/README.md).

## Résultats et suite du projet

Plusieurs marchés (`m1` à `m4`, `divergent`) ont été simulés et leurs
résultats comparés. L'exploitation de ces résultats n'est pas encore rédigée.

Pistes et points restants :

- [ ] analyse comparée des marchés d'exemple (stable, oscillatoire, divergent) ;
- [ ] étude de l'effet de la fonction de pondération `λ`, du nombre d'acteurs,
      des patrons de consommation et des stocks initiaux ;
- [ ] évolution du modèle (la première modélisation sera modifiée) ;
- [ ] `[dernier point du projet à préciser]` ;
- [ ] rapport complet.

## Hypothèses et limites

- La linéarité de la fonction de production `τ_a` est une hypothèse du modèle,
  non vérifiable par le type C.
- La règle de consentement bilatéral est locale et peu coûteuse, mais elle ne
  réalise pas une optimisation globale des échanges.
- L'ordre « échanges puis production » est un choix de modélisation : l'ordre
  inverse définirait une autre dynamique.
- Les valeurs sont en virgule fixe : la précision est de l'ordre de 10⁻⁶.
- Le modèle reste un modèle jouet : ses résultats ne décrivent pas une économie
  réelle.

## Auteur et crédits

Mathéo Glessmer, élève-ingénieur à l'ENSIIE (filière HPC).

Licence : tous droit réservé