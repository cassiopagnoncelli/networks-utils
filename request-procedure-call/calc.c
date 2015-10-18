#include <stdio.h>
#include <stdlib.h>
#include "calc.h"

char *
calc_expr (char *expr)
{
  FILE *fp = fopen ("conta.dat", "w+");
  fputs (expr, fp);
  fputs ("\nquit", fp);
  fclose (fp);

  /* calcula com bc */
  system ("bc -l < conta.dat > resultado.dat");
  system ("cat resultado.dat | tail -1 | tee resultado.dat 1>/dev/null");

  /* resultado da expressao em resultado.dat */
  fp = fopen ("resultado.dat", "r");
  char *res = malloc (50 * sizeof (char));
  fgets (res, 50, fp);

  // remove o \n
  int i;
  for (i = 0; i < 50; i++)
    if (res[i] == '\n')
      res[i] = '\0';

  fclose (fp);
  system ("rm resultado.dat conta.dat");

  return res;
}
