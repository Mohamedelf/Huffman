#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdint.h>

/*
 * io.h
 *
 * API d'E/S bit-à-bit et fonctions de compression/décompression basiques
 * pour l'implémentation Huffman du projet.
 *
 * Dépendances attendues :
 * - huffman.h (Noeud, construire_arbre_huffman, generer_codes, detruire_arbre, etc.)
 * - heap.h (utilisé indirectement par huffman.c)
 */

/*BitWriter / BitReader */

/* BitWriter : permet d'écrire des bits dans un FILE* (bufferisé par octet). */
typedef struct BitWriter {
    FILE *f;               /* flux de sortie (ouvert pour "wb") */
    unsigned char buffer;  /* tampon d'octet en construction (bits écrits de MSB->LSB) */
    int bit_count;         /* nombre de bits valides dans buffer (0..7) */
} BitWriter;

/* BitReader : permet de lire des bits depuis un FILE* (bufferisé par octet). */
typedef struct BitReader {
    FILE *f;               /* flux d'entrée (ouvert pour "rb") */
    unsigned char buffer;  /* octet courant */
    int bit_pos;           /* position du prochain bit à lire (0..7). -1 = buffer vide */
} BitReader;

/* Création / destruction */
BitWriter* bw_create(FILE *out);
void bw_write_flush(BitWriter *bw);    /* force l'écriture du dernier octet (avec padding zeros) */
void bw_destroy(BitWriter *bw);        /* n'appelle pas fclose(out) */

BitReader* br_create(FILE *in);
int br_read_bit(BitReader *br);        /* retourne 0 ou 1, ou -1 si EOF/error */
void br_destroy(BitReader *br);        /* n'appelle pas fclose(in) */

/* Écrire un bit (0/1) ; retourne 0 si OK, -1 si erreur */
int bw_write_bit(BitWriter *bw, int bit);

/* Écrire une séquence de bits fournie comme chaîne "01011..." ; retourne 0 si OK */
int bw_write_bits_from_string(BitWriter *bw, const char *bits);

/* Lecture de plusieurs bits (<=64) : retourne bits lus (LSB-aligned) ou -1 sur erreur/EOF.
 * count : nombre de bits souhaités (1..64). Si EOF avant, retourne -1.
 */
long long br_read_bits(BitReader *br, int count);

/*Header / fréquences*/

/* Écrit l'en-tête de fréquence sur le flux (magic + total_symbols + 256×uint64 BE).
 * total_symbols = somme des fréquences.
 * Retourne 0 si OK, -1 en cas d'erreur d'écriture.
 */
int write_freq_header(FILE *out, uint64_t total_symbols, const unsigned long freq_table[256]);

/* Lit l'en-tête et remplit freq_table[] et total_symbols (pointeur non NULL).
 * Retourne 0 si OK, -1 si format invalide ou erreur de lecture.
 */
int read_freq_header(FILE *in, uint64_t *out_total_symbols, unsigned long freq_table[256]);

/*Compression / Décompression haut-niveau*/

/* compress_file :
 * - lit le fichier source, compte les fréquences,
 * - construit l'arbre Huffman, génère les codes,
 * - écrit l'en-tête (table des fréquences + total) puis le flux compressé bit-à-bit.
 *
 * Retourne 0 si succès, -1 en cas d'erreur.
 */
int compress_file(const char *input_path, const char *output_path);

/* decompress_file :
 * - lit l'en-tête (reconstruit la table des fréquences),
 * - reconstruit l'arbre Huffman,
 * - lit le flux de bits et reconstruit exactement total_symbols octets,
 *   en écrivant dans output_path.
 *
 * Retourne 0 si succès, -1 en cas d'erreur.
 */
int decompress_file(const char *input_path, const char *output_path);

#endif /* IO_H */
