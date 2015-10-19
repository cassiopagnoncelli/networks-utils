#include "diretorio.h"
#include <features.h>  // _GNU_SOURCE necessario para realpath()
#include <climits>     // limites para realpath()
#include <errno.h>     // realpath() pode retornar erros
#include <cstdlib>     // realpath()
#include <cstdio>      // FILE
#include <cstring>     // strcpy
#include <sys/types.h> // necessario para opendir() e regex
#include <dirent.h>    // opendir()

//public:
/* construtores/destrutor */
Diretorio::Diretorio() 
{
   diretorio_atual.assign (realpath (".", NULL));
}

Diretorio::~Diretorio() {}


/* metodos de proposito geral */
std::string
Diretorio::get_dir()
{
   return diretorio_atual;
}

std::string
Diretorio::formata_caminho(std::string novo)
{
   if (novo.length() == 0)
      return get_dir();

   std::string destino;

   // endereco absoluto
   if (novo.substr(0, 1) == "/") {
      char *rp = realpath(novo.c_str(), NULL);
      if (rp && strlen(rp) > 0)
         destino.assign(rp);
   }
   // endereco relativo
   else {
      std::string prep(get_dir());
      if (prep.length() > 1)
         prep.append("/");
      prep.append(novo);

      char *rp = realpath(prep.c_str(), NULL);
      if (rp && strlen(rp) > 0)
         destino.assign(rp);
   }

   return destino;
}

std::string
Diretorio::formata_caminho(const char *novo)
{
   std::string a(novo ? novo : "");
   return formata_caminho(a);
}

bool
Diretorio::caminho_valido(std::string caminho)
{
   // faz backup de `errno'
   int erro = errno;
   bool ret = realpath(caminho.c_str(), NULL) != NULL;
   errno = erro;

   return ret;
}

bool
Diretorio::caminho_valido(const char *caminho)
{
   std::string a(caminho ? caminho : "");
   return caminho_valido(a);
}

bool
Diretorio::caminho_valido_eh_dir(std::string caminho)
{
   DIR *d = opendir(caminho.c_str());
   if (d) {
      closedir(d);
      return true;
   } else
      return false;
}

bool
Diretorio::caminho_valido_eh_dir(const char *caminho)
{
   std::string a(caminho ? caminho : "");
   return caminho_valido_eh_dir(a);
}

bool
Diretorio::caminho_valido_eh_arquivo(std::string caminho)
{
   // eh diretorio
   DIR *dd = opendir(caminho.c_str());
   if (dd != NULL) {
      closedir(dd);
      return false;
   } 
   // nao eh diretorio e eh arquivo
   else { 
      int errno_bkp = errno;
      FILE *fp = fopen(caminho.c_str(), "r");
      bool ret = (fp != NULL);
      if (fp) fclose(fp);
      errno = errno_bkp;
      return ret;
   }
}

bool
Diretorio::caminho_valido_eh_arquivo(const char *caminho)
{
   std::string a(caminho ? caminho : "");
   return caminho_valido_eh_arquivo(a);
}

int
Diretorio::altera_dir(std::string novo_dir)
{
   // formata o caminho como um endereco absoluto
   int errno_bkp = errno; // faz backup de `errno'
   errno = 0; 
   std::string caminho(formata_caminho(novo_dir));
   if (errno > 0) return errno; // endereco nao valido
   errno = errno_bkp; // restaura backup de `errno'

   // tenta trocar de diretorio
   if (caminho_valido(caminho) && caminho_valido_eh_dir(caminho)) {
      diretorio_atual.assign(caminho);
      return 0;
   } else {
      int ret = errno;
      errno = errno_bkp;
      return ret;
   }
}

int
Diretorio::altera_dir(const char *novo_dir)
{
   std::string a(novo_dir ? novo_dir : "");
   return altera_dir(a);
}

/* especificos para classe de comandos `ls' */
std::string
Diretorio::normaliza_args_ls(std::string args)
{
   std::string *norm = new std::string;

   char *t, *pre_token = strcpy(new char[args.length() + 1], args.c_str());
   std::string primeiro_token(
      (t=strtok(pre_token, " ")) ? t : "");

   std::string end_absoluto(formata_caminho(primeiro_token));
   if (!caminho_valido(end_absoluto))
      return (*norm);

   // monta argumento formado pelo diretorio...
   (*norm).assign(end_absoluto);
   (*norm).append(" ");

   // ...e pelo restante das opcoes iniciais
   char *resto = strcpy(new char[args.length() + 1], args.c_str());
   unsigned int i; bool continua = true;
   for (i=0; i<strlen(resto) && continua; i++)
      if (resto[i] == ' ') {
         continua = false;
         (*norm).append(resto + i + 1);
      }

   return (*norm);
}

std::string
Diretorio::normaliza_args_ls(const char *args)
{
   std::string a(args ? args : "");
   return normaliza_args_ls(a);
}

