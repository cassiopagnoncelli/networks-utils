/*
   Cliente da aplicacao FTP.

   Tem como primeiro e unico parametro a interface de 
   rede a usar para a comunicacao, e.g.:

   ./cliente eth0

   quando esse argumento nao for dado, eth0 sera assumido
   como interface de rede.
*/
#include "cliente.h"
#include <iostream>

int main (int argc, char **argv) 
{
   // interface de rede
   std::string interface_rede(argc == 2 ? argv[1] : "eth0");

   // aplicacao cliente
   Cliente c(interface_rede);
   c.executa();

   return 0;
}
