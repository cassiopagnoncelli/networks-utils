#ifndef mensagem_h
#define mensagem_h

#include "pacote.h"
#include "diretorio.h"
#include <list>     // lista duplamente ligada
#include <iostream> // std::string

/*
   Declarar erros remotos possiveis aqui.
   Arrumar apenas buffer_para_arquivo() para retornar os dois possiveis
   erros: espaço insuficiente e sem permissao de escrita. errno eh setado
   quando NULL eh devolvido, portanto pode-se enviar a mensagem de erro
   do proprio sistema quando NULL eh devolvido. ver ferror()
*/

/* 
   Uma mensagem (ls, cd, put, ...) pode ser vista de dois 
   modos:

   (1) um comando (possivelmente a ser interpretado localmente)
       a ser convertido em um grupo de pacotes, com finalidade
       de ser enviado por cliente/servidor; ou,
   
   (2) um grupo de pacotes a ser convertido para a mensagem
       original --isso pode ser um comando ou um arquivo--, com
       finalidade de ser interpretado pelo cliente/servidor.
*/
class Mensagem {
protected:
/* dados da mensagem */
     std::string        linha_cmd;
     Diretorio*         dir_local;
public:
/* manipulacao interna de pacotes */
     void         segmenta_buffer_em_pacotes (enum pacote_t tipo,
                                               const void *buffer, size_t tam);
/* arquivos */
     unsigned int arquivo_para_buffer (const char *arquivo, char **buffer);
     bool         buffer_para_arquivo (const char *buffer, unsigned int tam, 
                                       const char *arquivo);
     unsigned int ls_para_buffer (const char *args, char **buffer);
     bool         pacotes_para_arquivo ();
     long         tamanho_arquivo (const char *arquivo);
     long         tamanho_arquivo ();

     std::list<Pacote*> pacotes;

/* construtores/destrutor */
     Mensagem(Diretorio* diretorio_local = NULL);
    ~Mensagem();
/* interpretacao de comandos */
     void          set_comando (std::string linha);
     std::string   get_comando ();
     enum pacote_t tipo_linha_comando (); 
     std::string   arg_comando ();
     bool          comando_valido (); 
     enum pacote_t interpreta_mensagem ();//?*
/* manipulacao de grupos de pacotes (requerem `linha_cmd' preenchida) */
     bool          prepara_pergunta (int estagio); //?*
     bool          prepara_resposta (); //?*
     void          enfila_pacote (Pacote *p);
     Pacote*       desenfila_pacote ();
/* metodos de proposito geral */
     bool          processa_resposta_no_cliente (enum pacote_t t);//?*
     bool          processa_resposta_no_servidor(enum pacote_t t); //!*
     bool          mensagem_de_erro ();
     std::string   remonta_pacotes_como_string();
/* debug */
     void          imprime_pacotes();
};

#endif /* mensagem.h */
