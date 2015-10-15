#ifndef tokenring_h
#define tokenring_h

/* inicia a maquina e roda a maquina nó do token ring */
void rodar(char *nome_host, int num_maquina);

#ifdef conta_bastao
/* mostra estatisticas da passagem do bastao imprimindo
 * na tela quantas vezes o bastao passou por essa maquina
 * nos ultimos 10s.
 */
void *contagem_bastao(void *arg);
#endif

/* recebe do teclado e enfila a nova mensagem */
void *recebe_teclado(void *arg);

/* escuta por nova mensagem na rede */
void *escuta_rede(void *arg);

/* recria o bastao apos o timeout */
void *timeout_bastao(void *arg);

/* pede para usuario digitar alguma coisa */
unsigned char *req_linha();

void erro(char *strerr);

int enviar_direita(const unsigned char *buffer, unsigned int tam_buffer);

#endif /* tokenring.h */
