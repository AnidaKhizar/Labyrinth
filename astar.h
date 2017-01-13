#ifndef _ASTAR_H
#define _ASTAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "labyrinthAPI.h"
#include "update.h"

/*Structure contenant toutes les informations d'une case*/
typedef struct c{
  Pos pos;
  int cost;
  int heuristique;
  Pos prec;
  struct c* suiv;
}Case;


/* Fonction renvoyant un tableau de mouvements consécutifs a effectuer pour se rendre a l'arrivee
en passant par le plus court chemin possible */
t_move* a_etoile(Lab* laby, int size_x, int size_y);


/* Fonction reconstituant le chemin trouve a partir de la case Tresor afin de renvoyer la liste de mouvement a effectuer*/
t_move* reconstitution_chemin(Lab* laby, Case* closedList);


/*Fonction ajoutant un voisin de la case x a la liste chainee openList
Ici les elements de l'openList sont ranges dans l'ordre croissant de leur heuristique 
Si une position se trouve deja dans l'openList, on garde celle ayant l'heuristique la plus faible*/
Case* ajout_element(Case* openList,Case* x, Pos voisin, int size_x, int size_y, Pos tresor);

/*Fonction renvoyant un pointeur vers la case de la closedList se trouvant à la position p */
Case* trouver_element(Case* closedList, Pos p);

/* fonction liberant les openList et closedList*/
void liberer_memoire(Case* list);

/* Fonction retournant le minimum entre deux nombres */
int min(int a, int b);

/*Fonction estimant la distance la plus courte separant la case x de l'arrivee
 Ici on choisit la distance de manathan*/
int estimation_distance(Pos dep, Pos arr, int size_x, int size_y);

#endif
