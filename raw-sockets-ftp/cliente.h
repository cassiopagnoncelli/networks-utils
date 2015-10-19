#ifndef cliente_h
#define cliente_h

#include "socket.h"
#include "diretorio.h"
#include "janela_deslizante.h"
#include <cstdio>   // size_t
#include <iostream> // std::string

/*
   Descreve suscintamente as operacoes em alto nivel de abstracao o lado
   cliente da aplicacao. Uma instancia de Cliente representa uma maquina
   atuando no lado cliente da aplicacao sendo a ela associado um 
   dispositivo de rede. Usa-se Janelas deslizantes na troca de mensagens.
*/
class Cliente {
protected:
     JanelaDeslizante jd;
     Socket           sock;
     Diretorio        local;
private:
/* metodos de proposito geral do cliente */
     std::string* req_linha();
     bool         comando_valido(std::string& str);
     bool         eh_comando_local(std::string& str);
     void         processa_comando_local(std::string& cmd);
     void         processa_comando_remoto(std::string& cmd); //!*
public:
/* construtores/destrutor */
     Cliente(std::string& interface_rede);
    ~Cliente();
/* roda aplicacao cliente */
     void executa();
};

#endif /* cliente.h */
