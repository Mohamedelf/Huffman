/*
 * huffman.c
 *
 * Implémentation des primitives Huffman :
 * - création / destruction de Noeud
 * - construction d'arbre à partir d'un tableau de fréquences
 * - génération d'une table de codes (chaînes "0"/"1")
 * - comptage de fréquences depuis un fichier
 *
 * Ce fichier utilise l'API du tas définie dans heap.h pour sélectionner
 * les deux noeuds minima à chaque étape.
 */

#include "huffman.h"
#include "heap.h"    /* API du tas : creer_tas_min, inserer_tas, extraire_min, detruire_tas, taille_tas */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h> /* pour CHAR_BIT, si nécessaire */
static char* my_strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char* copy = malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}
#define strdup my_strdup

/* ---------- Création / destruction de noeuds ---------- */

Noeud* creer_noeud(unsigned char c, unsigned long freq, Noeud *left, Noeud *right) {
    Noeud *n = (Noeud*) malloc(sizeof(Noeud));
    if (!n) return NULL;
    n->c = c;
    n->freq = freq;
    n->left = left;
    n->right = right;
    n->leaf = (left == NULL && right == NULL) ? 1 : 0;
    return n;
}

/* Libération récursive de l'arbre (post-order) */
void detruire_arbre(Noeud *root) {
    if (!root) return;
    detruire_arbre(root->left);
    detruire_arbre(root->right);
    free(root);
}

/* ---------- Construction de l'arbre de Huffman ---------- */

/* Construire l'arbre en utilisant un min-heap de Noeud*.
 *
 * Algorithme :
 * 1) Pour chaque symbole avec freq>0 : créer une feuille et l'insérer dans le tas.
 * 2) Tant que le tas a 2 éléments ou plus :
 *      a = extraire_min()
 *      b = extraire_min()
 *      créer un noeud interne p avec freq = a.freq + b.freq, left=a, right=b
 *      inserer p dans le tas
 * 3) Si le tas est vide -> retourne NULL
 *    Si le tas a un seul élément -> extrait et retourne cet élément (racine)
 */
Noeud* construire_arbre_huffman(const unsigned long freq_table[256]) {
    if (!freq_table) return NULL;

    TasMin *tas = creer_tas_min(16);
    if (!tas) return NULL;

    /* Étape 1 : insérer toutes les feuilles (symboles existants) */
    for (int i = 0; i < 256; ++i) {
        if (freq_table[i] > 0) {
            Noeud *leaf = creer_noeud((unsigned char)i, freq_table[i], NULL, NULL);
            if (!leaf) {
                /* en cas d'erreur d'allocation : cleanup et sortie */
                detruire_tas(tas);
                return NULL;
            }
            if (inserer_tas(tas, leaf) != 0) {
                /* échec d'insertion (OOM) : libérer l'arbre créé jusqu'ici */
                detruire_arbre(leaf); /* free leaf */
                /* Vider et libérer ce qu'on a inséré avant */
                while (taille_tas(tas) > 0) {
                    Noeud *n = extraire_min(tas);
                    detruire_arbre(n);
                }
                detruire_tas(tas);
                return NULL;
            }
        }
    }

    /* Cas particulier : aucun symbole */
    if (taille_tas(tas) == 0) {
        detruire_tas(tas);
        return NULL;
    }

    /* Cas particulier : un seul symbole -> retourner directement la feuille */
    if (taille_tas(tas) == 1) {
        Noeud *single = extraire_min(tas);
        detruire_tas(tas);
        return single;
    }

    /* Étape 2 : construire l'arbre en combinant deux à deux */
    while (taille_tas(tas) > 1) {
        Noeud *a = extraire_min(tas);
        Noeud *b = extraire_min(tas);
        if (!a || !b) {
            /* situation improbable mais sécure : nettoyage */
            if (a) detruire_arbre(a);
            if (b) detruire_arbre(b);
            while (taille_tas(tas) > 0) {
                Noeud *n = extraire_min(tas);
                detruire_arbre(n);
            }
            detruire_tas(tas);
            return NULL;
        }
        unsigned long somme = a->freq + b->freq;
        Noeud *parent = creer_noeud(0 /* non significatif */, somme, a, b);
        if (!parent) {
            /* error: cleanup */
            detruire_arbre(a);
            detruire_arbre(b);
            while (taille_tas(tas) > 0) {
                Noeud *n = extraire_min(tas);
                detruire_arbre(n);
            }
            detruire_tas(tas);
            return NULL;
        }
        if (inserer_tas(tas, parent) != 0) {
            detruire_arbre(parent);
            /* cleanup */
            while (taille_tas(tas) > 0) {
                Noeud *n = extraire_min(tas);
                detruire_arbre(n);
            }
            detruire_tas(tas);
            return NULL;
        }
    }

    /* Le dernier élément du tas est la racine de l'arbre */
    Noeud *root = extraire_min(tas);
    detruire_tas(tas);
    return root;
}

/* ---------- Génération de codes (table binaire sous forme de chaînes "0"/"1") ---------- */

/* Helper récursif : parcours en profondeur, construit la chaîne binaire dans buf.
 * - buf est un tableau de caractères contenant '0'/'1' (non NUL-terminé pendant la construction)
 * - depth est la longueur courante (nombre de bits écrits)
 * - codes est le tableau de 256 pointeurs; lorsque l'on rencontre une feuille on strdup la chaîne
 */
static void generer_codes_rec(const Noeud *node, char *buf, int depth, char **codes) {
    if (!node) return;

    if (node->leaf) {
        /* feuille : terminer la chaîne et la dupliquer */
        if (depth == 0) {
            /* cas spécial : arbre réduit à une seule feuille -> convention : code "0" */
            codes[node->c] = (char*) malloc(2);
            if (codes[node->c]) {
                codes[node->c][0] = '0';
                codes[node->c][1] = '\0';
            }
        } else {
            buf[depth] = '\0';
            codes[node->c] = strdup(buf);
        }
        return;
    }

    /* parcours gauche = '0' */
    buf[depth] = '0';
    generer_codes_rec(node->left, buf, depth + 1, codes);

    /* parcours droit = '1' */
    buf[depth] = '1';
    generer_codes_rec(node->right, buf, depth + 1, codes);
}

/* Génère et retourne un tableau alloué de 256 chaînes (ou NULL pour symboles absents).
 * L'appelant doit appeler liberer_codes() pour libérer la mémoire.
 */
char** generer_codes(const Noeud *root) {
    /* allouer tableau 256 pointeurs initialisés à NULL */
    char **codes = (char**) calloc(256, sizeof(char*));
    if (!codes) return NULL;

    if (!root) return codes; /* vide : tableau rempli de NULL */

    /* profondeur maximale raisonnable : 256 (sécurité) */
    char buf[512]; /* assez grand pour la profondeur maximale (sûr) */

    /* cas spécial : arbre avec une seule feuille */
    if (root->leaf) {
        codes[root->c] = strdup("0");
        return codes;
    }

    generer_codes_rec(root, buf, 0, codes);
    return codes;
}

/* Libération du tableau de codes */
void liberer_codes(char **codes) {
    if (!codes) return;
    for (int i = 0; i < 256; ++i) {
        if (codes[i]) {
            free(codes[i]);
            codes[i] = NULL;
        }
    }
    free(codes);
}

/*Comptage de fréquences depuis un fichier  */

int compter_frequences_fichier(const char *path, unsigned long freq_table[256]) {
    if (!path || !freq_table) return -1;

    /* initialiser le tableau */
    for (int i = 0; i < 256; ++i) freq_table[i] = 0UL;

    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    /* lire octet par octet */
    unsigned char buffer[4096];
    size_t r;
    while ((r = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        for (size_t i = 0; i < r; ++i) {
            freq_table[ buffer[i] ]++;
        }
    }

    if (ferror(f)) {
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

/*Affichage debug de l'arbre  */

void afficher_arbre_recursive(const Noeud *node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; ++i) putchar(' ');
    if (node->leaf) {
        /* afficher l'octet en tant qu'entier et (si imprimable) en caractère */
        unsigned char ch = node->c;
        if (ch >= 32 && ch <= 126) {
            printf("leaf '%c' (0x%02X) : freq=%lu\n", ch, ch, node->freq);
        } else {
            printf("leaf 0x%02X : freq=%lu\n", ch, node->freq);
        }
    } else {
        printf("node : freq=%lu\n", node->freq);
    }
    afficher_arbre_recursive(node->left, depth + 2);
    afficher_arbre_recursive(node->right, depth + 2);
}

void afficher_arbre(const Noeud *root, int depth) {
    if (!root) {
        printf("<arbre vide>\n");
        return;
    }
    afficher_arbre_recursive(root, depth);
}
