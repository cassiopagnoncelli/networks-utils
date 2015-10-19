#ifndef diretorio_h
#define diretorio_h

#include <iostream> // std::string
#include <errno.h>  // errno

/*
   Manipulacao de diretorios e arquivos com caminho absoluto ou relativo.
*/
class Diretorio {
protected:
     std::string diretorio_atual;
public:
/* construtores/destrutor */
     Diretorio();
    ~Diretorio();
/* metodos de proposito geral */
     std::string get_dir ();
     // em caso de erro, `errno' sera modificado nos metodos a seguir
     std::string formata_caminho (std::string novo);
     std::string formata_caminho (const char *novo);
     bool        caminho_valido (std::string caminho);
     bool        caminho_valido (const char *caminho);
     bool        caminho_valido_eh_dir (std::string caminho);
     bool        caminho_valido_eh_dir (const char *caminho);
     bool        caminho_valido_eh_arquivo (std::string caminho);
     bool        caminho_valido_eh_arquivo (const char *caminho);
     int         altera_dir (std::string novo_dir);
     int         altera_dir (const char *novo_dir);
/* especificos para classe de comandos `ls' */
     std::string normaliza_args_ls (std::string args);
     std::string normaliza_args_ls (const char *args);
};

#endif /* diretorio.h */
