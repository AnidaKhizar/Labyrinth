//
// ASTAR 
//
//
// Regroupe l'ensemble des fonctions mettant l'algorithme A* en oeuvre 
// Permet de renvoyer un tableau de mouvements a effectuer
// pour se rendre jusqu'au labyrinthe

#include "astar.h"
#include "update.h"
#include <stdio.h>
#include <stdlib.h>
#include "labyrinthAPI.h"

t_move* a_etoile(Lab* laby, int size_x, int size_y)
{
   
  // Initialsiation du tableau de mouvements a renvoyer 
  t_move* move;

  /* Initialisation de openList (qui contient juste la case depart) et closedList */
  Case* closedList = NULL;
  Case* openList = NULL;

  openList = (Case*)malloc(sizeof(Case)); 
  if (openList == NULL)
    {
      printf("Erreur dans le malloc de la fonction ajouter_element\n");
      return NULL;
    }
  openList-> pos = laby->play;
  openList-> cost = 0;
  openList-> heuristique = estimation_distance(laby->play, laby->Tresor, size_x, size_y);
  openList-> prec = laby->play; //on choisit une position precedente arbitraire
  openList-> suiv = NULL;
  
  /*Initialisation des positions des voisins de la case exploree */
  Pos voisin_h;
  Pos voisin_g;
  Pos voisin_b;
  Pos voisin_d;


  /* pour plus de lisibilite, on stocke la position de la case exploree dans 2 variables x et y */
  int x;
  int y;

  /* booleen etant faux si on a trouve un chemin, vrai sinon */
  int chemin_non_trouve = 1;
  
   /* variable temporaire utile pour transferer un element de l'openList a la closedList ou pour chercher un element dans la closedList */
  Case* tmp;

  /*execution de l'agorithme a* */

  // Tant que l'openList est non vide et qu'on n'a pas trouve de chemin
  while (openList && chemin_non_trouve)
    {
      // on transfert la case exploree de l'openList a la closedList
      tmp = closedList;
      closedList = openList;
      openList = openList -> suiv;
      closedList->suiv = tmp;
 
      /* stockage de la position de la case exploree */
      x = closedList->pos.x;
      y = closedList->pos.y;
 
      /* on verifie que l'element tranfere est l'objectif d'arrivee */
      if (x == laby->Tresor.x && y == laby->Tresor.y)
	{
	  // on vient de trouver un chemin
	  chemin_non_trouve = 0;
	  
	  /* on reconstitue chemin afin de renvoyer les mouvements a effectuer dans un tableau*/
	  move = reconstitution_chemin(laby, closedList);
	  
	}
      
      /* si on n'a pas trouve le tresor, on ajoute les voisins de la case exploree à l'openList*/
      else
	{
	  /* stockage des positions des voisins de la case exploree */
	  voisin_h.x = reste(x - 1, size_y);
	  voisin_h.y = y;
	  voisin_b.x = reste(x + 1, size_y);
	  voisin_b.y = y;
	  voisin_g.x = x;
	  voisin_g.y = reste(y - 1, size_x);
	  voisin_d.x = x;
	  voisin_d.y = reste(y + 1, size_x);
	  
	  /* on ajoute le voisin du haut (si ce n'est pas un mur et qu'il n'a pas deja ete explore)*/
	  if (laby -> lab[ size_x * voisin_h.x + y ] == 0 && trouver_element(closedList, voisin_h) == NULL)
	    openList = ajout_element(openList, closedList, voisin_h, size_x, size_y, laby-> Tresor);	    
	  
	  /* on ajoute le voisin du bas (si ce n'est pas un mur et qu'il n'a pas deja ete explore)*/
	  if (laby -> lab[ size_x * voisin_b.x + y ] == 0 && trouver_element(closedList, voisin_b) == NULL)
	    openList = ajout_element(openList, closedList, voisin_b, size_x, size_y, laby-> Tresor);
	  
	  /* on ajoute le voisin de gauche (si ce n'est pas un mur et qu'il n'a pas deja ete explore)*/
	  if (laby -> lab[ size_x * x + voisin_g.y ] == 0  && trouver_element(closedList, voisin_g) == NULL)
	    openList = ajout_element(openList, closedList, voisin_g, size_x, size_y, laby-> Tresor);
		  
	  /* on ajoute le voisin de droite (si ce n'est pas un mur et qu'il n'a pas deja ete explore) */
	  if (laby -> lab[ size_x * x + voisin_d.y ] == 0 && trouver_element(closedList, voisin_d) == NULL)
	    openList = ajout_element(openList, closedList, voisin_d, size_x, size_y, laby-> Tresor);
	}
	  
    }

  liberer_memoire(openList);
  liberer_memoire(closedList);

  /* Si on est sorti de la boucle sans trouver de chemins on renvoie un mouvement unique DO_NOTHING*/
  if (chemin_non_trouve)
    {
      move = (t_move*)malloc(sizeof(t_move));
      move[0].type = 8;
      move[0].value = 0;
      
    }
  
  return move;
}


t_move* reconstitution_chemin(Lab* laby, Case* closedList)
{
  // Initialsiation du tableau de mouvements a renvoyer 
  t_move* move = NULL;

  /* variables temporaires utilises pour parcourir la closedList */
  Case *tmp = closedList;
  
  /*Entier permettant de connaître le nombre de cases parcourues pendant le chemin */
  int nb_case = 1;

  int i; // compteur

  /* Tant qu'on n'a pas remonte jusqu'a la position de depart, on continue de rebrousser chemin pour trouver le nombre de cases parcourues dans le chemin*/
  while(tmp->prec.x != laby->play.x || tmp->prec.y!= laby->play.y)
    {
      tmp = trouver_element(closedList, tmp->prec);
      nb_case++;
    }
  
  /* On alloue la memoire necessaire pour le tableau de mouvements */
  move = (t_move*)malloc(nb_case * sizeof(t_move));
  
  /* on remplit desormais le tableau de mouvements: on part de la derniere case et on essaye de retrouver pour chaque case le mouvement qui correspond pour passer de cette case à son predecesseur*/ 

  tmp = closedList;	  
  for (i=nb_case-1; i>=0; i--)
    {
      // en bas
      if(tmp->pos.x - tmp->prec.x == 1 || tmp->pos.x - tmp->prec.x < -1)
	{
	  move[i].type = 5;
	  move[i].value = 0;		 
	}
      //en haut
      if(tmp->pos.x - tmp->prec.x == -1 || tmp->pos.x - tmp->prec.x > 1)
	{
	  move[i].type = 4;
	  move[i].value = 0;		 
	}
      // a droite	      
      if(tmp->pos.y - tmp->prec.y == 1 || tmp->pos.y - tmp->prec.y < -1)
	{
	  move[i].type = 7;
	  move[i].value = 0;		 
	}
      // a gauche
      if(tmp->pos.y - tmp->prec.y == -1 || tmp->pos.y - tmp->prec.y > 1)
	{
	  move[i].type = 6;
	  move[i].value = 0;		 
	}
      
      // on passe à la case d'avant
      tmp = trouver_element(closedList, tmp->prec);
      
    }
  return move;
}


Case* ajout_element(Case* openList,Case* x, Pos voisin, int size_x, int size_y, Pos tresor)
{
  Case* cur = openList; //curseur qui va parcourir chaque élément de la liste
  Case* prec = NULL; //on garde en mémoire l'élément qui se trouve avant celui qui est parcouru

  /*On crée maintenant l'élément à ajouter: on alloue d'abord de la mémoire puis on initialise chacun de ses champs*/

  Case *element_ajout = NULL; 
  element_ajout = (Case*)malloc(sizeof(Case)); 
  if (element_ajout == NULL)
    {
      printf("Erreur dans le malloc de la fonction ajouter_element\n");
      return NULL;
    }
  
  element_ajout-> pos = voisin;
  element_ajout-> cost = x->cost +1;
  element_ajout-> heuristique = element_ajout->cost + estimation_distance(voisin, tresor, size_x, size_y);
  element_ajout-> prec = x->pos;
  element_ajout-> suiv = NULL;

  /* Si la liste est vide, on retourne simplement l'élément à ajouter*/
  if (openList == NULL)
    return element_ajout;
  
  /* Si l'element que l'on veut ajouter se trouve deja dans l'openList mais avec un cout plus eleve, alors on modifie la case deja existante */
  cur = trouver_element(openList, voisin);
  if (cur)
    {
      if (cur->cost >= element_ajout->cost) 
	{
	  cur -> cost = element_ajout -> cost;
	  cur -> heuristique = element_ajout -> heuristique;
	  cur -> prec = element_ajout -> prec;
	}
      free(element_ajout);
      return openList;
    }

  /*Cas où on doit placer l'élément en tête de liste*/
  if (openList->heuristique > element_ajout->heuristique)  
    {
      element_ajout -> suiv = openList;
      return element_ajout;
    }

  /*On parcourt les éléments de la liste tant qu'on n'a pas trouvé un entier plus grand que l'heuristique du voisin a ajouter*/
  cur = openList;
  while ( (cur!=NULL) && (cur->heuristique <= element_ajout->heuristique) )
    {     
      prec=cur;
      cur= cur->suiv;
    }

  /*On a trouvé l'emplacement adéquat de notre élément dans la liste: on l'ajoute */
  prec -> suiv = element_ajout;
  element_ajout -> suiv = cur;
  return openList;
}


Case* trouver_element(Case* closedList, Pos p)
{
  Case *tmp = closedList; //case parcourant la closedList  
  while(tmp)
    {
      // on vient de trouver l'element: on le renvoie
      if( tmp -> pos.x == p.x && tmp->pos.y == p.y)
	{
	  return tmp;	  
	}
      tmp= tmp->suiv;
    }

  // on n'a pas trouve d'éléments: on renvoie le pointeur NULL
  return NULL;
}


void liberer_memoire(Case* list)
{
  Case* tmp=list;
  // on parcourt chaque case de la liste pour les liberer chacune
  while(list)
    {
      tmp = tmp->suiv;
      free(list);
      list = tmp;
    }
}


int min(int a, int b)
{
  if (a>b)
    return b;
  return a;
}


int estimation_distance(Pos dep, Pos arr, int size_x, int size_y)
{
  int x = min( size_y - arr.x + dep.x, abs(arr.x - dep.x) );               /*on regarde s'il est plus rapide d'aller vers la droite ou vers la gauche */
  int y = min( size_x - arr.y + dep.y, abs(arr.y - dep.y) );               /*on regarde s'il est plus rapide de passer par en haut ou par en bas */
  return x+y;
}
