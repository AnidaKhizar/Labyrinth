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
  int x = min( size_y - arr.x + dep.x, abs(arr.x - dep.x) );               /*on regarde s'il est plus rapide d'aller vers la droite ou vers la gauche */
  int y = min( size_x - arr.y + dep.y, abs(arr.y - dep.y) );               /*on regarde s'il est plus rapide de passer par en haut ou par en bas */
  return x+y;
}

/*Fonction renvoyant un pointeur vers la case de la closedList se trouvant à la position p */
Case* trouver_element(Case* closedList, Pos p)
{
  Case *tmp = closedList; //case parcourant la closedList  
  while(tmp)
    {
      if( tmp -> pos.x == p.x && tmp->pos.y == p.y)
	{
	  return tmp;	  
	}
      tmp= tmp->suiv;
    }
  return NULL;
}

/*Fonction ajoutant un voisin de la case x a la liste chainee openList
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
  if (openList->heuristique > element_ajout->heuristique)  
    {
      element_ajout -> suiv = openList;
      return element_ajout;
    }

  /*On parcourt les éléments de la liste tant qu'on n'a pas trouvé un entier plus grand que l'heuristique du voisin a ajouter*/
  while ( (cur!=NULL) && (cur->heuristique <= element_ajout->heuristique) )
    {
      /* Si l'element que l'on veut ajouter se trouve deja dans l'openList mais avec un cout plus eleve, alors on modifie la case deja existante*/
      if (cur -> pos.x == element_ajout->pos.x && cur -> pos.y == element_ajout->pos.y)
	{
	  if  (cur->cost >= element_ajout->cost) 
	    {
	      cur -> cost = element_ajout ->cost;
	      cur -> prec = element_ajout->prec;
	    }
	  free(element_ajout);
	  return openList;
	  
	}
      
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

/* Fonction reconstituant le chemin trouve a partir de la case Tresor afin de renvoyer la liste de mouvement a effectuer*/
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


/* Fonction renvoyant un tableau de mouvements consécutifs a effectuer pour aller du depart a l'arrivee*/

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


int main()
{
	char labName[50];					/* name of the labyrinth */
	char* labData;						/* data of the labyrinth */
	t_return_code ret = MOVE_OK;		/* indicates the status of the previous move */
	t_move move;						/* a move */
	int player;
	int sizeX,sizeY;
	Lab *laby = (Lab*)malloc(sizeof(Lab));

	/*Tableau contenant la liste des mouvements a effectuer*/
	t_move* list_moves;

	int nb_tour = 0; //indique le numero de notre tour
	
	/* connection to the server */
	connectToServer( "pc4023.polytech.upmc.fr", 1234, "prog_mahsh_ani");
	
	
	/* wait for a game, and retrieve informations about it */
	//waitForLabyrinth( "DO_NOTHING timeout=400", labName, &sizeX, &sizeY);	
	//waitForLabyrinth( "PLAY_RANDOM timeout=100 rotation=TRUE", labName, &sizeX, &sizeY);
	waitForLabyrinth("ASTAR timeout=100", labName, &sizeX, &sizeY);
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

	list_moves = a_etoile(laby,sizeX,sizeY);
	
	do
	  {
	    printLabyrinth();
	    
	    if (player==1)	/* The opponent plays */
	      {
		ret = getMove( &move);
		/* On met a jour sa position dans le labyrinthe s'il s'est juste deplace*/
		pos_update_translation(&(laby->adv), sizeX, sizeY, move);
		
		/* On met a jour les donnees du labyrinthe  s'il a effectue une rotation */
		if (move.type<=3)
		  {
		    lab_update(laby -> lab, sizeX, sizeY, move);		
		    pos_update_rotation(&(laby->adv), sizeX, sizeY, move);		
		    pos_update_rotation(&(laby->Tresor), sizeX, sizeY, move);
		    pos_update_rotation(&(laby->play), sizeX, sizeY, move);
		    
		  }
		
		/*C'est a nous de jouer */
		player = 0;
	      }
	    
	    else
	      {
		//	move =  make_move(laby, sizeX, sizeY);
		//	move = a_etoile(laby,sizeX,sizeY);

		/*On génère un mouvement puis on met a jour notre structure laby */
		move = list_moves[nb_tour];
		ret = sendMove(move);
		pos_update_translation(&(laby->play), sizeX, sizeY, move);

		/* On met a jour les donnees du labyrinthe  si on a effectue une rotation */
		if (move.type<=3)
		  {
		    lab_update(laby -> lab, sizeX, sizeY, move);		
		    pos_update_rotation(&(laby->adv), sizeX, sizeY, move);		
		    pos_update_rotation(&(laby->Tresor), sizeX, sizeY, move);
		    pos_update_rotation(&(laby->play), sizeX, sizeY, move);
		    
		  }
		player = 1;
		nb_tour++;
	      }
	    
	  } while(ret == MOVE_OK);	
	
	if ( (player == 0 && ret == MOVE_WIN) || (player == 1 && ret == MOVE_LOSE ) )
	  printf("I lose the game\n");
	else
	  printf("I win the game\n");

	/* we do not forget to free the allocated array */
	free(labData);
	free(laby);
	free(list_moves);
	
	
	/* end the connection, because we are polite */
	closeConnection();
	
	return EXIT_SUCCESS;
}
