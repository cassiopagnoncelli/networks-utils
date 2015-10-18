#ifndef msg_h
#define msg_h

typedef enum {expr_e='E', nova_maq='N'} msg_tipo;

typedef struct msg_t {
  char tp;
  char expressao[35];
  char nome_host[35];
  int porta;
} msg;

msg *buffer_para_msg(unsigned char *);
unsigned char *msg_para_buffer(msg *);

msg *nova_expressao(unsigned char *);
msg *nova_maquina(char *, int);

#endif /* msg.h */
