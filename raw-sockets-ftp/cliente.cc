#include "cliente.h"
#include <cstring>     // fgets()
#include <cstdlib>     // exit(), system()
#include <sys/types.h> // constantes de expressoes regulares
#include <regex.h>     // expressoes regulares
#include <errno.h>     // constantes de codigo de erro
#include <unistd.h>    // usleep()

//private
/* metodos de proposito geral */
std::string* 
Cliente::req_linha () 
{
   setvbuf(stdin, NULL, _IONBF, 0);
   fprintf(stdout, ">> ");
   char *l = new char[1026]; // 1K + '\n' + '\0' = 1K + 2
   while (!fgets (l, 1024, stdin)); // recebe comando com tamanho de ate 1K.
   l[strlen(l) - 1] = '\0'; // fgets considera [enter] como '\n'.
   std::string* s = new std::string(l);
   delete [] l;
   return s;
}

bool
Cliente::comando_valido(std::string& str) 
{
   bool valido = false;
   regex_t er;

   if (regcomp (&er, "^ls$", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else
   if (regcomp (&er, "^ls [[:alnum:]|[:cntrl:]|[:punct:]|[:space:]]+$", 
          REG_EXTENDED|REG_NOSUB)==0 && regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else 
   if (regcomp (&er, "^cd$", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   if (regcomp (&er, "^cd [[:alnum:]|[:cntrl:]|[:punct:]|[:space:]]+$", 
          REG_EXTENDED|REG_NOSUB)==0 && regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else 
   if (regcomp (&er, "^put [[:alnum:]|[:cntrl:]|[:punct:]|[:space:]]+$", 
          REG_EXTENDED|REG_NOSUB)==0 && regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else 
   if (regcomp (&er, "^get [[:alnum:]|[:cntrl:]|[:punct:]|[:space:]]+$", 
          REG_EXTENDED|REG_NOSUB)==0 && regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else 
   if (regcomp (&er, "^lls$", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else 
   if (regcomp (&er, "^lls [[:alnum:]|[:cntrl:]|[:punct:]|[:space:]]+$", 
          REG_EXTENDED|REG_NOSUB)==0 && regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else 
   if (regcomp (&er, "^lcd$", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else
   if (regcomp (&er, "^lcd [[:alnum:]|[:cntrl:]|[:punct:]|[:space:]]+$", 
          REG_EXTENDED|REG_NOSUB)==0 && regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else 
   if (regcomp (&er, "^quit$", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else 
   if (regcomp (&er, "^exit$", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;
   else 
   if (regcomp (&er, "^help$", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      valido = true;

   regfree (&er);
   return valido;
}

bool
Cliente::eh_comando_local(std::string& str) 
{
   bool eh_cmd_local = false;
   regex_t er;

   if (regcomp (&er, "^lls", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      eh_cmd_local = true;
   else
   if (regcomp (&er, "^lcd", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      eh_cmd_local = true;
   else 
   if (regcomp (&er, "^quit$", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      eh_cmd_local = true;
   else 
   if (regcomp (&er, "^exit$", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      eh_cmd_local = true;
   else 
   if (regcomp (&er, "^help$", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, str.c_str(), 0, 0, 0)==0)
      eh_cmd_local = true;
   else
   if (regcomp (&er, "^put [[:alnum:]|[:cntrl:]|[:punct:]|[:space:]]+$", 
          REG_EXTENDED|REG_NOSUB)==0 && regexec (&er, str.c_str(), 0, 0, 0)==0) {
      if (str.length() >= 5) {
         FILE *fp = fopen((local.formata_caminho(str.c_str() + 4)).c_str(), "r");
         if (fp)
            fclose(fp);
         else {
            eh_cmd_local = true;
            switch (errno) {
            case 2: {
               fprintf(stdout, "Esse arquivo nao existe.\n");
            } break;
            case 13: {
               fprintf(stdout, "Voce nao tem permissao para acessar esse arquivo\n");
            } break;
            default: {
               fprintf(stderr, "Nao foi possivel abrir o arquivo.\n");
            } break;
            }
         }
      } else
         eh_cmd_local = true;
   }

   regfree (&er);
   return eh_cmd_local;
}

void
Cliente::processa_comando_local(std::string& cmd) 
{
   regex_t er;
   char *ponto = new char [2]; ponto[0]='.'; ponto[1]='\0';

   // ls local
   if (regcomp (&er, "^lls", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, cmd.c_str(), 0, 0, 0)==0) 
   {
      std::string destino("ls ");

      int errno_bkp = errno; errno = 0;
      destino.append(local.normaliza_args_ls(
         cmd.length() >= 4 ? cmd.c_str() + 4 : ponto));
      int ret_err = errno; errno = errno_bkp;
#ifdef LS_COMO_LL
      destino.append(" -l --color");
#endif
      
      if (ret_err == 0) {
         if (system (destino.c_str()) == -1) 
            printf ("erro: ** chamada de sistema system().\n");
      } else {
         switch (ret_err) {
         case EACCES:
            printf ("ls: permissao negada.\n");
            break;
         case ENOENT:
            printf ("ls: diretorio nao existe ou o argumento invalido.\n");
            break;
         case ENOTDIR:
            printf ("ls: o argumento nao eh um diretorio.\n");
            break;
         default:
            printf ("ls: erro.\n");
            break;
         }
      }
   } else
   // cd local
   if (regcomp (&er, "^lcd", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, cmd.c_str(), 0, 0, 0)==0) 
   {
      std::string destino(
         cmd.length() >= 4 ? cmd.c_str() + 4 : ponto);

      int errno_bkp = errno; errno = 0;
      switch (local.altera_dir (destino)) {
      case 0:
         fprintf (stdout, "mudando diretorio para %s\n", 
                          (local.get_dir()).c_str());
      break;
      case EACCES:
         fprintf (stderr, "cd: acesso negado, sem permissao.\n");
         break;
      case EINVAL:
         fprintf (stderr, "cd: endereco nao resolvido.\n");
         break;
      case EIO:
         fprintf (stderr, "cd: ocorreu um erro de entrada/saida.\n");
      break;
      case ELOOP:
         fprintf (stderr, "cd: muitos links simbolicos no destino.\n");
      break;
      case ENAMETOOLONG:
         fprintf (stderr, "cd: diretorio com nome muito longo.\n");
       break;
      case ENOENT: 
         fprintf (stderr, "cd: diretorio nao existente.\n");
      break;
      case ENOTDIR:
         fprintf (stderr, "cd: componente do argumento invalido.\n");
      break;
      default:
         fprintf (stderr, "cd: erro desconhecido.\n");
      break;
      }

      errno = errno_bkp;
   } else
   // help, mostra os comandos validos
   if (regcomp (&er, "^help", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, cmd.c_str(), 0, 0, 0)==0) 
   {
       fprintf (stderr, "Comandos validos:\
                     \n\t%s \n\t%s \n\t%s \n\t%s \n\t%s \n\t%s \n\t%s \n%s \n%s \n",
                     "ls [DIR [OPCOES..]]", 
                     "cd [DIR_REMOTO [OPCOES..]]",
                     "put ARQUIVO_LOCAL", 
                     "get ARQUIVO_REMOTO",
                     "lls [DIR [OPCOES..]]", 
                     "lcd [DIR [OPCOES..]]",
                     "quit",
                     "help",
                     "[OP]: indica OP como parametro opcional.");     
   }

   regfree (&er);
}

void
Cliente::processa_comando_remoto(std::string& cmd) 
{
   regex_t er;
   if ((regcomp (&er, "^ls", REG_EXTENDED|REG_NOSUB)==0 && // ls, ou
        regexec (&er, cmd.c_str(), 0, 0, 0)==0) ||
       (regcomp (&er, "^cd", REG_EXTENDED|REG_NOSUB)==0 && // cd, ou
        regexec (&er, cmd.c_str(), 0, 0, 0)==0) ||
       (regcomp (&er, "^get", REG_EXTENDED|REG_NOSUB)==0 && // get, ou
        regexec (&er, cmd.c_str(), 0, 0, 0)==0) || 
       (regcomp (&er, "^put", REG_EXTENDED|REG_NOSUB)==0 && // put
        regexec (&er, cmd.c_str(), 0, 0, 0)==0)
      )
   {
      Mensagem p1(&local);
      p1.set_comando(cmd);
      if (p1.prepara_pergunta(0)) {
         if (jd.enviar(p1)) {
            Mensagem r(&local);
            if (jd.receber(r)) {
               r.processa_resposta_no_cliente(p1.tipo_linha_comando());
               if (!r.mensagem_de_erro() && // segundo estagio do put
                   (regcomp (&er, "^put", REG_EXTENDED|REG_NOSUB)==0 && 
                    regexec (&er, cmd.c_str(), 0, 0, 0)==0)) 
               {
                  Mensagem arquivo(&local);
                  arquivo.set_comando(cmd);
                  arquivo.prepara_pergunta(1);
                  if (!jd.enviar(arquivo))
                     fprintf (stdout, "timeout: arquivo nao foi enviado.\n");
                  else 
                     fprintf (stdout, "Arquivo enviado.\n");
               }
            } else {
               fprintf (stdout, "timeout: Impossível receber.\n");
            }
         } else {
            fprintf (stdout, "timeout: Impossivel enviar.\n");
         }
      } else {
         fprintf (stdout, "[prepara_pergunta()]: ** erro local **\n");
      }
   }

   regfree (&er);

   // forca escrita de stdout, de modo a nao comprometer o buffer
   // da entrada de comandos na linha de comando no cliente
   //(void) fflush(stdout);
}


//public
/* construtores/destrutor */
Cliente::Cliente (std::string& interface_rede) : jd(1, &sock)
{
   // erro
   if (!sock.cria_socket (interface_rede)) {
      fprintf (stderr, "erro: ** inicializacao do cliente.\n");
      exit (EXIT_FAILURE);
   } 
}

Cliente::~Cliente () {}

void
Cliente::executa () 
{
   std::string *s;
   do {
      usleep(10000);
      s = req_linha();
      if (s && comando_valido (*s)) 
      {
         // local
         if (eh_comando_local (*s))
            processa_comando_local (*s);
         // remoto
         else 
            processa_comando_remoto (*s);
      }
   } while (s && (*s).compare("quit") != 0 && (*s).compare("exit") != 0);
}

