/* main.c
 *
 * Petit utilitaire CLI pour compresser / décompresser des fichiers
 * en utilisant l'implémentation Huffman fournie.
 *
 * Usage :
 *   ./huffman -c input_path output_path   # compresse
 *   ./huffman -d input_path output_path   # décompresse
 *   ./huffman -h                          # aide
 *
 * Le programme appelle compress_file() / decompress_file() définies dans io.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#include "io.h"        /* compress_file, decompress_file, etc. */
#include "huffman.h"   /* pour fonctions utilitaires si besoin (affichage arbre...) */

/* Retourne la taille (en octets) d'un fichier. -1 en cas d'erreur. */
static long long file_size_bytes(const char *path) {
    if (!path) return -1;
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long long) st.st_size;
}

static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s -c <input> <output>    # compresser\n", prog);
    printf("  %s -d <input> <output>    # décompresser\n", prog);
    printf("  %s -h                     # aide\n", prog);
}

/* Optionnel : affiche résumé après compression */
static void print_stats_after_compress(const char *in, const char *out) {
    long long in_sz = file_size_bytes(in);
    long long out_sz = file_size_bytes(out);
    if (in_sz < 0 || out_sz < 0) {
        printf("Compression terminée (tailles indisponibles).\n");
        return;
    }
    double ratio = (in_sz == 0) ? 0.0 : (100.0 * (1.0 - ((double) out_sz / (double) in_sz)));
    printf("Input :  %s  => %lld octets\n", in, in_sz);
    printf("Output:  %s  => %lld octets\n", out, out_sz);
    if (in_sz == 0) {
        printf("Fichier source vide (aucune donnée compressée).\n");
    } else {
        printf("Taux de réduction : %.2f%%\n", ratio);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (argc != 4) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *mode = argv[1];
    const char *input = argv[2];
    const char *output = argv[3];

    if (strcmp(mode, "-c") == 0) {
        printf("Compression : %s -> %s\n", input, output);
        int rc = compress_file(input, output);
        if (rc != 0) {
            fprintf(stderr, "Erreur : échec de la compression (code %d)\n", rc);
            return EXIT_FAILURE;
        }
        print_stats_after_compress(input, output);
        return EXIT_SUCCESS;
    } else if (strcmp(mode, "-d") == 0) {
        printf("Décompression : %s -> %s\n", input, output);
        int rc = decompress_file(input, output);
        if (rc != 0) {
            fprintf(stderr, "Erreur : échec de la décompression (code %d)\n", rc);
            return EXIT_FAILURE;
        }
        long long out_sz = file_size_bytes(output);
        if (out_sz >= 0) {
            printf("Fichier décompressé écrit (%s) : %lld octets\n", output, out_sz);
        } else {
            printf("Fichier décompressé écrit (%s)\n", output);
        }
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Mode inconnu : %s\n", mode);
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
}
