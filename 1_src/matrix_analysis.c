#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

// Inclusion de l'en-tête de LAPACKE
#include <lapacke.h> 

#define ECO_REAL_SCALE 1000000.0

/**
 * Lit la matrice depuis le fichier et la charge dans un tableau 1D
 * adapté à la convention Row-Major de LAPACKE.
 */
double* load_matrix(const char* filename, int* n) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }

    uint32_t rows, cols;
    if (fscanf(file, "%" SCNu32 " %" SCNu32, &rows, &cols) != 2) {
        fprintf(stderr, "Erreur de lecture des dimensions dans %s.\n", filename);
        fclose(file);
        return NULL;
    }

    if (rows != cols) {
        fprintf(stderr, "Erreur: Le Jacobien doit etre une matrice carree.\n");
        fclose(file);
        return NULL;
    }

    *n = (int)rows;
    double* mat = malloc(sizeof(double) * rows * cols);

    for (uint32_t i = 0; i < rows; i++) {
        for (uint32_t j = 0; j < cols; j++) {
            int64_t raw_val;
            fscanf(file, "%" SCNd64, &raw_val);
            // Conversion Virgule fixe -> Flottant
            mat[i * cols + j] = (double)raw_val / ECO_REAL_SCALE;
        }
    }

    fclose(file);
    return mat;
}

int main(int argc, char** argv) {
    // 1. Vérification stricte des arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <fichier_entree.mat> <fichier_sortie.txt> <nb_decimales>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* fichier_entree = argv[1];
    const char* fichier_sortie = argv[2];
    int nb_decimales = atoi(argv[3]);

    if (nb_decimales < 0) nb_decimales = 0; // Sécurité

    // 2. Chargement de la matrice
    int n;
    double* A = load_matrix(fichier_entree, &n);
    if (A == NULL) return EXIT_FAILURE;

    // 3. Préparation pour LAPACKE
    double* wr = malloc(sizeof(double) * n);
    double* wi = malloc(sizeof(double) * n);

    // Calcul des valeurs propres
    lapack_int info = LAPACKE_dgeev(LAPACK_ROW_MAJOR, 'N', 'N', n, A, n, wr, wi, NULL, 1, NULL, 1);

    if (info > 0) {
        fprintf(stderr, "Erreur : L'algorithme QR de LAPACKE n'a pas converge.\n");
        free(A); free(wr); free(wi);
        return EXIT_FAILURE;
    }

    // 4. Ouverture du fichier de sortie
    FILE* f_out = fopen(fichier_sortie, "w");
    if (f_out == NULL) {
        fprintf(stderr, "Erreur : Impossible de creer le fichier de sortie %s\n", fichier_sortie);
        free(A); free(wr); free(wi);
        return EXIT_FAILURE;
    }

    // 5. Écriture ligne par ligne au format demandé
    for (int i = 0; i < n; i++) {
        if (fabs(wi[i]) > 1e-9) { 
            // Valeur propre complexe : Module : Phase
            // hypot(x, y) est plus robuste que sqrt(x*x + y*y) face aux limites du type double
            double module = hypot(wr[i], wi[i]);
            
            // atan2(y, x) donne la phase (l'argument) en radians entre -pi et pi
            double phase = atan2(wi[i], wr[i]); 
            
            // L'astuce "%.*f" permet d'utiliser une variable pour le nombre de décimales !
            fprintf(f_out, "%.*f : %.*f\n", nb_decimales, module, nb_decimales, phase);
        } else { 
            // Valeur propre réelle pure
            fprintf(f_out, "%.*f\n", nb_decimales, wr[i]);
        }
    }

    // 6. Nettoyage
    fclose(f_out);
    free(A);
    free(wr);
    free(wi);

    printf("Extraction reussie ! %d valeurs propres ecrites dans '%s'.\n", n, fichier_sortie);

    return EXIT_SUCCESS;
}