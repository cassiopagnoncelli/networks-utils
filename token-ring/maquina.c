#include "tokenring.h"
#include <stdio.h>
#include <stdlib.h>

// arg1 = host, arg2 = porta
int main (int argc, char **argv) {
   if (argc != 3) {
      char h[100];
      printf("host [padrao=localhost]: ");
      scanf("%s", h);

      int num_maquina;
      printf("numero da maquina [0,1,2,3]: ");
      scanf("%d", &num_maquina);

      rodar(h, num_maquina);
   } else
      rodar(argv[1], atoi(argv[2]));

   return 0;
}
