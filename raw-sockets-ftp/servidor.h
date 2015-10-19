#ifndef servidor_h
#define servidor_h

#include "socket.h"
#include "diretorio.h"
#include "janela_deslizante.h"
#include "mensagem.h"
#include <cstdio>   // size_t
#include <iostream> // std::string

/*
   Descreve suscintamente as operacoes em alto nivel de abstracao o lado
   servidor da aplicacao. Uma instancia de Servidor representa uma maquina
   atuando no lado servidor da aplicacao sendo a ela associado um 
   dispositivo de rede. Usa-se Janelas deslizantes na troca de mensagens.
*/
class Servidor {
protected:
     JanelaDeslizante jd;
     Socket           sock;
     Diretorio        local;
public:
/* construtores/destrutor */
     Servidor(std::string& interface_rede);
    ~Servidor();
/* roda aplicacao servidor */
     void executa();
};

#endif /* servidor.h */
