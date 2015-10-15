#include "mensagem.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

mensagem *nova_mensagem(mensagem *msg, unsigned char origem, unsigned char tipo, unsigned char *d) 
{
   if (!msg) return NULL;

   msg->origem = origem;
   msg->tipo = tipo;

   if (!d || strlen((char *)d) == 0) {
      msg->dados[0] = '\0';
      msg->tam_dados = 0;
   } else {
      msg->tam_dados = (unsigned char) strlen((char *)d);
      memcpy(&msg->dados[0], d, msg->tam_dados);
      msg->dados[msg->tam_dados] = '\0';
   }

   msg->tam_dados++; // |'\0'|

   // tam_msg = |o| + + |t| + |t_dados| + tam_dados 
   //         = 3 + tam_dados
   msg->tam_msg = 3 + msg->tam_dados;

   msg->anterior = NULL;
   msg->proximo = NULL;

   return msg;
}

unsigned char *mensagem_para_buffer(mensagem *m) 
{
   unsigned char *buf = malloc(sizeof(unsigned char) * m->tam_msg);
   unsigned char *pbuf = buf;

   memcpy(pbuf, &m->origem, 1); pbuf++;
   memcpy(pbuf, &m->tipo, 1); pbuf++;
   memcpy(pbuf, &m->tam_dados, 1); pbuf++;
   if (m->tam_dados > 0)
      memcpy(pbuf, m->dados, m->tam_dados);

   return buf;
}

// remonta mensagem original a partir do buffer
mensagem *buffer_para_mensagem(const unsigned char *buffer) 
{
   unsigned char *pb = (unsigned char *)buffer;
   if (!pb) {
      fprintf(stderr, "buffer invalido.\n");
      return NULL;
   }

   mensagem *m = malloc(sizeof(mensagem));
   if (!m) {
      fprintf(stderr, "malloc(): memoria insuficiente.\n");
      return NULL;
   }

   m->origem = *pb; pb++;
   m->tipo = *pb; pb++;
   m->tam_dados = *pb; pb++;

   if (m->tam_dados > 0)
      memcpy(&m->dados[0], pb, m->tam_dados);

   m->tam_msg = 3 + m->tam_dados;

   m->anterior = NULL;
   m->proximo = NULL;

   return m;
}

// devolve 1 se a mensagem for um bastao, 0 c.c.
int eh_bastao(const unsigned char *buffer) 
{
   return *(buffer + 1) == BASTAO;
}

// imprime mensagem em stdout
void imprime_mensagem(mensagem *m) 
{
   if (!m) 
      return;

#ifdef msg_formatada
   fprintf(stdout, 
   "# |m|=%u o=%u t=%c tam_dados=%u dados=%s\n",
      m->tam_msg,
      (unsigned int)m->origem,
      m->tipo,
      (unsigned int)m->tam_dados,
      (char*)m->dados);
#else
   fprintf(stdout, "%s\n", (char*)m->dados);
#endif
}


fila_mensagens *inicializa_fila(fila_mensagens *f) {
   if (!f) return NULL;
   f->primeiro = NULL;
   f->ultimo = NULL;
   return f;
}

fila_mensagens *enfila_mensagem(fila_mensagens *f, mensagem *m) {
   if (f == NULL || m == NULL) return NULL;

   if (f->primeiro == NULL || f->ultimo == NULL) {
      f->ultimo = m;
      f->primeiro = m;
   } else {
      m->proximo = f->primeiro;
      f->primeiro->anterior = m;
      f->primeiro = m;
   }

   return f;
}

mensagem *desenfila_mensagem(fila_mensagens *f) {
   if (!f) return NULL;
   if (fila_vazia(f)) return NULL;

   mensagem *desenfila = f->ultimo, *i = f->primeiro;
   if (f->primeiro == f->ultimo) {
      f->primeiro = NULL;
      f->ultimo = NULL;
   } else {
      while (i) {
         if (i->proximo == f->ultimo) {
            f->ultimo = i;
            f->ultimo->proximo = NULL;
            i = NULL;
         } else
            i = i->proximo;
      }
   }

   return desenfila;
}

int fila_vazia(fila_mensagens *f) {
   return f->ultimo == NULL;
}
