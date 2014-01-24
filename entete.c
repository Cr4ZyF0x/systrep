#include "entete.h"

int xdr_couple(XDR *xdrp, struct couple *p)
{
  return(xdr_string(xdrp,&p->nom,100) && xdr_int(xdrp,&p->port));
}