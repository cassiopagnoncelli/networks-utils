#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> /* bind() */
#include <sys/types.h>  /* bind() */
#include <netinet/in.h>
#include <netdb.h>
#include "msg.h"
#define fecha_socket(s) close(s)
#include <time.h>


int timestamp()
{
    time_t ltime;
    struct tm *Tm;

    ltime=time(NULL);
    Tm=localtime(&ltime);
 
    printf("[%d-%d-%d %d:%d:%d] ",
            Tm->tm_year+1900,
            Tm->tm_mon,
            Tm->tm_mday,
            Tm->tm_hour,
            Tm->tm_min,
            Tm->tm_sec);
}


/* imprime na saída padrão a mensagem de erro e devolve falha. */
int erro(const char *str)
{
  fprintf(stderr, "** erro: %s\n", str);
  return -1;
}

/* abre uma conexão TCP na `porta' e devolve o socket. */
int sock_cliente(char *nome_servidor, char *porta)
{
  struct hostent *dns = gethostbyname(nome_servidor);
  if (!dns)
    return erro("servidor DNS inalcançável.");

  struct sockaddr_in endereco_servidor;
  bcopy(dns->h_addr, &(endereco_servidor.sin_addr), dns->h_length);
  endereco_servidor.sin_family = AF_INET;
  endereco_servidor.sin_port   = htons(atoi(porta));

  /* abre o socket. */
  int cliente_servidor = socket(AF_INET, SOCK_STREAM, 0);
  if (cliente_servidor < 0)
    return erro("socket().");

  timestamp();
  printf("[C] Tentando conectar com %s em %s\n", nome_servidor, porta);
  if (connect(cliente_servidor, (struct sockaddr *) &endereco_servidor, sizeof(endereco_servidor)) < 0)
    return erro("connect().");

  return cliente_servidor;
}

int main(int argc, char **argv)
{
  if (argc != 4)
    return erro("./servidor <nome_servidor> <porta> <expr>");

  char 
    *nome_host = argv[1],
    *porta     = argv[2],
    *expr      = argv[3];

  timestamp();
  printf("[C] Sou o cliente e minhas mensagens terao prefixo [C].\n");

  /* abre conexão. */
  int con_serv = sock_cliente(nome_host, porta);
  if (con_serv <= 0) 
    return erro("abrir_conexao()");

  if (write(con_serv, msg_para_buffer(nova_expressao(expr)), sizeof(msg)) < 0)
    return erro("Nao consegui enviar...");

  timestamp();
  printf("[C] Enviei a expressao\n");

  char buffer[BUFSIZ];
  bzero(buffer, BUFSIZ);
  read(con_serv, buffer, BUFSIZ);
  msg *m = buffer_para_msg(buffer);

  timestamp();;
  printf("[C] Recebi: ( %s )\n", m->expressao);
 
  /* quem sai por último, apaga a luz. */
  return fecha_socket(con_serv);
}
