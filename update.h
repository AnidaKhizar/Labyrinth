#ifndef _UPDATE_H
#define _UPDATE_H

#include <stdlib.h>
#include "labyrinthAPI.h"

/* Structure stockant une position dans le labyrinthe */ 
typedef struct position{
  int x; 
  int y;
}Pos;

/* Structure stockant les données du labyrinthe */
typedef struct donnees_lab{
  char *lab;
  int energy;
  Pos play;
  Pos adv;
  Pos Tresor;
}Lab;


/*Fonction renvoyant le reste de la division euclidienne de a par b:
ici, l'opérateur % ne convient pas car il peut renvoyer des entiers négatifs!
Or le reste d'une division euclidienne est nécessairement positif
 */
int reste(int a, int b);

/* Fonction mettant à jour le labyrinthe s'il y a eu rotation */
int lab_update(char* lab,int size_x, int size_y, t_move m);

/* Fonction mettant à jour une position dans le labyrinthe
(qu'il s'agisse de celle du trésor, de la notre ou celle de l'adversaire)
s'il y a eu rotation et qu'on se trouve sur la même ligne/colonne que celle qui est bougée  */
int pos_update_rotation(Pos* pos,int size_x, int size_y, t_move m);


/* Fonction mettant à jour une position dans le labyrinthe
(qu'il s'agisse de la notre ou celle de l'adversaire)
si on s'est déplacé sur une autre case  */
int pos_update_translation(Pos* pos,int size_x, int size_y, t_move m);

#endif
