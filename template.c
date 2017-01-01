//
// TEMPLATE 
//
//
// Permet de jouer un seul tour (en ne faisant rien s'il commence ou en 
// réceptionnant le coup de l'adversaire s'il ne commence pas) 
// et termine le jeu.
// Ce programme vous sert de base pour construire votre propre programme



#include <stdio.h>
#include <stdlib.h>
#include "labyrinthAPI.h"
#include <unistd.h>
#include <string.h>
#include <time.h>


extern int debug;	/* hack to enable debug messages */

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

/*Structure contenant toutes les informations d'une case*/
typedef struct c{
  Pos pos;
  int cost;
  int heuristique;
  Pos prec;
  struct c* suiv;
}Case;


/*Fonction renvoyant le reste de la division euclidienne de a par b:
ici, l'opérateur % ne convient pas car il peut renvoyer des entiers négatifs!
Or le reste d'une division euclidienne est nécessairement positif
 */
int reste(int a, int b)
{
  return ( (a%b + b) % b);
}

/* Fonction retournant le minimum entre deux nombres */
int min(int a, int b)
{
  if (a>b)
    return b;
  return a;
}

/*Fonction estimant la distance la plus courte separant la case x de l'arrivee
 Ici on choisit la distance de manathan*/

int estimation_distance(Pos dep, Pos arr, int size_x, int size_y)
{
  int x = min( abs(size_x - arr.x + dep.x), abs(arr.x - dep.x) );               /*on regarde s'il est plus rapide d'aller vers la droite ou vers la gauche */
  int y = min( abs(size_y - arr.y + dep.y), abs(arr.y - dep.y) );               /*on regarde s'il est plus rapide de passer par en haut ou par en bas */
  return x+y;
}

/*Fonction permettant de savoir si un element se trouve deja dans la closedList, pour eviter de l'ajouter dans l'openList */
Case* trouver_element(Case* closedList, Pos p)
{
  Case *tmp = closedList;
  while(tmp)
    {
      if( tmp -> pos.x == p.x && tmp->pos.y == p.y)
	return tmp;
      tmp= tmp->suiv;
    }
  return NULL;
}



/*Fonction ajoutant le voisin de la case x a la liste chainee openList
Ici les elements de l'openList sont ranges dans l'ordre croissant de leur heuristique */

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
  
  /*Cas où on doit placer l'élément en tête de liste*/
  if (openList->heuristique >= element_ajout->heuristique)  
    {
      element_ajout -> suiv = openList;
      return element_ajout;
    }

  /*On parcourt les éléments de la liste tant qu'on n'a pas trouvé un entier plus grand que l'heuristique du voisin a ajouter*/
  while ( (cur!=NULL) && (cur->heuristique < element_ajout->heuristique) )
    {
      prec=cur;
      cur= cur->suiv;
    }

  /*On a trouvé l'emplacement adéquat de notre élément dans la liste: on l'ajoute */
  prec -> suiv = element_ajout;
  element_ajout -> suiv = cur;
  return openList;
}


/* Fonction mettant à jour le labyrinthe s'il y a eu rotation */
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
     
/* Fonction mettant à jour une position dans le labyrinthe
(qu'il s'agisse de celle du trésor, de la notre ou celle de l'adversaire)
s'il y a eu rotation et qu'on se trouve sur la même ligne/colonne que celle qui est bougée  */

int pos_update_rotation(Pos* pos,int size_x, int size_y, t_move m)
{
  
  if (m.type == 0 && m.value == pos->x)
    pos->y = reste(pos->y - 1, size_x);
    
  if (m.type == 1 && m.value == pos->x)
    pos->y = reste(pos->y + 1, size_x);
    
  if (m.type == 2 && m.value == pos->y)
    pos->x = reste(pos->x - 1,size_y);
    
  if (m.type == 3 && m.value == pos->y)
    pos->x = reste(pos->x + 1 , size_y);

  return 0;

}

/* Fonction mettant à jour une position dans le labyrinthe
(qu'il s'agisse de la notre ou celle de l'adversaire)
si on s'est déplacé sur une autre case  */

int pos_update_translation(Pos* pos,int size_x, int size_y, t_move m)
{
  if (m.type == 4)
    pos->x = reste(pos->x - 1,size_y); 
	     
  if (m.type == 5) 
    pos->x = reste(pos->x + 1,size_y);
  
  if (m.type == 6)
    pos->y = reste(pos->y - 1,size_x);
  
  if (m.type == 7)
    pos->y = reste(pos->y + 1,size_x);
  
  return 0;  
}


/* Fonction renvoyant un mouvement aléatoire autorisé*/
t_move make_move(Lab* laby, int size_x, int size_y)
{
  srand(time(NULL));
  /* booleen verifiant la validite du mouvement tiré aleatoirement */
  int check = 0;
 
  /* mouvement */
  t_move m;
 
  /*Position de notre joueur */
  int x = laby -> play.x ;
  int y = laby -> play.y;
 
  /* tant que le mouvement tiré n'est pas valide, on tire un autre mouvement aleatoirement */
  while(check == 0)
    {
      /* On tire un mouvement au hasard */
      
        m.type = rand() % 9;
     
      /* Si le mouvement est une rotation, on tire au hasard la ligne ou la colonne à tourner */

      if(m.type >= 0 && m.type <= 3 && laby->energy >= 5)
	{
	  laby -> energy -= 5;
	  check = 1;

	  if (m.type == 0 || m.type == 1)
	    m.value = rand() % size_y;
	  
	  else
	    m.value = rand() % size_x;
	   
	}
     

      /*Sinon on vérifie que le joueur a le droit d'effectuer le mouvement tiré au sort: 
	il ne doit pas y avoir de murs à l'endroit où l'on se rend */
      else 
	{
	  if ( (m.type == 4 && laby -> lab[ size_x * reste(x-1,size_y) + y ] == 0)
	    || (m.type == 5 && laby -> lab[ size_x * reste(x+1,size_y) + y ] == 0)
	    || (m.type == 6 && laby -> lab[ size_x * x + reste(y - 1,size_x) ] == 0)
	    || (m.type == 7 && laby -> lab[ size_x * x + reste(y + 1,size_x) ] == 0)  )

	    {
	      check = 1 ;
	      (laby -> energy) ++;
	    }
	}

    }
    
  return m;
  
} 

/* fonction liberant les openList et closedList*/
void liberer_memoire(Case* list)
{
  Case* tmp=list;
  while(list)
    {
      tmp = tmp->suiv;
      free(list);
      list = tmp;
    }
}

int a_etoile(Lab* laby, int size_x, int size_y)
{
   
  /* Initialisation de openList (qui contient juste la case deaprt) et closedList */
  Case* closedList = NULL;
  Case* openList = NULL;

  openList = (Case*)malloc(sizeof(Case)); 
  if (openList == NULL)
    {
      printf("Erreur dans le malloc de la fonction ajouter_element\n");
      return 1;
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
  
  /* variables temporaires utiles pour transferer un element de l'openList a la closedList ou pour chercher un element dans la closedList */
  Case* tmp;
  Pos pos_tmp;
  
  /*execution de l'agorithme a* */
  while (openList)
    {
      /*on transfert le plus petit element (donc le premier) de l'openList dans la closedList*/
      tmp = closedList;
      closedList = openList;
      openList = openList -> suiv;
      closedList->suiv = tmp;
      
      /* stockage de la position de la case exploree */
      x = closedList->pos.x;
      y = closedList->pos.y;

      /* stockage des positions des voisins de la case exploree */
      voisin_h.x = reste(x - 1, size_y);
      voisin_h.y = y;
      voisin_b.x = reste(x + 1, size_y);
      voisin_b.y = y;
      voisin_g.x = x;
      voisin_g.y = reste(y - 1, size_x);
      voisin_d.x = x;
      voisin_d.y = reste(y + 1, size_x);
      
      /* on verifie que l'element tranfere est l'objectif d'arrivee */
      if (x == laby->Tresor.x && y == laby->Tresor.y)
	{
	   /* on reconstitue chemin : pour le moment on ne fait qu'afficher le chemin emprunté par l'algorithme*/

	  
	  /* Stockage de la case et de la position de la case dans des variables temporaires*/ 
	  tmp = closedList;
	  pos_tmp.x = x;
	  pos_tmp.y = y;
	  
	  printf("%d %d\n",x,y);
	  
	  /* Tant qu'on n'a pas remonte jusqu'a la position de depart, on affiche */
	  while(pos_tmp.x!= laby->play.x && pos_tmp.y!= laby->play.y)
	    {
	      printf("%d %d\n", tmp ->prec.x, tmp->prec.y);
	      pos_tmp.x = tmp ->prec.x;
	      pos_tmp.y = tmp->prec.y;
	      tmp = trouver_element(closedList, pos_tmp); 
	    }
	}
      else
	{
	   /* on ajoute le voisin du haut (si ce n'est pas un mur et qu'il n'a pas deja ete explore) a l'openList*/
	  if (laby -> lab[ size_x * voisin_h.x + y ] == 0 && trouver_element(closedList, voisin_h) == NULL)
	    {
	      openList = ajout_element(openList, closedList, voisin_h, size_x, size_y, laby-> Tresor);
	      
	    }
	  /* on ajoute le voisin du bas (si ce n'est pas un mur et qu'il n'a pas deja ete explore) a l'openList*/
	  if (laby -> lab[ size_x * voisin_b.x + y ] == 0 && trouver_element(closedList, voisin_b) == NULL)
	    {
	       openList = ajout_element(openList, closedList, voisin_b, size_x, size_y, laby-> Tresor);
	    }
	  /* on ajoute le voisin de gauche (si ce n'est pas un mur et qu'il n'a pas deja ete explore) a l'openList*/
	  if (laby -> lab[ size_x * x + voisin_g.y ] == 0  && trouver_element(closedList, voisin_g) == NULL)
	    {
	       openList = ajout_element(openList, closedList, voisin_g, size_x, size_y, laby-> Tresor);
	    }
	  /* on ajoute le voisin de droite (si ce n'est pas un mur et qu'il n'a pas deja ete explore) a l'openList*/
	  if (laby -> lab[ size_x * x + voisin_d.y ] == 0 && trouver_element(closedList, voisin_d) == NULL)
	    {
	      openList = ajout_element(openList, closedList, voisin_d, size_x, size_y, laby-> Tresor);
	      
	    }	
	  
	}	  
	  
    }

  liberer_memoire(openList);
  liberer_memoire(closedList);
  return 0;
}


int main()
{
	char labName[50];					/* name of the labyrinth */
	char* labData;						/* data of the labyrinth */
	t_return_code ret = MOVE_OK;		/* indicates the status of the previous move */
	t_move move;						/* a move */
	int player;
	int sizeX,sizeY;
	Lab *laby = (Lab*)malloc(sizeof(Lab));
	
	/* connection to the server */
	connectToServer( "pc4025.polytech.upmc.fr", 1234, "prog_mahsh_ani");
	
	
	/* wait for a game, and retrieve informations about it */
	waitForLabyrinth( "DO_NOTHING timeout=400", labName, &sizeX, &sizeY);	
	//waitForLabyrinth( "PLAY_RANDOM timeout=100 rotation=TRUE", labName, &sizeX, &sizeY);
	labData = (char*) malloc( sizeX * sizeY );
	player = getLabyrinth( labData);
	
	
	/* display the labyrinth */
	printf("sizex: %d \t sizey: %d\n", sizeX,sizeY);

	// Initialisation de laby
	laby -> lab = labData;
	laby -> energy = player;
	laby -> play.x = sizeY/2;
	laby -> adv.x = sizeY/2;
	laby -> Tresor.x = sizeY/2;
	laby -> Tresor.y = sizeX/2;
	
	if (player == 1)
	  {
	    laby -> play.y = sizeX - 1;
	    laby -> adv.y = 0;
	  }

	else
	  {
	    laby -> play.y = 0;
	    laby -> adv.y = sizeX - 1;
	  }
	
	do
	  {
	    printLabyrinth();
	    
	    if (player==1)	/* The opponent plays */
	      {
		ret = getMove( &move);
		
		lab_update(laby -> lab, sizeX, sizeY, move);
		pos_update_translation(&(laby->adv), sizeX, sizeY, move);
		pos_update_rotation(&(laby->adv), sizeX, sizeY, move);

		/* On met a jour notre position dans le labyrinthe ainsi que le labyrinthe s'il a effectue une rotation */
		pos_update_rotation(&(laby->Tresor), sizeX, sizeY, move);
		pos_update_rotation(&(laby->play), sizeX, sizeY, move);

		/*C'est a nous de jouer */
		player = 0;
	      }
	    
	    else
	      {
		/*On génère un mouvement puis on met a jour notre structure laby */
		//	move =  make_move(laby, sizeX, sizeY);
		move = a_etoile(laby,sizeX,sizeY);
		ret = sendMove(move);
		lab_update(laby -> lab, sizeX, sizeY, move);

		pos_update_translation(&(laby->play), sizeX, sizeY, move);
		pos_update_rotation(&(laby->play), sizeX, sizeY, move);
		pos_update_rotation(&(laby->Tresor), sizeX, sizeY, move);
		pos_update_rotation(&(laby->adv), sizeX, sizeY, move);
		
		player = 1;
	      }
	    
	  } while(ret == MOVE_OK);	
	
	if ( (player == 0 && ret == MOVE_WIN) || (player == 1 && ret == MOVE_LOSE ) )
	  printf("I lose the game\n");
	else
	  printf("I win the game\n");

	/* we do not forget to free the allocated array */
	free(labData);
	free(laby);
	
	
	/* end the connection, because we are polite */
	closeConnection();
	
	return EXIT_SUCCESS;
}
