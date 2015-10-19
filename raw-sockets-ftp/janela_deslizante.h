#ifndef janela_deslizante_h
#define janela_deslizante_h

#include "socket.h" 
#include "pacote.h"
#include "mensagem.h"

#ifndef PARA_E_ESPERA
#define PARA_E_ESPERA       1
#endif
#ifndef JANELAS_DESLIZANTES
#define JANELAS_DESLIZANTES 2
#endif
#ifndef TROCA_MSGS
#define TROCA_MSGS PARA_E_ESPERA
#endif

/*
   O tamanho maximo da janela nao pode exceder o campo
   de representacao do campo `sequencia' do protocolo
   cujo tamanho mede 8 bits pois, nesse caso extremo, 
   o primeiro e ultimo pacote terao o mesmo numero de 
   sequencia, tornando-os nao identificaveis. Assim,
   o tamanho maximo da janela deslizante mede

   max |JD| = 1 + 2 + 2^2 + 2^3 + ... + 2^(8-1)
            = 2^0 + 2^1 + 2^2 + ... + 2^7
            = 2^8 - 1
            = 255
*/
#define JANELA_DESLIZANTE_MAX_TAM 255

/*
   Controle geral da janela deslizante.
*/
class JanelaDeslizante {
protected:
     unsigned char tamanho;
     Socket*       rawsock;
public:
/* construtores/destrutor. */
     JanelaDeslizante(unsigned char tam, Socket* sock);
    ~JanelaDeslizante();
/* controle externo. */
     bool          set_tamanho (unsigned int tam);
     unsigned char get_tamanho ();
/* troca de mensagens. devolve F em caso de timeout, ou V em caso de sucesso. */
     bool          enviar (Mensagem &m);
     bool          receber (Mensagem &m);
};

#endif /* janela_deslizante.h */
