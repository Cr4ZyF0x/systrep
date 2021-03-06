#include <rpc/types.h>
#include <rpc/xdr.h>

#define ARITH_PROG 0x33515151
#define ARITH_VERS1 1
#define MULT_PROC 1
#define RAC_PROC 2

#define MAX(a, b) (((a)>(b))?(a):(b))

struct couple
{
  char * nom;
  int port;
};

int xdr_couple();
char * stock();
void emit_udp(char * mes, char * nom, int port);
void send_udp(char * mes, char * nom, int port, int * horloge);
void suzuki_kasami(int my_id, int port, char * nom_client1, int port_c1, int id_c1, char * nom_client2, int port_c2, int id_c2, int jeton_present);
