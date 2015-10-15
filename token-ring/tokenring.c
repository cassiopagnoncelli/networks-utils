#include "tokenring.h"
#include "mensagem.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define PORTA_GUIA_TURISTICO 2513 // 1102 + 1411
#ifndef MAQUINAS_NA_REDE
#define MAQUINAS_NA_REDE 4
#endif
#define TIMEOUT_BASTAO (2 * MAQUINAS_NA_REDE + 1)

/* VARS usadas em (quase) todas as funcoes */
fila_mensagens *fila;

int sock;

// informacoes dessa maquina
struct sockaddr_in maquina;
unsigned char origem_num;
unsigned int origem_porta;
// maquina imediatamente a direita
struct sockaddr_in maquina_direita;
unsigned char destino_num;
unsigned int destino_porta;

// parametros da rede
char host[100];

// info bastao
time_t tempo_bastao;
//pthread_t thread_recria_bastao;
#ifdef conta_bastao
unsigned long int bastoes_passados = 0;
#endif

/* FUNCOES */
// constroi a rede token ring e entra na rede
void
rodar (char *nome_host, int num_maquina)
{
  // inicializa as variaveis
  origem_num = (unsigned char) (num_maquina);
  destino_num = (unsigned char) ((num_maquina + 1) % MAQUINAS_NA_REDE);
  origem_porta = PORTA_GUIA_TURISTICO + 1 + (num_maquina % MAQUINAS_NA_REDE);
  destino_porta =
    PORTA_GUIA_TURISTICO + 1 + ((num_maquina + 1) % MAQUINAS_NA_REDE);
  strcpy (host, nome_host ? nome_host : "localhost");
  if (!(fila = inicializa_fila (malloc (sizeof (fila_mensagens)))))
    erro ("[rodar()]: fila nao inicializada.");
  tempo_bastao = time(NULL);

  // abre socket e o configura
  sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    erro
      ("** socket() em inicializa_rede **.\nerro: impossivel abrir socket.\n");
  memset (&maquina, 0, sizeof (maquina));
  maquina.sin_family = AF_INET;
  maquina.sin_addr.s_addr = INADDR_ANY;
  maquina.sin_port = htons (origem_porta);
  if (bind (sock, (struct sockaddr *) &maquina, sizeof (maquina)) < 0)
    erro ("** bind() em inicializa_rede **.\n");

  // mostra informacoes de rede
  printf ("maq=%u porta=%u maq_destino=%u porta_destino=%u rede_destino=%s\n",
	  origem_num, origem_porta, destino_num, destino_porta, host);

  // monta_rede()...
  // maquina da direita
  maquina_direita.sin_family = AF_INET;
  maquina_direita.sin_port = htons (destino_porta);
  struct hostent *hp = gethostbyname (host);
  if (!hp)
    erro ("host invalido.");
  if (!memcpy
      ((char *) &maquina_direita.sin_addr, (char *) hp->h_addr, hp->h_length))
    erro ("memcpy().");
  // ...fim monta_rede()

  // receber mensagens e receber texto do teclado e recriar bastao
  pthread_t thread_escuta_rede, thread_recebe_teclado, thread_recria_bastao;
  if (pthread_create (&thread_escuta_rede, NULL, &escuta_rede, NULL) != 0)
    fprintf (stderr, "erro: ** retorno da thread escuta_rede **.\n");
  if (pthread_create (&thread_recebe_teclado, NULL, &recebe_teclado, NULL) != 0)
    fprintf (stderr, "erro: ** retorno da thread recebe_teclado **.\n");
  if (pthread_create (&thread_recria_bastao, NULL, &timeout_bastao, NULL) != 0)
    fprintf (stderr, "erro: thread para recriar o bastao nao foi colocada.\n");
#ifdef conta_bastao

  pthread_t thread_conta_bastao;
  if (pthread_create(&thread_conta_bastao, NULL, &contagem_bastao, NULL) != 0)
    fprintf (stderr, "aviso: thread para contagem de bastao nao foi colocada.\n");

#endif
  pthread_join (thread_recebe_teclado, NULL);
}

#ifdef conta_bastao
// estatisticas da contagem do bastao
void *
contagem_bastao (void *arg)
{
   while (1) {
      sleep(10);
      fprintf(stdout, 
         "aviso: nos ultimos 10s o bastao passou por essa maquina %lu vezes.\n", 
	 bastoes_passados);
      bastoes_passados = 0;
   }

   // este trecho nunca sera executado, while(1){...} nao para.
   return arg == NULL ? arg : NULL; // uma forma boba de usar o argumento
}
#endif

// recebe do teclado e enfila a nova mensagem
void *
recebe_teclado (void *arg)
{
  mensagem *m;
  while (1)
    if ((m = nova_mensagem (malloc (sizeof (mensagem)), origem_num, TEXTO, req_linha ())) != NULL
	&& enfila_mensagem (fila, m) == NULL)
      fprintf (stderr, "erro: ** enfila_mensagem() em recebe_teclado **.\n");

  return NULL;
}

// escuta por nova mensagem na rede
void *
escuta_rede (void *arg)
{
  unsigned char *btmp, bufmsg[1000];
  mensagem *m, *direita_imprimir, *msg_bastao;
  ssize_t n;
  struct sockaddr_in maquina_esquerda;
  unsigned int esq_tam = sizeof (struct sockaddr_in);
  char str_nula = '\0';

  while (1) {
    n = recvfrom (sock, bufmsg, 260, 0, (struct sockaddr *) &maquina_esquerda, &esq_tam);
    if (n < 0) {
      fprintf (stderr, "erro: ** recvfrom() em escuta_rede **.\n");
      fprintf (stderr, "-> possivel mensagem descartada OU ruido.\n");
    }
    else 
    if (esq_tam >= 2) 
    {
      // recebeu um bastao, todas as mensagens sao enviadas aqui
      if (eh_bastao (bufmsg)) 
      {
        tempo_bastao = time(NULL);

	// envia as mensagens da fila
	if ((m = desenfila_mensagem (fila)) != NULL) {
	  sleep (2);
	  btmp = mensagem_para_buffer (m);
	  if (enviar_direita (btmp, m->tam_msg) < 0)
	    fprintf (stderr, "erro: ** sendto() em escuta_rede **.\n%s\n",
		     "-> aparentemente a mensagem nao foi enviada.");
	  free (btmp);
	}
	// repassa o bastao
        if (enviar_direita (bufmsg, esq_tam) < 0) {
	    fprintf (stderr, "erro: ** sendto() em escuta_rede **.\n");
	    fprintf (stderr, "-> aparentemente o bastao nao foi repassado.\n");
        }

#ifdef conta_bastao
	bastoes_passados++;
#endif
      }
      // eh uma mensagem de texto
      else 
      if ((m=buffer_para_mensagem(bufmsg)) != NULL) 
      {
	if (m->origem != origem_num) {
           imprime_mensagem(m);
	   enfila_mensagem(fila, m);
	}
      } // eh_bastao?
    } // tam_msg >= 2?
  }

  return NULL;
}

void *
timeout_bastao (void *arg)
{
  char str_nula = '\0';
  mensagem *msg_bastao = nova_mensagem (malloc (sizeof (mensagem)), 
                            origem_num, BASTAO, &str_nula); 

  while (1) {
    //sleep(1);
    if (time(NULL) - tempo_bastao > TIMEOUT_BASTAO) {
       if (enviar_direita (mensagem_para_buffer (msg_bastao), msg_bastao->tam_msg) < 0)
         fprintf (stderr, "aviso: timeout do bastao, nao consegui recoloca-lo no anel.\n");
#ifdef info_bastao
       else
         fprintf (stdout, "aviso: timeout do bastao, um novo bastao foi colocado no anel.\n");
#endif

       tempo_bastao = time(NULL);
    }
  }

  return NULL;
}

// pede para usuario digitar alguma coisa
unsigned char *
req_linha ()
{
  char *l = malloc (sizeof (char) * TAM_MAX_MSG);
#ifdef quebra_msg
  fgets(l, TAM_MAX_MSG, stdin);
#else
  while (!fgets (l, TAM_MAX_MSG, stdin));
#endif
  l[strlen (l) - 1] = '\0';	// fgets considera [enter] como '\n'.
  return (unsigned char *) l;
}

void
erro (char *strerr)
{
  fprintf (stderr, "erro: %s\n", strerr);
  exit (EXIT_FAILURE);
}

int
enviar_direita (const unsigned char *buffer, unsigned int tam_buffer)
{
  return sendto (sock, buffer, tam_buffer, 0, (void *) &maquina_direita,
		 sizeof (struct sockaddr_in));
}
