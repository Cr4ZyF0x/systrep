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
    int x, m, port_c1, port_c2, id_self, id_client1, id_client2;
    int a=0, i=0, j=0, k=0, l=0;
    int s_com, recus;
    struct couple don, res;
    char mes[100], tmp[50], nom_client1[20], nom_client2[20], port_client1[5], port_client2[5], host[20];
    struct sockaddr_in adr_rpc, appelant;
    struct hostent *entree;
    int lg_app;
    int jeton_present = -1; // -1 = non init, 0 = non présent, 1 = présent
    
    if (argc != 3)
    {
	printf("--------------- Erreur !!!!!!!!!!!!!!!! ------------------\n");
	printf("Usage : ./client <port> <Serveur_hostname>\n");
	exit(1);
    }

    tmp[49] = '\0';
    gethostname(tmp, 49);
    
    don.nom = tmp;
    
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
	printf("[Comm UDP -> Serv] Pb de creation socket\n");
	exit(1); 
    }
    
    // --------------- Call RPC ------------------------------
    sprintf(host,"%s",argv[2]);
    printf("nom du serv : '%s'\n");

    m=callrpc(host, ARITH_PROG, ARITH_VERS1, MULT_PROC, xdr_couple,&don,xdr_int,&x);
    
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
	printf("[Comm rpc] pb callRPC\n");
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
	    printf("[Comm UDP -> Serv] Pb de reception\n");
	}
	else 
	{
	    sendto(s_com,"OK",3,0,(struct sockaddr *)&appelant, lg_app);
	}
	
	
	// --------- Récupération des infos des autres clients envoyées par le serveur --------------
	// message reçu du type "0 etud 5151 1 etud 1664 2"
	
	// mon id parmi les clients
	id_self = atoi(&mes[i]);
	i+=2; // espace + début nom_client1
	
	// =================================
	// -------- Premier client ---------
	while(mes[i]!=' ') 	// nom d'hôte
	{
	    nom_client1[a]=mes[i];
	    i++;
	    a++;
	}
	nom_client1[a]='\0';
	i++;
	while(mes[i]!=' ')	// port de comm
	{
	    port_client1[j]=mes[i];
	    i++;
	    j++;
	}
	port_client1[j]='\0';
	port_c1 = atoi(port_client1);
	i++;
	
	id_client1 = atoi(&mes[i]);	// id du client
	i+=2; // espace + début
	
	// ================================
	// -------- Second client --------- 
	while(mes[i]!=' ')	// nom d'hôte
	{
	    nom_client2[k]=mes[i];
	    i++;
	    k++;
	}
	nom_client2[k]='\0';
	i++;
	while(mes[i]!=' ')	// port de comm
	{
	    port_client2[l]=mes[i];
	    i++;
	    l++;
	}
	port_client2[l]='\0';
	port_c2 = atoi(port_client2);
	i++;
	
	id_client2 = atoi(&mes[i]);	// id du client
	
	printf("\n-- Liste des clients : --\n%s %d %d\n%s %d %d\n", nom_client1, port_c1, id_client1, nom_client2, port_c2, id_client2);
	printf("Mon id est %d\n", id_self);
	
	close(s_com);	// fermeture socket de comm rpc avec le serveur
	
	// -------------------- Début partie SUZUKI-KASAMI -------------------------------
	suzuki_kasami(id_self, don.port, nom_client1, port_c1, id_client1, nom_client2, port_c2, id_client2, jeton_present);
    }
}


void suzuki_kasami(int my_id, int port, char * nom_client1, int port_c1, int id_c1, char * nom_client2, int port_c2, int id_c2, int jeton_present)
{
    int etat;	 	// -1 = Hors SC pas demand, 0 = Hors SC en attente, 1 = SC car demand
    int lg_app, i, j, id_sender, id_min;
    int local[3], horloge[3], toSend[3];	// horloge vectorielle local et transmise
    int demande = 0; 	// pour savoir si on a déjà demandé la SC
    fd_set rfds;	// ensemble des descripteurs de fichier en READ
    int s_fd, retour;	// socket pour UDP, retour du select
    struct timeval attente;
    char entree[100];	// buffer d'entrée clavier
    char msgRecu[100], msgEnvoi[100], type_requete[100];	// message recu sur la socket d'écoute s_fd et à envoyer
    struct sockaddr_in adr, appelant;
    
    
    // ------------- Création socket UDP pour comm entre clients --------------------
    s_fd=socket(AF_INET, SOCK_DGRAM,0);
    
    adr.sin_family=AF_INET;
    adr.sin_port=htons(port);	// mon port de comm
    adr.sin_addr.s_addr=INADDR_ANY;
    
    if (bind(s_fd,(struct sockaddr *) &adr, sizeof (struct sockaddr_in)) !=0)
    {
	printf("[Comm UDP -> Client] Pb de creation socket\n");
	exit(1); 
    }
    
    etat = -1;		// De base OUT de SC
    
    // init du vecteur d'horloges vectorilles
    for(i=0; i<3; i++)
    {
	local[i] = 0;
	horloge[i] = 0;
    }
    
    
    while(1)
    {
	FD_ZERO(&rfds);		// on vide l'ensemble
	FD_SET(0,&rfds);	// ajout de STDIN
	FD_SET(s_fd, &rfds);	// ajout de la socket
	
	attente.tv_sec = 4;	// 4 secondes d'écoute stdin ou socket
	attente.tv_usec = 0;
	
	
	retour = select(s_fd+1, &rfds, NULL, NULL, &attente);
	
	if (retour == -1)
	{		
	    perror("[Suzuki] --Erreur-- du select() :\n\n");
	    exit(1);
	}
	
	if (FD_ISSET(s_fd, &rfds))
	{
	    // ---- Data reçue sur la socket ---------
	    
	    lg_app = sizeof(struct sockaddr_in);
	    retour = recvfrom(s_fd, msgRecu, 100, 0,(struct sockaddr *) &appelant, &lg_app);
	    if (retour <= 0)
	    {
		perror("[Suzuki] --Erreur-- creation de socket\n\n");
		exit(1);
	    }
	    else
	    {
		//MAJ local[i]
		local[my_id] = MAX(local[my_id], horloge[my_id]);
		
		// requete du type "id_expéditeur type_requete" ex: "1 needJeton"		
		printf("[Suzuki] Requete recue : %s\n\n",msgRecu);
				
		i=0;
		j=0;
		
		id_sender = atoi(&msgRecu[i]);
		i += 2; // espace + début type_requete
		
		while(msgRecu[i] != '\0')
		{
		    type_requete[j] = msgRecu[i];
		    i++;
		    j++;
		}
		type_requete[j] = '\0';
		
		// -->> comportement en réception de requete SC  << --
		// quelqu'un veut le jeton && on l'a && on est pas en SC
		if (strcmp(type_requete, "needJeton") == 0 && jeton_present == 1 && etat == -1)
		{
		    sprintf(msgEnvoi,"%d incomingJeton", my_id);
		    printf("msgEnvoi: '%s'\n", msgEnvoi);
		    
		    if (id_sender == id_c1)
		    {
			send_udp(msgEnvoi, nom_client1, port_c1, horloge);
		    }
		    else if (id_sender == id_c2)
		    {
			send_udp(msgEnvoi, nom_client2, port_c2, horloge);
		    }
		    jeton_present = 0;
		}
		
		// si on reçoit le jeton et qu'on l'attendait
		if (strcmp(type_requete, "incomingJeton") == 0 && etat == 0)
		{
		    jeton_present = 1;
		    etat = 1;		// IN
		    
		    printf("===========================\n");
		    printf("J'ai le jeton !!!!!!!!!!!!!\nC'est parti pour 10 sec\n");
		    printf("===========================\n");
		    
		    sleep(10);		// repos de 10 sec
		    
		    printf("------ Je sors ------\n\n");
		    
		    etat = -1;		// OUT
		    
		    // MAJ horloge
		    horloge[my_id] ++;
		    
		    // détermination des sites ayant été en SC
		    for (i=0; i<3; i++)
		    {
			if (local[i] > horloge[i])
			{
			    toSend[i] = 1;
			}
			else
			{
			    toSend[i] = 0;
			}
		    }
		    
		    // détermination du site à qui envoyer le jeton
		    id_min = -1;
		    for(i=0; i<3; i++)
		    {
			if (toSend[i] == 1)
			{
			    if (id_min == -1)
			    {
				id_min = i;
			    }
			    if (horloge[i] < horloge[id_min])
			    {
				id_min = i;
			    }
			}
		    }
		    
		    // envoi du jeton si différence (cad id_min != -1)
		    if (id_min == id_c1)
		    {
			send_udp(msgEnvoi, nom_client1, port_c1, horloge);
		    }
		    else if (id_min == id_c2)
		    {
			send_udp(msgEnvoi, nom_client2, port_c2, horloge);
		    }
		}
	    }
	}
	
	if (FD_ISSET(0, &rfds))
	{		
	    // réception d'entrée clavier
	    j = 0;
	    while ((i=getchar()) != '\n')
		entree[j++]=i;
	    entree[j]='\0';
	    
	    if (!strcmp(entree, "E") || !strcmp(entree, "e"))
	    {
		if (etat != -1)
		{
		    printf("===================================\n");
		    printf("------ En attente ou En SC ! ------\n------ Veuillez patienter ------\n");
		    printf("===================================\n");
		}
		else
		{
		    // MAJ horloge locale
		    ++local[my_id];
		    		    
		    if (jeton_present == 1)
		    {
			etat = 1;		// IN
			printf("===================================\n");
			printf("Entree en SC car demande\nC'est parti pour 10 sec\n");
			printf("===================================\n");
			
			sleep(10);		// repos de 10 sec
			printf("------ Je sors ------\n\n");
			
			etat = -1;		// OUT
			
			// MAJ horloge
			horloge[my_id] ++;
			
			// détermination des sites ayant été en SC
			for (i=0; i<3; i++)
			{
			    if (local[i] > horloge[i])
			    {
				toSend[i] = 1;
			    }
			    else
			    {
				toSend[i] = 0;
			    }
			}
			
			// détermination du site à qui envoyer le jeton
			id_min = -1;
			for(i=0; i<3; i++)
			{
			    if (toSend[i] == 1)
			    {
				if (id_min == -1)
				{
				    id_min = i;
				}
				if (horloge[i] < horloge[id_min])
				{
				    id_min = i;
				}
			    }
			}
			
			// envoi du jeton si différence (cad id_min != -1)
			if (id_min == id_c1)
			{
			    send_udp(msgEnvoi, nom_client1, port_c1, horloge);
			}
			else if (id_min == id_c2)
			{
			    send_udp(msgEnvoi, nom_client2, port_c2, horloge);
			}
		    }
		    else
		    {
			// envoi du jeton à tout le monde
			sprintf(msgEnvoi,"%d needJeton", my_id);
			printf("msgEnvoi: '%s'\n", msgEnvoi); 
			
			// envoi aux 2 autres clients
			send_udp(msgEnvoi, nom_client1, port_c1, horloge);
			send_udp(msgEnvoi, nom_client2, port_c2, horloge);
			
			etat = 0;	// Attente
		    }
		}
	    }
	    else if (!strcmp(entree, "Q") || !strcmp(entree, "q") || !strcmp(entree, "quit") || !strcmp(entree, "exit"))
	    {
		close(s_fd);
		exit(0);
	    }
	    else
		printf("Commande inconnue !\nAppuyez sur :\n\t E ou e pour entrer en SC\n\t Q, q, quit ou exit pour quitter\n");
	}
    }
}


void send_udp(char * mes, char * nom, int port, int * horloge)
{
    int s_com, emis;
    struct sockaddr_in adr, appelant;
    struct hostent *entree;
    int lg_app;
    
    s_com=socket(AF_INET, SOCK_DGRAM,0);
    printf("socket envoi creee vers %d\n", port);
    
    adr.sin_family=AF_INET;
    adr.sin_port=htons(port);
    adr.sin_addr.s_addr=INADDR_ANY;
    
    lg_app=sizeof(struct sockaddr_in);
    
    entree= (struct hostent *)gethostbyname(nom);
    bcopy((char *) entree->h_addr, (char *)&adr.sin_addr, entree->h_length);
    
    emis=sendto(s_com,mes,100,0,(struct sockaddr *)&adr, lg_app);
    
    if (emis <=0)
	printf("[Suzuki UDP -> client] Pb envoi msgEnvoi\n");
    
    close(s_com);
}
