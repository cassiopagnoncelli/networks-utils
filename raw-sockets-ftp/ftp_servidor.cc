/*
   Servidor da aplicacao FTP.
*/
#include "servidor.h"
#include <iostream>

int main (int argc, char **argv) 
{
   // interface de rede
   std::string dev(argc == 2 ? argv[1] : "eth0");

   // aplicacao servidor
   Servidor serv(dev);
   serv.executa();

   return 0;
}
