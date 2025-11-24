/*
 * io.c
 *
 * Implémentation des utilitaires d'E/S pour Huffman :
 * - BitWriter / BitReader
 * - lecture/écriture d'un en-tête contenant la table de fréquences
 * - fonctions haut-niveau compress_file / decompress_file
 *
 * Utilise l'API définie dans huffman.h (construire_arbre_huffman, generer_codes, etc).
 */

#include "io.h"
#include "huffman.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*Helper: écriture / lecture d'entiers 64-bit en big-endian */

static int write_u64_be(FILE *f, uint64_t v) {
    unsigned char b[8];
    for (int i = 0; i < 8; ++i) b[7 - i] = (unsigned char) ((v >> (i * 8)) & 0xFFULL);
    size_t w = fwrite(b, 1, 8, f);
    return (w == 8) ? 0 : -1;
}

static int read_u64_be(FILE *f, uint64_t *out_v) {
    unsigned char b[8];
    size_t r = fread(b, 1, 8, f);
    if (r != 8) return -1;
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) {
        v = (v << 8) | (uint64_t) b[i];
    }
    *out_v = v;
    return 0;
}

/*BitWriter implementation*/

BitWriter* bw_create(FILE *out) {
    if (!out) return NULL;
    BitWriter *bw = (BitWriter*) malloc(sizeof(BitWriter));
    if (!bw) return NULL;
    bw->f = out;
    bw->buffer = 0;
    bw->bit_count = 0;
    return bw;
}

/* Écrit le tampon courant (si bit_count > 0) en complétant par des zéros à droite. */
void bw_write_flush(BitWriter *bw) {
    if (!bw) return;
    if (bw->bit_count == 0) return;
    /* écrire l'octet (les bits non écrits à droite sont considérés 0) */
    unsigned char towrite = bw->buffer << (8 - bw->bit_count); /* aligner MSB */
    fwrite(&towrite, 1, 1, bw->f);
    bw->buffer = 0;
    bw->bit_count = 0;
    /* Ne pas fermer le FILE* ici */
}

void bw_destroy(BitWriter *bw) {
    if (!bw) return;
    /* ne pas fermer bw->f ; l'appelant gère FILE* */
    free(bw);
}

/* Écrit un bit (0 ou 1). On stocke les bits dans buffer de gauche à droite
 * (MSB first). Ex: premier bit écrit -> position 0 (MSB).
 */
int bw_write_bit(BitWriter *bw, int bit) {
    if (!bw) return -1;
    bit = (bit ? 1 : 0);
    /* mettre le bit en position (7 - bit_count) pour MSB-first */
    bw->buffer |= (unsigned char) (bit << (7 - bw->bit_count));
    bw->bit_count++;
    if (bw->bit_count == 8) {
        if (fwrite(&bw->buffer, 1, 1, bw->f) != 1) return -1;
        bw->buffer = 0;
        bw->bit_count = 0;
    }
    return 0;
}

/* Écrire une chaîne "010010..." pratique pour écrire un code produit par generer_codes. */
int bw_write_bits_from_string(BitWriter *bw, const char *bits) {
    if (!bw || !bits) return -1;
    for (const char *p = bits; *p; ++p) {
        if (*p == '0') {
            if (bw_write_bit(bw, 0) != 0) return -1;
        } else if (*p == '1') {
            if (bw_write_bit(bw, 1) != 0) return -1;
        } else {
            /* caractère invalide dans la chaîne de bits */
            return -1;
        }
    }
    return 0;
}

/*BitReader implementation*/

BitReader* br_create(FILE *in) {
    if (!in) return NULL;
    BitReader *br = (BitReader*) malloc(sizeof(BitReader));
    if (!br) return NULL;
    br->f = in;
    br->buffer = 0;
    br->bit_pos = -1; /* buffer vide initialement */
    return br;
}

/* Lire un bit (MSB-first). Retourne 0 ou 1, -1 si EOF ou erreur */
int br_read_bit(BitReader *br) {
    if (!br) return -1;
    if (br->bit_pos < 0) {
        unsigned char tmp;
        size_t r = fread(&tmp, 1, 1, br->f);
        if (r != 1) return -1; /* EOF ou erreur */
        br->buffer = tmp;
        br->bit_pos = 7; /* commencer par le MSB */
    }
    int bit = ( (br->buffer >> br->bit_pos) & 1 );
    br->bit_pos--;
    return bit;
}

/* Lire 'count' bits (1..64) et retourner la valeur dans un entier signé (non -1).
 * Nous retournons -1 si erreur/EOF.
 * Les bits sont lus MSB-first et renvoyés LSB-aligned :
 * ex: lire 3 bits '1','0','1' -> retourne b000...0101 = 5
 */
long long br_read_bits(BitReader *br, int count) {
    if (!br || count < 1 || count > 64) return -1;
    unsigned long long value = 0;
    for (int i = 0; i < count; ++i) {
        int b = br_read_bit(br);
        if (b < 0) return -1;
        value = (value << 1) | (unsigned long long) b;
    }
    return (long long) value;
}

void br_destroy(BitReader *br) {
    if (!br) return;
    free(br);
}

/*Header (freq table)*/

int write_freq_header(FILE *out, uint64_t total_symbols, const unsigned long freq_table[256]) {
    if (!out || !freq_table) return -1;
    /* magic */
    if (fwrite("HUF1", 1, 4, out) != 4) return -1;
    /* total symbols */
    if (write_u64_be(out, total_symbols) != 0) return -1;
    /* 256 uint64 BE */
    for (int i = 0; i < 256; ++i) {
        if (write_u64_be(out, (uint64_t) freq_table[i]) != 0) return -1;
    }
    return 0;
}

int read_freq_header(FILE *in, uint64_t *out_total_symbols, unsigned long freq_table[256]) {
    if (!in || !out_total_symbols || !freq_table) return -1;
    unsigned char magic[4];
    if (fread(magic, 1, 4, in) != 4) return -1;
    if (memcmp(magic, "HUF1", 4) != 0) return -1; /* format invalide */

    uint64_t total;
    if (read_u64_be(in, &total) != 0) return -1;
    *out_total_symbols = total;

    for (int i = 0; i < 256; ++i) {
        uint64_t f;
        if (read_u64_be(in, &f) != 0) return -1;
        freq_table[i] = (unsigned long) f; /* truncation possible si platform smaller */
    }
    return 0;
}

/*Compression haut niveau*/

int compress_file(const char *input_path, const char *output_path) {
    if (!input_path || !output_path) return -1;

    /* 1) compter fréquences */
    unsigned long freq_table[256];
    if (compter_frequences_fichier(input_path, freq_table) != 0) {
        return -1;
    }

    /* calcul du total */
    uint64_t total = 0;
    for (int i = 0; i < 256; ++i) total += (uint64_t) freq_table[i];
    if (total == 0) {
        /* fichier vide : créer fichier de sortie avec header et rien d'autre */
        FILE *out = fopen(output_path, "wb");
        if (!out) return -1;
        if (write_freq_header(out, 0, freq_table) != 0) { fclose(out); return -1; }
        fclose(out);
        return 0;
    }

    /* 2) construire arbre Huffman */
    Noeud *root = construire_arbre_huffman(freq_table);
    if (!root) return -1;

    /* 3) générer codes */
    char **codes = generer_codes(root);
    if (!codes) {
        detruire_arbre(root);
        return -1;
    }

    /* 4) ouvrir fichiers et écrire header */
    FILE *in = fopen(input_path, "rb");
    if (!in) {
        liberer_codes(codes);
        detruire_arbre(root);
        return -1;
    }
    FILE *out = fopen(output_path, "wb");
    if (!out) {
        fclose(in);
        liberer_codes(codes);
        detruire_arbre(root);
        return -1;
    }

    if (write_freq_header(out, total, freq_table) != 0) {
        fclose(in); fclose(out);
        liberer_codes(codes);
        detruire_arbre(root);
        return -1;
    }

    /* 5) initialiser BitWriter et écrire codes pour chaque octet lu */
    BitWriter *bw = bw_create(out);
    if (!bw) {
        fclose(in); fclose(out);
        liberer_codes(codes);
        detruire_arbre(root);
        return -1;
    }

    unsigned char buf[4096];
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), in)) > 0) {
        for (size_t i = 0; i < r; ++i) {
            unsigned char ch = buf[i];
            const char *code = codes[ch];
            if (!code) {
                /* théoriquement impossible si freq_table a été généré à partir du fichier */
                bw_destroy(bw);
                fclose(in); fclose(out);
                liberer_codes(codes);
                detruire_arbre(root);
                return -1;
            }
            if (bw_write_bits_from_string(bw, code) != 0) {
                bw_destroy(bw);
                fclose(in); fclose(out);
                liberer_codes(codes);
                detruire_arbre(root);
                return -1;
            }
        }
    }
    /* flush final (pad 0 jusqu'à octet) */
    bw_write_flush(bw);
    /* cleanup */
    bw_destroy(bw);
    fclose(in);
    fclose(out);

    liberer_codes(codes);
    detruire_arbre(root);
    return 0;
}

/*Décompression*/

/* Décodage : lit bits et suit l'arbre jusqu'à feuille, écrit le symbole, répète
 * jusqu'à total_symbols symboles produits.
 */
int decompress_file(const char *input_path, const char *output_path) {
    if (!input_path || !output_path) return -1;

    FILE *in = fopen(input_path, "rb");
    if (!in) return -1;

    uint64_t total_symbols;
    unsigned long freq_table[256];
    if (read_freq_header(in, &total_symbols, freq_table) != 0) {
        fclose(in);
        return -1;
    }

    /* si fichier compressé avec table mais total=0 => fichier original vide */
    if (total_symbols == 0) {
        FILE *out = fopen(output_path, "wb");
        if (!out) { fclose(in); return -1; }
        fclose(out);
        fclose(in);
        return 0;
    }

    /* reconstruire arbre */
    Noeud *root = construire_arbre_huffman(freq_table);
    if (!root) { fclose(in); return -1; }

    FILE *out = fopen(output_path, "wb");
    if (!out) {
        detruire_arbre(root);
        fclose(in);
        return -1;
    }

    BitReader *br = br_create(in);
    if (!br) {
        fclose(out);
        detruire_arbre(root);
        fclose(in);
        return -1;
    }

    /* Décodage symboles */
    uint64_t produced = 0;
    Noeud *cursor = root;

    while (produced < total_symbols) {
        int b = br_read_bit(br);
        if (b < 0) {
            /* EOF prématuré -> erreur */
            br_destroy(br);
            fclose(out);
            detruire_arbre(root);
            fclose(in);
            return -1;
        }

        /* avancer dans l'arbre : convention gauche=0, droite=1 */
        if (b == 0) {
            if (cursor->left) cursor = cursor->left;
        } else {
            if (cursor->right) cursor = cursor->right;
        }

        if (cursor->leaf) {
            unsigned char ch = cursor->c;
            if (fwrite(&ch, 1, 1, out) != 1) {
                br_destroy(br);
                fclose(out);
                detruire_arbre(root);
                fclose(in);
                return -1;
            }
            produced++;
            cursor = root; /* revenir à la racine pour décoder prochain symbole */
        }
        /* cas spécial : si l'arbre est une seule feuille (root->leaf) alors on ignore bits lus
         * et on doit ré-écrire le même symbole total_symbols fois. Le code ci-dessus
         * fonctionne si le flux contient au moins un bit (mais en pratique compress_file
         * écrit le code "0" pour la feuille unique). */
    }

    /* cleanup */
    br_destroy(br);
    fclose(out);
    detruire_arbre(root);
    fclose(in);
    return 0;
}
