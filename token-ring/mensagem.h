#ifndef mensagem_h
#define mensagem_h

#define TAM_MAX_MSG 256
struct mensagem_t {
// tamanho em bytes
  unsigned int tam_msg;
// protocolo
  unsigned char origem;
  unsigned char tipo;
  unsigned char tam_dados;
  unsigned char dados[TAM_MAX_MSG];
// fila
  struct mensagem_t *anterior, *proximo;
};
typedef struct mensagem_t mensagem;

enum {
  BASTAO = 'B',
  TEXTO  = 'T'
};


// gera uma nova mensagem com finalidade de ser 
// enviada para maquina da direita
mensagem *nova_mensagem(mensagem *msg, unsigned char origem, unsigned char tipo, unsigned char *d);

// converte a mensagem em buffer para enviar
unsigned char *mensagem_para_buffer(mensagem *m);

// remonta mensagem original a partir do buffer
mensagem *buffer_para_mensagem(const unsigned char *buffer);

// devolve 1 se a mensagem for um bastao, 0 c.c.
int eh_bastao(const unsigned char *buffer);

// imprime mensagem em stdout
void imprime_mensagem(mensagem *m);


/*
   fila de mensagens
*/
typedef struct {
   mensagem *primeiro, *ultimo;
} fila_mensagens;

fila_mensagens *inicializa_fila(fila_mensagens *f);

fila_mensagens *enfila_mensagem(fila_mensagens *f, mensagem *m);

mensagem *desenfila_mensagem(fila_mensagens *f);

int fila_vazia(fila_mensagens *f);

#endif /* mensagem.h */
