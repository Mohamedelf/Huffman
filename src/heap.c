#include "heap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * On inclut huffman.h pour la définition complète de Noeud.
 * Si huffman.h n'existe pas encore au moment de compiler, veille à
 * l'ajouter ou à fournir une déclaration compatible :
 *
 * typedef struct Noeud {
 *     unsigned char ch;
 *     unsigned int freq;
 *     struct Noeud *gauche, *droite;
 * } Noeud;
 */
#include "huffman.h"


/* fonctions utilitaires internes */

/* échange deux pointeurs de Noeud */
static void swap_noeuds(Noeud **a, Noeud **b) {
    Noeud *tmp = *a;
    *a = *b;
    *b = tmp;
}

/* doubler la capacité du tas; retourne 0 si OK, -1 si erreur */
static int agrandir_tas(TasMin *tas) {
    int nouvelle = (tas->capacite == 0) ? 4 : tas->capacite * 2;
    Noeud **tmp = (Noeud**) realloc(tas->tab, sizeof(Noeud*) * nouvelle);
    if (!tmp) return -1;
    tas->tab = tmp;
    tas->capacite = nouvelle;
    return 0;
}

/* indices utilitaires */
static inline int parent_idx(int i) { return (i - 1) / 2; }
static inline int gauche_idx(int i) { return 2 * i + 1; }
static inline int droite_idx(int i) { return 2 * i + 2; }

/*API publique*/

TasMin* creer_tas_min(int capacite_initiale) {
    TasMin *tas = (TasMin*) malloc(sizeof(TasMin));
    if (!tas) return NULL;
    tas->taille = 0;
    tas->capacite = (capacite_initiale > 0) ? capacite_initiale : 4;
    tas->tab = (Noeud**) malloc(sizeof(Noeud*) * tas->capacite);
    if (!tas->tab) {
        free(tas);
        return NULL;
    }
    return tas;
}

void detruire_tas(TasMin *tas) {
    if (!tas) return;
    free(tas->tab);
    free(tas);
}

int taille_tas(const TasMin *tas) {
    if (!tas) return 0;
    return tas->taille;
}

/* entasser_min : heapify-down depuis l'indice i */
void entasser_min(TasMin *tas, int i) {
    if (!tas) return;
    int n = tas->taille;
    int plus_petit = i;

    for (;;) {
        int g = gauche_idx(i);
        int d = droite_idx(i);
        plus_petit = i;

        if (g < n && tas->tab[g]->freq < tas->tab[plus_petit]->freq) {
            plus_petit = g;
        }
        if (d < n && tas->tab[d]->freq < tas->tab[plus_petit]->freq) {
            plus_petit = d;
        }

        if (plus_petit != i) {
            swap_noeuds(&tas->tab[i], &tas->tab[plus_petit]);
            i = plus_petit;
        } else {
            break;
        }
    }
}

/* inserer_tas : ajoute un noeud et fait heapify-up */
int inserer_tas(TasMin *tas, Noeud *n) {
    if (!tas || !n) return -1;
    if (tas->taille >= tas->capacite) {
        if (agrandir_tas(tas) != 0) return -1;
    }
    int idx = tas->taille;
    tas->tab[idx] = n;
    tas->taille++;

    /* heapify-up */
    while (idx > 0) {
        int p = parent_idx(idx);
        if (tas->tab[p]->freq > tas->tab[idx]->freq) {
            swap_noeuds(&tas->tab[p], &tas->tab[idx]);
            idx = p;
        } else {
            break;
        }
    }
    return 0;
}

/* extraire_min : retire la racine et la retourne */
Noeud* extraire_min(TasMin *tas) {
    if (!tas || tas->taille == 0) return NULL;
    Noeud *min = tas->tab[0];
    tas->taille--;
    if (tas->taille > 0) {
        tas->tab[0] = tas->tab[tas->taille];
        entasser_min(tas, 0);
    }
    return min;
}

/* afficher_tas : affichage pour le debug */
void afficher_tas(const TasMin *tas) {
    if (!tas) {
        printf("[tas NULL]\n");
        return;
    }
    printf("Tas (taille=%d, capacite=%d)\n", tas->taille, tas->capacite);
    for (int i = 0; i < tas->taille; ++i) {
        Noeud *n = tas->tab[i];
        if (!n) {
            printf(" idx %d : NULL\n", i);
            continue;
        }
        /* si Noeud a ch et freq, on les affiche proprement */
        unsigned int f = n->freq;
        unsigned int ch = (unsigned int) n->c;
        if (ch >= 32 && ch <= 126) {
            printf(" idx %2d : '%c' (freq=%u)\n", i, (char)ch, f);
        } else {
            printf(" idx %2d : (ch=%u) freq=%u\n", i, ch, f);
        }
    }
}
