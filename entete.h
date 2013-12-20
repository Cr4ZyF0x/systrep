#include <rpc/types.h>
#include <rpc/xdr.h>
#define ARITH_PROG 0x33515151
#define ARITH_VERS1 1
#define MULT_PROC 1
#define RAC_PROC 2

struct couple
{
  char * nom;
  int port;
};

int xdr_couple();
char * stock();
void emit_udp(char * mes, char * nom, int port);

/*#include <rpc/types.h>
#include <rpc/xdr.h>
#define ARITH_PROG 0x33333333
#define ARITH_VERS1 1
#define MULT_PROC 1
#define RAC_PROC 2

struct couple
{
  float f1, f2;
};

int xdr_couple();
char *rac();
char *mult();
*/


