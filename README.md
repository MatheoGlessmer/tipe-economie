# Suite de tests C pour le modèle économique

Ce dossier contient une suite de tests unitaire légère, sans dépendance externe.
Elle remplace volontairement CUnit pour éviter d'ajouter une bibliothèque à
installer. Les sorties sont colorées :

- vert : `PASS`, test réussi ;
- rouge : `FAIL`, postcondition non respectée ;
- violet : `CRASH`, segmentation fault, division illégale, abort, etc. ;
- jaune : `TODO`, fonction non encore implémentée.

## Organisation

```text
include/
  economie_model.h
  economie_display.h
src/
  eco_test_framework.*   mini-framework de tests avec couleurs
  eco_test_utils.*       fixtures, matrices, vecteurs, virgule fixe
tests/
  test_structure_helpers.c
  test_model_alloc.c
  test_model_algorithms.c
  test_display.c
  main.c
Makefile
```

## Compilation sans implémentation

```bash
make run
```

Les tests des fonctions non implémentées seront marqués `TODO` au lieu de faire
échouer l'édition des liens. Cela repose sur les symboles faibles GCC/Clang.

## Compilation avec ton implémentation

Quand tu auras ajouté les fichiers `.c` du modèle, lance par exemple :

```bash
make run IMPL_SRCS="../src/economie_model.c ../src/economie_display.c"
```

Dès qu'une fonction existe, son test passe de `TODO` à un vrai test de
préconditions/postconditions.

## Principe des tests

On ne teste pas volontairement des entrées invalides pour provoquer des erreurs.
Le but est :

1. construire une entrée qui respecte les préconditions ;
2. appeler la fonction ;
3. vérifier que les postconditions attendues sont satisfaites.

C'est exactement le style de test adapté à ton document mathématique.
