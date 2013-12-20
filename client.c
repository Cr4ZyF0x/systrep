#include "entete.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
  int x, m, port_c1, port_c2;
  int i=0, j=0, k=0, l=0;
  int s_com,recus;
  struct couple don, res;
  char mes[100], tmp[50], nom_client1[20], nom_client2[20], port_client1[5], port_client2[5];
  struct sockaddr_in adr_rpc, appelant;
  struct hostent *entree;
  int lg_app;
  
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
	printf("\nListe des clients : \n%s\n",mes);
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
  
	while(mes[i]!=' ')
	{
		nom_client2[k]=mes[i];
		i++;
		k++;
	}
	nom_client2[k]='\0';
	i++;
	while(mes[i]!='\0')
	{
		port_client2[l]=mes[i];
		i++;
		l++;
	}
	port_client2[l]='\0';
	port_c2 = atoi(port_client2);
	
	

	printf("\nListe des clients :\n%s %d %s %d\n", nom_client1, port_c1, nom_client2, port_c2);
    close(s_com);
  }
}
