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
#include "update.h"
#include "astar.h"


extern int debug;	/* hack to enable debug messages */


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

	int Rotation_check = 0;
	
	/* connection to the server */
	connectToServer( "pc4023.polytech.upmc.fr", 1234, "prog_mahsh_ani");
	
	
	/* wait for a game, and retrieve informations about it */
	//waitForLabyrinth( "DO_NOTHING timeout=400", labName, &sizeX, &sizeY);	
	waitForLabyrinth( "PLAY_RANDOM timeout=100 rotation=TRUE", labName, &sizeX, &sizeY);
	//waitForLabyrinth("ASTAR timeout=100", labName, &sizeX, &sizeY);
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
		    Rotation_check = 1;
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

		/* s'il y a eu rotation, on doit recalculer le chemin*/
		if (Rotation_check)
		  {
		    list_moves =  a_etoile(laby,sizeX,sizeY);
		    nb_tour = 0;
		  }
		/*On génère un mouvement puis on met a jour notre structure laby */
		move = list_moves[nb_tour];
		ret = sendMove(move);
		pos_update_translation(&(laby->play), sizeX, sizeY, move);

		/* On met a jour les donnees du labyrinthe  si on a effectue une rotation */
		if (move.type<=3)
		  {		    
		    Rotation_check = 1;
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
