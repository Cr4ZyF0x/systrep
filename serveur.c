#include <stdio.h>
#include <string.h>
#include "entete.h"
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

struct couple stockage[4];
int i;


int main(int argc, char * argv[])
{
    int rep;
    i = 0;
    
    rep = registerrpc(ARITH_PROG, ARITH_VERS1, MULT_PROC, stock,xdr_couple,xdr_int);
    
    if (rep==-1)
    { 
	printf("Erreur d'enregistrement\n");
	exit(2); 
    }
    
    svc_run();
}


char * stock(struct couple *p)
{
    static int res;
    char mes[100];
    
    res = 0;
    
    if (i<=3)
    {
	
	stockage[i].nom = p->nom;
	stockage[i].port = p->port;
	
	printf("*** Début comm RPC %d ***\n", stockage[i].port);
	
	printf("rpc nom : %s // rpc port : %d\n", stockage[i].nom, stockage[i].port);
	printf("%d\n",i);
	
	if(i==0)
	{
	    res = 2;
	}
	else if(i==1||i==2)
	{
	    res = 1;
	}
	else if(i==3)
	{
	    
	    printf("\n-------- Envoi message UDP ---------\n");
	    printf("Nombre max de clients atteint\n\n");
	    
	    // message du type "id_dest host1 port1 id_1 host2 port2 id_2"
	    
	    printf("Client %d // nom : %s // port : %d \nClient %d // nom : %s // port : %d \n", 1, stockage[1].nom, stockage[1].port, 2, stockage[2].nom, stockage[2].port);
	    sprintf(mes,"%d %s %d %d %s %d %d",0 , stockage[1].nom, stockage[1].port, 1,  stockage[2].nom, stockage[2].port, 2);
	    printf("-- mes : %s --\n", mes);
	    emit_udp(mes, stockage[0].nom, stockage[0].port);
	    
	    printf("Client %d // nom : %s // port : %d\nClient %d // nom : %s // port : %d\n", 0, stockage[0].nom, stockage[0].port, 2, stockage[2].nom, stockage[2].port);
	    sprintf(mes,"%d %s %d %d %s %d %d", 1, stockage[0].nom, stockage[0].port, 0, stockage[2].nom, stockage[2].port, 2);
	    printf("-- mes : %s --\n", mes);
	    emit_udp(mes, stockage[1].nom, stockage[1].port);
	    
	    printf("Client %d // nom : %s // port : %d\nClient %d // nom : %s // port : %d\n", 0, stockage[0].nom, stockage[0].port, 1, stockage[1].nom, stockage[1].port);
	    sprintf(mes,"%d %s %d %d %s %d %d", 2, stockage[0].nom, stockage[0].port, 0, stockage[1].nom, stockage[1].port, 1);
	    printf("-- mes : %s --\n", mes);	    
	    emit_udp(mes, stockage[2].nom, stockage[2].port);
	}
	
	printf("*** Fin comm RPC %d ***\n", stockage[i].port);
	
	i++;
    }
    else
    {
	printf("\n-------- Envoi message UDP ---------\n");
	printf("Nombre max de clients atteint\n");
    }
    
    return ((char *) &res);
}

void emit_udp(char * mes, char * nom, int port) 
{
    int s_com, emis;
    struct sockaddr_in adr, appelant;
    struct hostent *entree;
    int lg_app;
    
    s_com=socket(AF_INET, SOCK_DGRAM,0);
    printf("la socket est cree\n");
    
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
