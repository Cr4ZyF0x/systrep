#include "entete.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
  int x, m, port_c1, port_c2, num_c1, num_c2;
  int i=0, j=0, k=0, l=0;
  int s_com,recus;
  struct couple don, res;
  char mes[100], tmp[50], nom_client1[20], nom_client2[20], port_client1[5], port_client2[5];
  struct sockaddr_in adr_rpc, appelant;
  struct hostent *entree;
  int lg_app;
  int jeton_present = -1; // -1 = non init, 0 = non présent, 1 = présent
  
  if (argc != 2)
  {
    printf("--------------- Erreur !!!!!!!!!!!!!!!! ------------------\n");
    printf("Usage : ./client <port>\n");
    exit(1);
  }
  
  /*char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);*/
  tmp[49] = '\0';
  gethostname(tmp, 49);
  
  don.nom = tmp;
  
  //don.nom = argv[2];
  don.port = atoi(argv[1]);
  
  printf("local nom : %s // local port : %d\n", don.nom, don.port);
  
  // ------------- Création socket UDP --------------------
  s_com=socket(AF_INET, SOCK_DGRAM,0);
  printf("la socket est cree\n");
  
  adr_rpc.sin_family=AF_INET;
  adr_rpc.sin_port=htons(don.port);
  adr_rpc.sin_addr.s_addr=INADDR_ANY;
  
  if (bind(s_com,(struct sockaddr *) &adr_rpc, sizeof (struct sockaddr_in)) !=0)
  {
    printf("Pb de connexion\n");
    exit(1); 
  }
  
  // --------------- Call RPC ------------------------------
  
  m=callrpc("etud", ARITH_PROG, ARITH_VERS1, MULT_PROC,
	    xdr_couple,&don,xdr_int,&x);
	    
	    if (m==0)
	    {
	      printf("Nom et port envoye par RPC !\n");
	      printf("Réponse RPC du serveur : %d\n", x);
	      
	      // affectation du jeton en fonction rep serveur
	      if (x==2)
	      {
		jeton_present = 1;
	      }
	      else if (x == 1)
	      {
		jeton_present = 0;
	      }
	    }
	    else
	    {
	      printf("pb RPC\n");
	    }
	    
	    // --------------- Début part UDP ------------------------------------
	    
	    if (x == 0)
	    {
	      printf("Max de clients atteint !\n");
	      close(s_com);
	      exit(0);
	    }
	    else
	    {
	      printf("je suis en ecoute\n");
	      lg_app=sizeof(struct sockaddr_in);
	      recus=recvfrom(s_com,mes,100,0,(struct sockaddr *) &appelant,&lg_app);
	      
	      if (recus <=0)
	      {
		printf("bug\n");
	      }
	      else 
	      {
		//printf("\nListe des clients : \n%s\n",mes);
		sendto(s_com,"OK",3,0,(struct sockaddr *)&appelant, lg_app);
	      }
	      
	      while(mes[i]!=' ')
	      {
		nom_client1[i]=mes[i];
		i++;
	      }
	      nom_client1[i]='\0';
	      i++;
	      while(mes[i]!=' ')
	      {
		port_client1[j]=mes[i];
		i++;
		j++;
	      }
	      port_client1[j]='\0';
	      port_c1 = atoi(port_client1);
	      i++;
	      
	      num_c1 = atoi(&mes[i]);
	      i+=2; // ' ' et début
	      
	      
	      while(mes[i]!=' ')
	      {
		nom_client2[k]=mes[i];
		i++;
		k++;
	      }
	      nom_client2[k]='\0';
	      i++;
	      while(mes[i]!=' ')
	      {
		port_client2[l]=mes[i];
		i++;
		l++;
	      }
	      port_client2[l]='\0';
	      port_c2 = atoi(port_client2);
	      i++;
	      
	      num_c2 = atoi(&mes[i]);
	      
	      printf("\n-- Liste des clients : --\n%s %d %d\n%s %d %d\n", nom_client1, port_c1, num_c1, nom_client2, port_c2, num_c2);
	      
	      close(s_com);
	      
	      // -------------------- Début part SUZUKI-KAZAMI -------------------------------
	      suzuki_kasami(nom_client1, port_c1, num_c1, nom_client2, port_c2, num_c2, jeton_present);
	    }
}


void suzuki_kasami(char * nom_client1, int port_c1, int id_c1, char * nom_client2, int port_c2, int id_c2, int jeton_present)
{
  int etat, lg_app; // -1 = Hors SC pas demand, 0 = Hors SC en attente, 1 = SC car demand
  int compteur10sec = 1000;
  int v[3]; // horloge vectorielle
  int demande = 0; // pour savoir si on a déjà demandé la SC
  fd_set rfds;
  int s_fd, retour;	// socket pour UDP, retour du select
  struct timeval attente;
  char recu[100];	// message recu sur la socket d'écoute s_fd
  struct sockaddr_in appelant;
  
  // ------------- Création socket UDP --------------------
  s_fd=socket(AF_INET, SOCK_DGRAM,0);
  
  adr_rpc.sin_family=AF_INET;
  adr_rpc.sin_port=htons(don.port);
  adr_rpc.sin_addr.s_addr=INADDR_ANY;
  
  if (bind(s_fd,(struct sockaddr *) &adr_rpc, sizeof (struct sockaddr_in)) !=0)
  {
    printf("Pb de connexion\n");
    exit(1); 
  }
  
  // init du vecteur d'horloges vectorilles
  for(int i=0; i<3; i++)
  {
    v[i] = 0;
  }
  
  while(1)
  {
    // LNi : lastNumber = numéro de la dernière demande d'entrée en SC du site i satisfaite
    FD_ZERO(&rfds);		// on vide l'ensemble
    FD_SET(0,&rfds);		// ajout de STDIN
    FD_SET(s_fd, &rfds);	// ajout de la socket
    
    attente.tv_sec = 5;		// 5 secondes d'écoute stdin ou socket
    attente.tv_usec = 0;
    
    
    retour = select(s_fd+1, &rfds, NULL, NULL, &attente);
    
    if (retour == -1)
    {		
      perror("-Erreur- select() :");
      exit(1);
    }
    
    if (FD_ISSET(s_fd, &rfds))
    {
      /* reçu sur la socket */
      lg_app=sizeof(struct sockaddr_in);
      recus=recvfrom(s_fd, recu, 100, 0,(struct sockaddr *) &appelant, &lg_app);
      //donnee[retour] = 0;
      
      //printf("[DEBUG] Request received : %s\n",donnee);
      
      //thread = (pthread_t *) malloc(sizeof(pthread_t *));
      //pthread_create(thread, NULL, process_udp, (char*) donnee); 
      
      /* recu > sortir l'id du l'envoyeur et la requete
	 le strcmp ne marche pas*/
      
      // -->> comportement en réception de requete SC  << --
      if (strcmp(recu, "needJeton") == 0 && jeton_present == 1 && etat == -1)
      {
	//printf("[DEBUG] sending to %d the token\n", receiverInd);
	//emit_udp(recu, 
	send_udp(serializeData(sendToken, message), message, tab, ind, receiverInd, receiverInd + 1);
	jeton_present = 0;
      }
      
      if (strcmp(recu, "incomingJeton") == 0 && etat == 0)
      {
	printf("j'ai le jeton !!!!!!!!!!!!!\n");
	jeton_present = 1;
	enterCriticalSection();
      }
    } 
    
    if (FD_ISSET(0, &lectures))
    {		
      /* reçu sur STDIN */
      j = 0;
      while ((i=getchar()) != '\n')
	frappe[j++]=i;
      frappe[j]='\0';
      
      
      if (!strcmp(frappe, "E") || !strcmp(frappe, "e"))
      {
	if (etat != out)
	{
	  puts(" ____________________________________ ");
	  puts("|                                    |");
	  puts("| Already waiting or in, KEEP CALM!. |");
	  puts("|____________________________________|");
	}
	else
	{
	  // update clock
	  ++horloge[ind];
	  
	  thread = (pthread_t *) malloc(sizeof(pthread_t *));
	  
	  // send the query to everyone
	  pthread_create(thread, NULL, process_stdin, (int*) horloge);
	}
      }
      else
	puts("unknown command. Press e to enter in the critical section.");
    }
    if (getchar() == 'e')
    {
      demande = 1;
      printf("Demande d'entrée en SC\n");
      // ---------- Demande d'entrée en SC -------------
      v[i] = v[i] + 1;
      if (jeton_present == 1)
      {
	printf("J'entre en SC !\n");
	etat = 1; // Dedans
	
	while(compteur10sec>0)
	{
	  if(select(2, socket, stdin, NULL, NULL, timeout))
	  {
	    misEnAttenteDeLaDemande();
	  }
	  compteur10sec --;
	}
	// Sortie de la SC
	etat = -1;
	// maj LNi : 
	jeton.lni = v;
	demande = 0;
	if(jeton.lni != v)
	{
	  envoiJeton(Sj);
	  jeton_present = 0;
	}
      }
      else // jeton absent
      {
	diffuser(req, v, Si); -> emit_udp
	état = 0; // attente
      }
      
    }
    
    // -------------- Réception d'un message (rqt) ------------------
    
    maj(v); // horloge vect -> MAX de chaque val
    
    if( (etat == -1) && (jeton == 1)) // etat = dehors && jeton présent
    {
      envoiJeton(Sj); -> emit_udp
      jeton = 0;
    }
    else if ( etat == 1) // etat = dedans
    {
      misEnAttenteDeLaDemande();
    }
    
    // ------------- Réception d'un jeton -----------
    recvfrom(jeton);
    if ( etat == 0) // etat == en attente
    {
      etat = 1; // etat = dedans
    }
  }
}


void emit_udp(char * mes, char * nom, int port) 
{
  int s_com, emis;
  struct sockaddr_in adr, appelant;
  struct hostent *entree;
  int lg_app;
  
  s_com=socket(AF_INET, SOCK_DGRAM,0);
  printf("la socket suzuki est cree\n");
  
  adr.sin_family=AF_INET;
  adr.sin_port=htons(port);
  adr.sin_addr.s_addr=INADDR_ANY;
  
  lg_app=sizeof(struct sockaddr_in);
  
  entree= (struct hostent *)gethostbyname(nom);
  bcopy((char *) entree->h_addr, (char *)&adr.sin_addr, entree->h_length);
  
  emis=sendto(s_com,mes,100,0,(struct sockaddr *)&adr, lg_app);
  
  if (emis <=0)
    printf("gros probleme\n");
  
  recvfrom(s_com,mes,sizeof(mes),0, (struct sockaddr *)&appelant,&lg_app);
  printf("message recu : %s\n\n",mes);
  close(s_com);
}
