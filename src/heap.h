#ifndef HEAP_H /*Ces lignes empêchent le compilateur d'inclure ce fichier plusieurs fois (ce qui causerait des erreurs de redéfinition)*/
#define HEAP_H

/* declaration du Noeud (la structure du Noeud est défini dans huffman.h) */
typedef struct Noeud Noeud;

/* tableau dynamique de pointeurs vers Noeud */
typedef struct {
    int taille;      /* nombre d'éléments actuels */
    int capacite;    /* capacité du tableau */
    Noeud **tab;     /* tableau de pointeurs vers Noeud */
} TasMin;

/* Crée un tas min vide avec la capacité initiale demandée.
 * Retourne NULL en cas d'échec d'allocation.
 */
TasMin* creer_tas_min(int capacite_initiale);

/* Détruit le tas (ne libère pas les Noeud* contenus — gestion externe).
 * Passe NULL en paramètre pour être tolérant.
 */
void detruire_tas(TasMin *tas);

/* Retourne le nombre d'éléments dans le tas */
int taille_tas(const TasMin *tas);

/* Insère un noeud dans le tas (heapify up).
 * Retourne 0 si OK, -1 si erreur (allocation).
 */
int inserer_tas(TasMin *tas, Noeud *n);

/* Extrait et retourne le Noeud* de fréquence minimale.
 * Retourne NULL si le tas est vide.
 */
Noeud* extraire_min(TasMin *tas);

/* Fonction d'entassement (heapify down) depuis l'indice i.
 * Utilisée en interne mais rendue publique pour debug/tests.
 */
void entasser_min(TasMin *tas, int i);

/* Affiche le contenu du tas (indices et fréquences) — utile pour le debug.
 * Nécessite que Noeud ait un champ 'freq' ; si non, adapte l'affichage.
 */
void afficher_tas(const TasMin *tas);

#endif 
