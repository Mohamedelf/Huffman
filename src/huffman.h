#ifndef HUFFMAN_H  /*Ces lignes empêchent le compilateur d'inclure ce fichier plusieurs fois (ce qui causerait des erreurs de redéfinition)*/
#define HUFFMAN_H

#include <stddef.h> /* pour size_t */

/* Définition d'un noeud d'arbre Huffman.
 * - si leaf == 1, alors 'c' est valide et left/right sont NULL
 * - si leaf == 0, alors noeud interne : c non significatif, freq = somme des fréquences enfants
 */
typedef struct Noeud {
    unsigned char c;         /* symbole (0..255) pour les feuilles */
    unsigned long freq;      /* fréquence / poids */
    struct Noeud *left;      /* fils gauche (0) */
    struct Noeud *right;     /* fils droit (1) */
    int leaf;                /* indicateur feuille (1) ou interne (0) */
} Noeud;


/* Fonctions principales */

/* Crée un nouveau Noeud (feuille si left==right==NULL, sinon noeud interne).
 * Retourne NULL si l'allocation échoue.
 */
Noeud* creer_noeud(unsigned char c, unsigned long freq, Noeud *left, Noeud *right);

/* Libère récursivement un arbre (post-order). */
void detruire_arbre(Noeud *root);

/* Construire l'arbre de Huffman à partir d'un tableau de fréquences
 * freq_table[256] (pour chaque octet possible).
 *
 * Comportement :
 * - ignore les symboles dont la fréquence est 0
 * - si aucun symbole (toutes fréquences = 0) : retourne NULL
 * - si un seul symbole non nul : retourne ce noeud (ou un parent unique si souhaité)
 *
 * Utilise l'API du tas (heap.h) pour sélectionner à chaque itération les deux noeuds
 * de plus petite fréquence et les combiner.
 */
Noeud* construire_arbre_huffman(const unsigned long freq_table[256]);

/* Génère un tableau de codes (chaînes C) pour chaque octet.
 * - Retourne un tableau alloué de 256 pointeurs (char*). Chaque entrée est soit
 *   NULL (symbole absent) soit une chaîne NUL-terminée contenant '0'/'1'.
 * - L'appelant est responsable d'appeler liberer_codes() pour libérer la mémoire.
 *
 * Note : si l'arbre contient un seul symbole, son code sera "0" (convention).
 */
char** generer_codes(const Noeud *root);

/* Libère le tableau renvoyé par generer_codes (chaînes + tableau). */
void liberer_codes(char **codes);

/* Compte les fréquences d'un fichier binaire (octet par octet).
 * - path : chemin du fichier
 * - freq_table : tableau sur 256 cases (doit être alloué par l'appelant)
 * Retourne 0 si OK, -1 si erreur (ouverture/lecture).
 */
int compter_frequences_fichier(const char *path, unsigned long freq_table[256]);

/* Affiche l'arbre (affichage simple, pré-order) utile pour debug. */
void afficher_arbre(const Noeud *root, int depth);

#endif /* HUFFMAN_H */
