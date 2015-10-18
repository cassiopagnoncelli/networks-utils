#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> /* bind() */
#include <sys/types.h>  /* bind() */
#include <netinet/in.h>
#include <netdb.h>
#include "calc.h"
#include "msg.h"
#include <time.h>

#define TAM_FILA_CONEXOES 5
#define PORTA_TCP_PADRAO 63000


int meu_id;


/* abre uma conexão TCP na `porta' e devolve o socket. */
int sock_cliente(char *nome_servidor, int porta)
{
  struct hostent *dns = gethostbyname(nome_servidor);
  if (!dns)
    return erro("servidor DNS inalcançável.");

  struct sockaddr_in endereco_servidor;
  bcopy(dns->h_addr, &(endereco_servidor.sin_addr), dns->h_length);
  endereco_servidor.sin_family = AF_INET;
  endereco_servidor.sin_port   = htons(porta);

  /* abre o socket. */
  int cliente_servidor = socket(AF_INET, SOCK_STREAM, 0);
  if (cliente_servidor < 0)
    return erro("socket().");

  timestamp();
  printf("[%d] Tentando conectar com %s em %d\n", meu_id, nome_servidor, porta);
  if (connect(cliente_servidor, (struct sockaddr *) &endereco_servidor, sizeof(endereco_servidor)) < 0)
    return erro("connect().");

  return cliente_servidor;
}



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

/* encerra `sock'. */
int fecha_socket(int sock)
{
  return close(sock);
}

/* abre uma conexão TCP na `porta' e devolve o socket. */
int sock_serv(uint16_t porta)
{
  /* tradução DNS para IP */
  char host[100];
  gethostname(host, 100);
  struct hostent *IP;
  if ((IP=gethostbyname(host)) == NULL)
    return erro("DNS inalcançável.");

  /* abre socket */
  struct sockaddr_in endereco_local;
  endereco_local.sin_port = htons(porta);
  endereco_local.sin_family = AF_INET;
  memcpy(IP->h_addr, &(endereco_local.sin_addr), IP->h_length);
  int socket_conexao = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_conexao < 0)
    return erro("socket().");

  if (bind(socket_conexao, (struct sockaddr *) &endereco_local, sizeof(endereco_local)) < 0)
    return erro("bind().");

  /* modo passivo; i.e., esse processo é um servidor. */
  if (listen(socket_conexao, TAM_FILA_CONEXOES) != 0)
    return erro("listen().");

  return socket_conexao;
}

int tcp_serv(int num_serv, char *prox_serv)
{
  /* abre conexão. */
  int fila_sock = sock_serv(PORTA_TCP_PADRAO + meu_id);
  if (fila_sock < 0)
    return erro("abrir_conexao()");

  timestamp();
  printf("[%d] Sou o servidor %d e estou ouvindo na porta %d\n", meu_id, meu_id, PORTA_TCP_PADRAO + meu_id);

  /* atende os pedidos de conexão na fila. */
  int cliente_sock;
  socklen_t tam_endereco = (socklen_t) sizeof(struct sockaddr_in);
  struct sockaddr_in endereco_cliente;
  unsigned char buffer[BUFSIZ];
  char *res;
  msg *mensagem;
  while (1) {
    timestamp();
    printf("[%d] Estou esperando alguem falar comigo... (Modo de escuta)\n", meu_id);
    cliente_sock = accept(fila_sock, (struct sockaddr *) &endereco_cliente, &tam_endereco);

    /* cliente atendido. */
    if (cliente_sock > 0) {
      bzero(buffer, BUFSIZ);
      read(cliente_sock, buffer, BUFSIZ);
      mensagem = buffer_para_msg(buffer);
        if (meu_id == num_serv) {
          timestamp();
          printf("[%d] Recebi a seguinte expressão: ( %s )\n", meu_id, mensagem->expressao);
          res = calc_expr(mensagem->expressao);
          timestamp();
          printf("[%d] O valor dessa expressão é: ( %s )\n", meu_id, res);
          timestamp();
          printf("[%d] Resta repassar esse valor até o cliente.\n", meu_id);
          write (cliente_sock, msg_para_buffer(nova_expressao(res)), sizeof(msg));
        } else {
          timestamp();
          printf("[%d] Nao sou o ultimo servidor, entao estou repassando a expressão: ( %s )\n", meu_id, mensagem->expressao);
          int con_serv = sock_cliente(prox_serv, PORTA_TCP_PADRAO + meu_id + 1);
          if (con_serv <= 0) 
            return erro("abrir_conexao()");
          write (con_serv, buffer, sizeof(msg));
          bzero(buffer, BUFSIZ);
          read(con_serv, buffer, BUFSIZ);
          msg *m = buffer_para_msg(buffer);

          timestamp();
          printf("[%d] Recebi o retorno: ( %s )\n", meu_id, m->expressao);
          write (cliente_sock, buffer, sizeof(msg));
          fecha_socket (con_serv);
        }
        fecha_socket (cliente_sock);
    } else
      erro("accept(), erro possível com &tam_endereco.");
  }

  /* quem sai por último, apaga a luz. */
  return fecha_socket(fila_sock);
}

/* principal. */
int main(int argc, char **argv) {
  int num_serv;
  char *prox_serv = "localhost";
  if (argc != 3 && argc !=4)
    return erro("  ./servidor <identificador> <num_servidores> [<host>]   # onde <num_servidores> [<host>] é opcional.");

  meu_id = atoi(argv[1]);
  num_serv = atoi(argv[2]);

  if (argc == 4) {
    prox_serv = argv[3];
  }

  timestamp();
  printf("[%d] Num. servidores = [%d] - Proximo servidor = '%s'\n", meu_id, num_serv, prox_serv);



  return tcp_serv(num_serv, prox_serv);
}

