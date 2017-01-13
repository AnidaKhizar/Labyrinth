//
// UPDATE
//
// Fichier regroupant l'ensemble des fonctions
// permettant de mettre à jour le labyrinthe
// à chaque tour de jeu

#include "update.h"

int reste(int a, int b)
{
  return ( (a%b + b) % b);
}

int lab_update(char* lab,int size_x, int size_y, t_move m)
{
  /* indice parcourant la ligne ou la colonne à bouger*/
  int i;

  int tmp; // variable temporaire contenant un élément du tableau

  /*on deplace tous les elements de la ligne m.value vers la gauche */ 
  if (m.type == 0)
    {
      tmp = lab[size_x * m.value]; //tmp contient le premier element de la ligne à bouger
      for (i=0; i< size_x - 1; i++)
	{
	  lab[ size_x * m.value + i] =  lab[ size_x * m.value + i +1 ];
	}
      lab[ size_x * (m.value+1) -1 ] = tmp;
    }

   /*on deplace tous les elements de la ligne m.value vers la droite */
    if (m.type == 1)
    {
      tmp = lab[size_x * (m.value+1) -1]; //tmp contient le dernier element de la ligne à bouger
      for (i=size_x-1; i> 0; i--)
	{
	  lab[ size_x * m.value + i] =  lab[ size_x * m.value + i -1 ];
	}
      lab[ size_x * m.value ] = tmp;
    }

     /*on deplace tous les elements de la colonne m.value vers le haut */
    if (m.type == 2)
      {
	tmp = lab[m.value]; //tmp contient le premier element de la colonne à bouger
	for (i=0; i< size_y - 1; i++)
	  {
	    lab[ size_x * i + m.value] =  lab [size_x * (i +1) + m.value];
	  }
	lab[ size_x *(size_y -1) + m.value ] = tmp;
      }

     /*on deplace tous les elements de la colonne m.value vers le bas */
    if (m.type == 3)
      {
	tmp = lab[size_x *(size_y -1) + m.value]; //tmp contient le dernier element de la colonne à bouger
	for (i=size_y-1; i>0; i--)
	  {
	    lab[ size_x * i + m.value] =  lab [size_x * (i -1) + m.value];
	  }
	lab[m.value] = tmp;
      }
    return 0;
    
}
     
int pos_update_rotation(Pos* pos,int size_x, int size_y, t_move m)
{
  /* si on se trouve sur la meme ligne qui a ete translaté vers la gauche*/
  if (m.type == 0 && m.value == pos->x)
    pos->y = reste(pos->y - 1, size_x);

  /* si on se trouve sur la meme ligne qui a ete translaté vers la droite*/
  if (m.type == 1 && m.value == pos->x)
    pos->y = reste(pos->y + 1, size_x);

  /* si on se trouve sur la meme colonne qui a ete translaté vers le haut*/
  if (m.type == 2 && m.value == pos->y)
    pos->x = reste(pos->x - 1,size_y);

  /* si on se trouve sur la meme colonne qui a ete translaté vers le bas*/
  if (m.type == 3 && m.value == pos->y)
    pos->x = reste(pos->x + 1 , size_y);

  return 0;

}

int pos_update_translation(Pos* pos,int size_x, int size_y, t_move m)
{
  //en haut
  if (m.type == 4)
    pos->x = reste(pos->x - 1,size_y); 

  //en bas   
  if (m.type == 5) 
    pos->x = reste(pos->x + 1,size_y);

  //a gauche
  if (m.type == 6)
    pos->y = reste(pos->y - 1,size_x);

  //a droite
  if (m.type == 7)
    pos->y = reste(pos->y + 1,size_x);
  
  return 0;  
}
