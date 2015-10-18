#include "msg.h"
#include <stdlib.h>
#include <string.h>

msg *buffer_para_msg(unsigned char *b) {
  unsigned char *p = b;
  msg *m = malloc(sizeof(msg));

  memcpy(&m->tp, p, 1); p++;
  memcpy(m->expressao, p, 35); p += 35;
  memcpy(&m->porta, p, sizeof(int));

  return m;
}

unsigned char *msg_para_buffer(msg *m) {
  unsigned char *b = malloc(150 * sizeof(unsigned char));
  bzero(b, 150);
  unsigned char *p = b;

  memcpy(p, &m->tp, 1); p++;
  memcpy(p, m->expressao, 35); p += 35;
  memcpy(p, m->nome_host, 35); p += 35;
  memcpy(p, &m->porta, sizeof(int)); p += sizeof(int);

  return b;
}

msg *nova_expressao(unsigned char *e) {
  msg *m = malloc(sizeof(msg));

  m->tp = expr_e;
  memcpy(m->expressao, e, 35);

  return m;
}

msg *nova_maquina(char *nome_host, int porta) {
  msg *m = malloc(sizeof(msg));

  m->tp = nova_maq;
  memcpy(m->nome_host, nome_host, 35);
  m->porta = porta;

  return m;
}
