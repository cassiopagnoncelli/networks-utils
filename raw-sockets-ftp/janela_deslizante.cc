#include "janela_deslizante.h"
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

// pacotes transferidos
long transf, taxa_transferencia;
bool transf_enviando;

#define min(A, B) A < B ? A : B

//public:
/* construtores/destrutor */
JanelaDeslizante::JanelaDeslizante(unsigned char tam, Socket* sock) 
{
   tamanho = tam;
   rawsock = sock;
}

JanelaDeslizante::~JanelaDeslizante() {}

/* controle externo */
bool
JanelaDeslizante::set_tamanho (unsigned int tam) 
{
   if (tam == 0) {
      fprintf (stderr, "aviso: janela configurada para ter tamanho 1.\n");
      tamanho = 1;
   } 

   tamanho = static_cast<unsigned char>(tam);
   return true;
}

unsigned char
JanelaDeslizante::get_tamanho ()
{
   return tamanho;
}


#ifndef SLEEP_MILI
#define SLEEP_MILI 2000
#endif

/* Calcula a taxa de transferencia por segundo */
void *txtransf(void *arg) {
   long transf_antigo;
   do {
      transf_antigo = transf;
      sleep(1);
      taxa_transferencia = (long) ((transf - transf_antigo) / 1024);
   } while (1);

   return arg;
}

/* Mostra a barra de progresso de transferencia de arquivos */
void *exibe_transf(void *num_pacotes) {
   long *num = reinterpret_cast<long *>(num_pacotes);
   long n = *num <= 0 ? 1 : *num;
   int i;
   double taxa;

   // taxa de transferencia
   bool matar_thread_txtransf = false;
   pthread_t thread_taxa_transf;
   if (!transf_enviando && pthread_create(&thread_taxa_transf, NULL, txtransf, NULL) == 0)
      matar_thread_txtransf = true;

   while (transf < *num) {
      taxa = (double) (100 * transf / n);
      printf("[");
      for (i=0; i<100; i++)
         printf("%c", i < taxa ? '-' : ' ');
      printf("] %.2f%%", taxa);
      if (!transf_enviando)
         printf(" a %ld KB/s", taxa_transferencia);
      putchar('\n');

      usleep(SLEEP_MILI * 1000);
   }

   if (!transf_enviando && matar_thread_txtransf)
      pthread_cancel(thread_taxa_transf);

   // 100%
   printf("[");
   int j;
   for (j=0; j<100; j++)
      printf("-");
   if (!transf_enviando)
      printf("] 100%% a 0 KB/s\n");
   else
      printf("]\n");

   return NULL;

/*
   // mostra a transferencia
   transf = 0;
   std::list<Pacote *>::iterator primeiro = m.pacotes.begin();
   pthread_t thread_transf;
   if ((**primeiro).get_tipo() == F) {
      unsigned int tam = static_cast<unsigned int> (m.pacotes.size());
      pthread_create(&thread_transf, NULL, &exibe_transf, &tam);
   }

   (...)

   if ((**primeiro).get_tipo() == F)
      pthread_join(thread_transf, NULL);
*/
}




/*****************************************
   Metodos de controle de fluxo:

    PARA_E_ESPERA,
    JANELAS_DESLIZANTES.

*****************************************/
/*
   Usando para-e-espera com timeout
   estado: funcionando
*/
#if (TROCA_MSGS == PARA_E_ESPERA)
bool
JanelaDeslizante::enviar (Mensagem &m) 
{
   // pacotes e timeout
   unsigned char sequencia = 0;
   Pacote resposta;
   std::list<Pacote *>::iterator i = m.pacotes.begin();
   unsigned int tout_exp = 0;

   do {
      // envia pacote
      (**i).set_sequencia(sequencia);
      (*rawsock).enviar(**i);

      // recebe resposta
      switch ((*rawsock).receber(resposta, tout_exp)) {
      case CORROMPIDO: {
         (*rawsock).incr_pacotes_corrompidos();
         /* volta para inicio do while */
      } break;
      case ERRO: {
         printf("erro de alocacao de memoria.\n");
      } break;
      case OK: {
         // ACK
         if (resposta.get_tipo() == Y) {
            if (resposta.get_sequencia() == (**i).get_sequencia()) {
               if ((**i).get_tipo() == S) 
                  return true;

               i++;
               sequencia++;
               transf++;

	       tout_exp = 0;
            } else { printf("{jd.enviar(): Y<s> != seq} "); }
         } else
         if (resposta.get_tipo() == N) {
            if (resposta.get_sequencia() == (**i).get_sequencia()) { /* volta p/ inicio do while */ }
            else printf("{jd.enviar()} ");
         }
      } break;
      case TIMEOUT: {
         tout_exp++;
	 if (tout_exp >= 15)
	    return false;
      } break;
      default: {} break;
      }
   } while (1);

   return true;
}

bool
JanelaDeslizante::receber (Mensagem &m)
{
   // vars para pacotes
   unsigned char sequencia = 0;
   Pacote
      *p,
      ack (Y, new unsigned char (0), '\0', '\0'),
      nack(N, new unsigned char (0), '\0', '\0');

   // recebe pacotes
   do {
      p = new Pacote();

      switch ((*rawsock).receber(*p)) {
      case CORROMPIDO: {
         (*rawsock).incr_pacotes_corrompidos();
         nack.set_sequencia(sequencia);
         (*rawsock).enviar(nack);
      } break;
      case ERRO: {
         fprintf(stderr, "erro de alocacao de memoria.\n");
      } break;
      case OK: {
         (*rawsock).incr_pacotes_ok();

         // verifica se eh o pacote certo
         if ((*p).get_sequencia() == sequencia) {
            // anexa pacote na mensagem
            m.pacotes.push_back(p);

            // envia ack do pacote recebido
            ack.set_sequencia(sequencia);
            (*rawsock).enviar(ack);

            // incrementa sequencia
            sequencia = (*p).get_sequencia();
            sequencia++;
         } 
         // nao eh o pacote certo
         else // nao fazem diferenca os numeros de sequencia em para-e-espera
         if ((*p).get_sequencia() != sequencia) {
            if (m.pacotes.size() == 0) {
               nack.set_sequencia(0);
               (*rawsock).enviar(nack);
            } else {
               ack.set_sequencia((*m.pacotes.back()).get_sequencia());
               (*rawsock).enviar(ack);
            }
         } else {}
      } break;
      case TIMEOUT: {} break;
      default: {} break;
      }
   } while ((*p).get_tipo() != 'S');

   return true;
}
#endif

/*
   Janelas Deslizantes
   estado: nao funciona
*/
#if (TROCA_MSGS == JANELAS_DESLIZANTES)
bool
JanelaDeslizante::enviar (Mensagem &m) 
{
   // pacotes e timeout
   unsigned char sequencia = 0;
   Pacote resposta;
   std::list<Pacote *>::iterator i;
   unsigned int tout_exp = 0;

   // controle da janela
   unsigned int inf = 0, sup = 0, j, offset;

   while (m.pacotes.size() > 0) {
      sup = inf + min(tamanho, (unsigned int)m.pacotes.size());
      i = m.pacotes.begin();
      for (j=inf; j<=sup; j++, i++) {
         (**i).set_sequencia((sequencia + (j-inf)%256)%256);
         (*rawsock).enviar(**i);
      }

      // recebe resposta
      switch ((*rawsock).receber(resposta, tout_exp)) {
      case CORROMPIDO: {
         (*rawsock).incr_pacotes_corrompidos();
         /* volta para inicio do while */
      } break;
      case ERRO: {
         fprintf(stderr, "erro de alocacao de memoria.\n");
      } break;
      case OK: {
         // ACK
	 tout_exp = 0;
         if (resposta.get_tipo() == Y) {
            if (*resposta.get_dados() < sequencia)
	       offset = (int) (1 + ((int) 255+(*resposta.get_dados())) - sequencia);
	    else
	       offset = (int) (1 + ((int) (*resposta.get_dados())) - sequencia);

            for (j=0; j<offset && m.pacotes.size()>0; j++)
	       m.pacotes.pop_front();
         } else
         if (resposta.get_tipo() == N) {
            if (*resposta.get_dados() < sequencia)
	       offset = (unsigned int) (((unsigned int) 255+(*resposta.get_dados())) - sequencia);
	    else
	       offset = (unsigned int) (((unsigned int) (*resposta.get_dados())) - sequencia);

	    for (j=0; j<offset && m.pacotes.size()>0; j++)
	       m.pacotes.pop_front();
         }

	 if (offset > 0)
	    transf++;
	 sequencia = (sequencia + offset) % 256;
      } break;
      case TIMEOUT: {
         tout_exp++;
	 if (tout_exp >= 15)
	    return false;
      } break;
      default: {} break;
      }
   }

   return true;
}

bool
JanelaDeslizante::receber (Mensagem &m)
{
   // vars para pacotes
   unsigned char sequencia = 0;
   Pacote
      *p,
      ack (Y, new unsigned char (0), '\0', '\0'),
      nack(N, new unsigned char (0), '\0', '\0');

   bool enviar_ack;
   unsigned int j;

   // recebe pacotes
   do {
      for (j=0; j<tamanho; j++) {
         enviar_ack = false;
         p = new Pacote();
         switch ((*rawsock).receber(*p)) {
         case CORROMPIDO: {
            (*rawsock).incr_pacotes_corrompidos();
            nack.set_sequencia(sequencia);
            (*rawsock).enviar(nack);
	    j = tamanho;
         } break;
         case ERRO: {
            printf("erro de alocacao de memoria.\n");
         } break;
         case OK: {
            (*rawsock).incr_pacotes_ok();
            // anexa pacote na mensagem
            m.pacotes.push_back(p);

            // incrementa sequencia
            sequencia++;

            // marca ack para ser enviado, se tudo der certo
	    enviar_ack = true;
         } break;
         case TIMEOUT: {
	    return false;
	 } break;
         default: {} break;
         }
      }

      if (enviar_ack) {
         ack.set_sequencia(sequencia);
         (*rawsock).enviar(ack); 
      } else {
         nack.set_sequencia(sequencia);
         (*rawsock).enviar(nack);
      }
   } while ((*p).get_tipo() != 'S');

   return true;
}
#endif

