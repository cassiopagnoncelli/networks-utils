#ifndef socket_h
#define socket_h

#include "pacote.h"
#include <iostream>  // std::string
#include <pthread.h>
#include <list>

/* Metodos de controle de fluxo*/
#ifndef PARA_E_ESPERA
#define PARA_E_ESPERA       1
#endif
#ifndef JANELAS_DESLIZANTES
#define JANELAS_DESLIZANTES 2
#endif
#ifndef TROCA_MSGS
#define TROCA_MSGS PARA_E_ESPERA
#endif

enum resp_t {TIMEOUT, CORROMPIDO, OK, ERRO};

/*
   Interface com a placa de rede.
*/
class Socket {
protected:
/* descritor de arquivo do raw socket e codigo de erro. */
     int raw_socket;
     bool houve_erro;

/* timeout exponencial herdado do kermit, vetor de tempos, em segundos. */
     unsigned int timeout_exp[16];

public:
/* estatisticas dos pacotes */
     unsigned int 
        pacotes_ok,          /* pacotes consistentes */
        pacotes_corrompidos; /* erros de recebimento + pacotes corrompidos. */
//public:
/* construtor/destrutor. */
     Socket ();
    ~Socket ();

/* propriedades. */
     bool cria_socket (std::string& disp);
     int  get_fd (); 
     bool erro (); 
     void incr_pacotes_ok();
     void incr_pacotes_corrompidos();

/* 
   I/O da rede
*/
     bool enviar (Pacote &p);
     enum resp_t receber (Pacote &p, unsigned int tentativa = 0);
     void limpa_buffer();
};

#endif /* socket.h */
