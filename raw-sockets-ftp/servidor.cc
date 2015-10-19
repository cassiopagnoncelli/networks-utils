#include "servidor.h"
#include "mensagem.h"
#include "pacote.h"
#include <cstdlib>
#include <cstring>

//public
/* Servidor */
Servidor::Servidor (std::string& interface_rede) : jd(1, &sock)
{
   if (!sock.cria_socket (interface_rede)) {
      std::cout << "erro: ** inicializacao do servidor." << std::endl;
      exit (EXIT_FAILURE);
   }

   jd.set_tamanho(1);
}

Servidor::~Servidor () {}

void
Servidor::executa () 
{
   Mensagem *cmd = NULL, *resp = NULL, *put = NULL;
   enum pacote_t tp;
   while (1) {
      cmd = new Mensagem(&local);
      if (jd.receber (*cmd)) { // recebeu um pacote
         tp = (*cmd).interpreta_mensagem();
         if (tp == L || tp == C || tp == G || tp == P) {
            resp = new Mensagem(&local);
            (*resp).set_comando((*cmd).get_comando());
            (*resp).prepara_resposta();
            if (jd.enviar(*resp)) {
               if (tp == P) {
                  put = new Mensagem(&local);
                  if (jd.receber(*put)) {
                     if (!(*put).processa_resposta_no_servidor(tp))
                        fprintf (stdout, 
                           "[processa_resposta_no_servidor()]: impossivel gerar o arquivo.\n");
                  } else
                     fprintf (stdout, "timeout: arquivo nao recebido.\n");
                  
                  delete put;
               }
            } else 
               fprintf (stdout, "timeout: tempo limite de envio de resposta.\n");

            //fflush(stdout);
         } else 
         if (tp == J) {
            unsigned char *buffer = new unsigned char; 
            *buffer = jd.get_tamanho();
            if (!sock.enviar(*(new Pacote (J, buffer, 1))))
               std::cout << "[socket.enviar()]: pacote nao enviado.\n";
         } else
            std::cout << "Ruido recebido e descartado." << std::endl;

         delete resp;
      }

      delete cmd;
   }
}
