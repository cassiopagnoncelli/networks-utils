#include "mensagem.h"
#include <regex.h>
#include <cstdio>     // fgets() 
#include <features.h> // necessario para strtok()/strsep()
#include <cstring>    // strtok()
#include <cerrno>     // errno (usado para fwrite() e fread())
#include <cstdlib>    // system()
#include <unistd.h>   // write()

/* manipulacao interna de pacotes */
void
Mensagem::segmenta_buffer_em_pacotes (enum pacote_t tipo, const void *buffer, 
                                      size_t tam) 
{
   if (tam == 0) {
      char *final = new char; *final = '\0';
      enfila_pacote (new Pacote (tipo, final, static_cast<unsigned char>(1)));
      return;
   }

   size_t scroll = (tam>PACOTE_MAX_TAM_DADOS) ? PACOTE_MAX_TAM_DADOS : tam;
   char *bb = static_cast<char *> (const_cast<void *> (buffer));
   enfila_pacote(new Pacote(tipo, bb, static_cast<unsigned char>(scroll)));

   if (tam > PACOTE_MAX_TAM_DADOS)
      segmenta_buffer_em_pacotes (tipo, bb + scroll, tam - scroll);
}

/* arquivos */
unsigned int
Mensagem::arquivo_para_buffer (const char *arquivo, char **buffer) 
{
   FILE *fp = fopen (arquivo, "rb");
   if (!fp) 
      return 0;

   unsigned int retorno = 0; 
   size_t tam_buffer = 0;
   while (fgetc (fp) != EOF)
      tam_buffer++;

   rewind (fp);
   retorno = static_cast<unsigned int>(tam_buffer);

   if (retorno > 0) {
      *buffer = new char[tam_buffer];
      if (fread (*buffer, 1, tam_buffer, fp) < tam_buffer)
         retorno = 0;
   }

   fclose (fp);
   return retorno;
}

bool
Mensagem::buffer_para_arquivo (const char *buffer, unsigned int tam,
                               const char *arquivo) 
{
   FILE *fp = fopen (arquivo, "w");
   if (!fp) 
      return false;

   bool retorno = true;
   if (fwrite (buffer, 1, tam, fp) < tam)
      retorno = false;

   fclose (fp);
   return retorno;
}

unsigned int
Mensagem::ls_para_buffer (const char *args, char **buffer) 
{
   if (!dir_local)
      return 0;

   // construcao do comando
   std::string a("ls ");
   a.append((*dir_local).normaliza_args_ls(args));
#ifdef LS_COMO_LL
   a.append(" -l --color ");
#endif
   a.append (" >.ls_tmp");

   if (system (a.c_str ()) == -1)
      return 0;

   // carrega .ls_tmp no buffer, remove .ls_tmp e retorna tamanho do buffer
   unsigned int tam_buffer = arquivo_para_buffer (".ls_tmp", buffer);
   if (system ("if [ -e .ls_tmp ]; then rm .ls_tmp; fi") == -1)
      return 0;

   return tam_buffer;
}

bool
Mensagem::pacotes_para_arquivo()
{
   char 
      *buffer = new char[pacotes.size() * PACOTE_MAX_TAM_DADOS],
      *nome_arq_buffer = new char[PACOTE_MAX_TAM_DADOS * 5];
   unsigned int offset = 0, tam_dados = 0;
   bool tem_f = false, tem_d = false, tem_z = false, f_tam_ja_foi = false;

   Pacote *p;

   long int i = static_cast<long int> (pacotes.size());
   while (i-- > 0) {
      p = pacotes.front();
      pacotes.pop_front();

      if ((*p).get_tipo() == F) {
         if (f_tam_ja_foi) {
            if ((*p).get_tamanho_dados() > 0) {
               memcpy(nome_arq_buffer, (*p).get_dados(), 
                      static_cast<unsigned int>((*p).get_tamanho_dados()));
               nome_arq_buffer[(*p).get_tamanho_dados()] = '\0';
               tem_f = true;
            }
         } else {
            f_tam_ja_foi = true;
         }
      } else
      if ((*p).get_tipo() == D) {
         if ((*p).get_tamanho_dados() > 0) {
            tam_dados = static_cast<unsigned int> ((*p).get_tamanho_dados());
            memcpy(buffer + offset, (*p).get_dados(), tam_dados);
            offset += (*p).get_tamanho_dados();
            tem_d = true;
         }
      } else
      if ((*p).get_tipo() == Z) {
         i = 0;
         tem_z = true;
      }
   }

   // resolve nome do arquivo
   int off, nome_arq_tam = static_cast<int> (strlen(nome_arq_buffer));
   for (off=nome_arq_tam; off>=0; off--)
      if (*(nome_arq_buffer+off) == '/') {
         nome_arq_buffer += off + 1;
         off = 0;
      }

   std::string nome_str;
   if (dir_local) {
      nome_str.assign((*dir_local).get_dir());
      if (nome_str.length() > 1)
         nome_str.append("/");
      nome_str.append(nome_arq_buffer);
   } else
      nome_str.assign(nome_arq_buffer);

   bool retorno = true;
   if (tem_f && tem_d && tem_z)
      retorno = buffer_para_arquivo(buffer, offset, nome_str.c_str());

   delete [] buffer;
   return retorno;
}

long
Mensagem::tamanho_arquivo (const char *arquivo)
{
   long tam_arq = 0;

   FILE *fp = fopen(arquivo, "r");
   if (fp) {
      if (fseek(fp, 0L, SEEK_END) == 0)
         tam_arq = ftell(fp);
      fclose(fp);
   }

   return tam_arq;
}

long
Mensagem::tamanho_arquivo ()
{
   long tam_arq = 0;

   if (pacotes.size() > 0) {
      Pacote *p = pacotes.front();
      if (static_cast<size_t>((*p).get_tamanho_dados()) >= sizeof(long)) {
         unsigned char *d = (*p).get_dados();
         tam_arq = (long) (d[3] * (1<<24) + d[2] * (1<<16) + d[1] * (1<<8) + d[0]);
      }
   }

   return tam_arq;
}

/* construtores/destrutor */
Mensagem::Mensagem(Diretorio* diretorio_local) 
{
   dir_local = diretorio_local;
}

Mensagem::~Mensagem() 
{
   while (!pacotes.empty())
      pacotes.pop_back();
}


/* interpretacao de comandos */
void 
Mensagem::set_comando(std::string linha) 
{
   linha_cmd = linha;
}

std::string 
Mensagem::get_comando() 
{
   return linha_cmd;
}

enum pacote_t
Mensagem::tipo_linha_comando() 
{
   /* get, put, cd, ls */
   enum pacote_t tipo = I;

   regex_t er;
   if (regcomp (&er, "^ls", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, linha_cmd.c_str(), 0, 0, 0)==0)
      tipo = L;
   else 
   if (regcomp (&er, "^cd", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, linha_cmd.c_str(), 0, 0, 0)==0)
      tipo = C;
   else 
   if (regcomp (&er, "^put", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, linha_cmd.c_str(), 0, 0, 0)==0)
      tipo = P;  
   else 
   if (regcomp (&er, "^get", REG_EXTENDED|REG_NOSUB)==0 && 
       regexec (&er, linha_cmd.c_str(), 0, 0, 0)==0)
      tipo = G;

   regfree (&er); 
   return tipo;
}

std::string
Mensagem::arg_comando() 
{
   std::string arg;
   if (!comando_valido()) {
      arg.assign("<comando invalido>");
      return arg;
   }

   char *cstr = new char[linha_cmd.length() + 1];
   strcpy (cstr, linha_cmd.c_str ());

   strtok_r (cstr, " ", &cstr);
   cstr[strlen(cstr)] = '\0';

   arg.assign(cstr);

   return arg;
}

bool
Mensagem::comando_valido () 
{
   return tipo_linha_comando() != I;
}

enum pacote_t
Mensagem::interpreta_mensagem ()
{
   // construir linha de comando e devolver o tipo do pacote
   std::list<Pacote *>::iterator i = pacotes.begin();

   enum pacote_t tp;
   switch ((**i).get_tipo()) {
   case 'L': tp = L; 
   break;
   case 'C': tp = C; 
   break;
   case 'G': tp = G; 
   break;
   case 'P': tp = P; 
   break;
   case 'J': tp = J;
   break;
   default: tp = I;
   break;
   }

   if (tp == I)
      return tp;

   switch (tp) {
   case C:
      linha_cmd.assign("cd");
      if ((**i).get_tamanho_dados() > 0)
         linha_cmd.append(" ");
   break;
   case L:
      linha_cmd.assign("ls");
      if ((**i).get_tamanho_dados() > 0)
         linha_cmd.append(" ");
   break;
   case G:
      linha_cmd.assign("get");
      if ((**i).get_tamanho_dados() > 0)
         linha_cmd.append(" ");
   break;
   case P:
      linha_cmd.assign("put");
      if ((**i).get_tamanho_dados() > 0)
         linha_cmd.append(" ");
   break;
   case Y:
   case N:
   case E:
   case X:
   case D:
   case F:
   case Z:
   case S:
   case J:
   case I:
   default:
   break;
   }

   char *lcmd = new char[PACOTE_MAX_TAM_DADOS];
   while ((**i).get_tipo() != 'S' && i!=pacotes.end()) {
      if ((**i).get_tamanho_dados() > 0) {
         memcpy (lcmd, (**i).get_dados(), (**i).get_tamanho_dados());
         lcmd[(**i).get_tamanho_dados()] = '\0';
         linha_cmd.append(lcmd);
      }
      i++;
   }

   delete [] lcmd;
   return tp;
}

/* manipulacao de grupos de pacotes */
bool
Mensagem::prepara_pergunta (int estagio) 
{ 
   char *vazio = const_cast<char*>((*(new std::string (""))).c_str());
   long tam_arq = 0;
   switch (tipo_linha_comando ()) {
   // comando [C]d.
   // estagio 1: (C<...>, ..., C<...>, S)
   case C: {
      if (estagio == 0) {
         segmenta_buffer_em_pacotes (C, (arg_comando()).c_str(), 
            (arg_comando()).length());
         pacotes.push_back (new Pacote (S, vazio, 0));
      }
   } break;
   // comando [L]s.
   // estagio 1: (L<...>, ..., L<...>, S)
   case L: {
      if (estagio == 0) {
         segmenta_buffer_em_pacotes (L, (arg_comando()).c_str(),
            (arg_comando()).length());
         pacotes.push_back (new Pacote (S, vazio, 0));
      }
   } break;
   // comando [P]ut.
   // estagio 1: (P<...>, ..., P<...>, S)
   // estagio 2: (F<...>, ..., F<...>, D<...>, ..., D<...>, Z, S)
   case P: {
      if (estagio == 0) {
         segmenta_buffer_em_pacotes (P, (arg_comando()).c_str(),
            (arg_comando()).length());
         pacotes.push_back (new Pacote (S, vazio, 0));
      } else if (estagio == 1) {
         /*printf("tentando calcular o tam do arquivo: %s\n", (arg_comando()).c_str());
         tam_arq = tamanho_arquivo((arg_comando()).c_str());
         pacotes.push_back(new Pacote (F, &tam_arq, sizeof(long)));
         segmenta_buffer_em_pacotes (F, (arg_comando()).c_str(),
            (arg_comando()).length());

         char *buffer;
         unsigned int t = 
            arquivo_para_buffer ((arg_comando()).c_str(), &buffer);
         segmenta_buffer_em_pacotes (D, buffer, t);
         printf("check-point, ok!\n");

         pacotes.push_back (new Pacote (Z, vazio, 0));
         pacotes.push_back (new Pacote (S, vazio, 0));*/

         std::string destino_arquivo((*dir_local).formata_caminho(arg_comando()));
         tam_arq = tamanho_arquivo(destino_arquivo.c_str());
         pacotes.push_back(new Pacote (F, &tam_arq, sizeof(long)));
         segmenta_buffer_em_pacotes (F, (arg_comando()).c_str(),
            (arg_comando()).length());

         char *buffer;
         unsigned int t = 
            arquivo_para_buffer (destino_arquivo.c_str(), &buffer);
         segmenta_buffer_em_pacotes (D, buffer, t);

         pacotes.push_back (new Pacote (Z, vazio, 0));
         pacotes.push_back (new Pacote (S, vazio, 0));
      }
   } break;
   // comando [G]et.
   // estagio 1: (G<...>, ..., G<...>, S)
   case G: {
      if (estagio == 0) {
         segmenta_buffer_em_pacotes (G, (arg_comando()).c_str(),
            (arg_comando()).length());
         pacotes.push_back (new Pacote (S, vazio, 0));
      }
   } break;
   // comando invalido.
   case J:
   case Y:
   case N:
   case E:
   case X:
   case D:
   case F:
   case Z:
   case S:
   case I:
   default:
   break;
   }

   return true;
}

bool
Mensagem::prepara_resposta () 
{
   if (!dir_local) {
      fprintf (stdout, "[prepara_resposta()]: diretorio local invalido.\n");
      return false;
   }

   char *buffer, *vazio = new char, errmsg[50], nomedir[1024]; vazio[0] = '\0';
   unsigned int tbuffer;
   FILE *fp;
   std::string str_err;
   long tam_arq = 0;
   switch (tipo_linha_comando ()) {
   // comando [C]d.
   // (S) ou (E<...>, ..., E<...>, S)
   case C: {
      // () ou (E<...>, ..., E<...>)
      switch ((*dir_local).altera_dir(arg_comando())) {
      case 0:
      break;
      case ENOENT: {
         strcpy(errmsg, "Diretorio nao encontrado.\n");
         pacotes.push_back(new Pacote (E, errmsg, static_cast<unsigned char>(strlen(errmsg))));
      } break;
      case ENOTDIR: {
         strcpy(errmsg, "Isso nao eh um diretorio.\n");
         pacotes.push_back(new Pacote (E, errmsg, static_cast<unsigned char>(strlen(errmsg))));
      } break;
      case EPERM: {
         strcpy(errmsg, "Permissao negada.\n");
         pacotes.push_back(new Pacote (E, errmsg, static_cast<unsigned char>(strlen(errmsg))));
      } break;
      default: {
         pacotes.push_back(new Pacote (E, vazio, 0));
      } break;
      }

      // (..., S)
      strcpy(nomedir, ((*dir_local).get_dir()).c_str()); nomedir[251] = '\0';
      pacotes.push_back (new Pacote (S, nomedir, static_cast<unsigned char>(strlen(nomedir))));
   } break;
   // comando [L]s.
   // (X<...>, ..., X<...>, S) ou (E<...>, ..., E<...>, S)
   case L: {
       // () ou (E<...>, ..., E<...>)
      tbuffer = ls_para_buffer ((arg_comando()).c_str(), &buffer);
      segmenta_buffer_em_pacotes (X, buffer, tbuffer);

      // (..., S)
      pacotes.push_back (new Pacote (S, vazio, 0));
   } break;
   // comando [P]ut.
   // (S) ou (E<...>, ..., E<...>, S)
   case P: {
      void();

      // (..., S)
      pacotes.push_back (new Pacote (S, vazio, 0));
   } break;
   // comando [G]et.
   // (E<...>, ..., E<...>, S) ou 
   // (F<...>, ..., F<...>, D<...>, ..., D<...>, Z, S)
   case G: {
      // (F<...>, ..., F<...>, D<...>, ..., D<...>, Z)
      std::string destino_arquivo((*dir_local).formata_caminho(arg_comando()));
      tam_arq = tamanho_arquivo(destino_arquivo.c_str());
      if ((fp=fopen(destino_arquivo.c_str(), "r"))) {
         pacotes.push_back (new Pacote (F, &tam_arq, sizeof(long)));
         segmenta_buffer_em_pacotes (F, (arg_comando()).c_str(),
                                     (arg_comando()).length());
         tbuffer = arquivo_para_buffer (destino_arquivo.c_str(), &buffer);
         segmenta_buffer_em_pacotes (D, buffer, tbuffer);
         pacotes.push_back (new Pacote (Z, vazio, 0));
      }
      // (E<...>, ..., E<...>)
      else {
         if (errno == 2)
            str_err = "Arquivo inexistente.\n";
         else
         if (errno == 13)
            str_err = "Sem permissao de acesso.\n";
         else
            str_err = "Impossivel abrir o arquivo.\n";

         segmenta_buffer_em_pacotes (E, str_err.c_str(), str_err.length() + 1);
      }

      // (..., S)
      pacotes.push_back (new Pacote (S, vazio, 0));
   } break;
   // comando invalido.
   case J:
   case Y:
   case N:
   case E:
   case X:
   case D:
   case F:
   case Z:
   case S:
   case I:
   default:
   break;
   }

   return true;
}

void
Mensagem::enfila_pacote (Pacote *p) 
{
   pacotes.push_back (p);
}

Pacote*
Mensagem::desenfila_pacote () 
{
   if (pacotes.size() == 0)
      return NULL;

   Pacote *p = pacotes.back();
   pacotes.pop_front();
   return p;
}

/* metodos de proposito geral */
bool
Mensagem::processa_resposta_no_cliente(enum pacote_t t)
{
   switch (t) {
   case L: {
      std::string s = remonta_pacotes_como_string();
      if (s.length() > 0)
         std::cout << s << std::endl;
   } break;
   case C: {
      std::string s = remonta_pacotes_como_string();
      if (s.length() > 0)
         std::cout << s;

      write(1, "diretorio remoto corrente: ", 27);
      write(1, (*(pacotes.back())).get_dados(), (*(pacotes.back())).get_tamanho_dados());
      printf("\n");
   } break;
   case G: {
      std::string s = remonta_pacotes_como_string();
      if (s.length() > 0)
         std::cout << s << std::endl;
      else {
         std::cout << "gerando arquivo... ";
         bool ret = pacotes_para_arquivo();
         std::cout << "OK" << std::endl;
         return ret;
      }
   } break;
   case P: {
      std::string s = remonta_pacotes_como_string();
      if (s.length() > 0)
         std::cout << s << std::endl;
   } break;
   case Y:
   case N:
   case E:
   case X:
   case D:
   case F:
   case Z:
   case S:
   case J:
   case I:
   default:
   break;
   }

   return true;
}

bool
Mensagem::processa_resposta_no_servidor(enum pacote_t t)
{
   if (t == P) {
      return pacotes_para_arquivo();
   } else {
      fprintf (stdout, "nao sei onde processar a resposta.\n");
   }

   return true;
}

bool
Mensagem::mensagem_de_erro () 
{
   std::list<Pacote*>::iterator i;
   for (i=pacotes.begin(); i!=pacotes.end(); i++) 
      if ((**i).get_tipo() == 'E')
         return true;

   return false;
}

std::string
Mensagem::remonta_pacotes_como_string() 
{
   char *buffer = new char[PACOTE_MAX_TAM_DADOS + 1];
   std::string s;
   std::list<Pacote*>::iterator i;
   for (i=pacotes.begin(); i!=pacotes.end(); i++) 
      if ((**i).get_tipo() == 'E' || (**i).get_tipo() == 'X') {
         if ((**i).get_tamanho_dados() > 0) {
            memcpy(buffer, (**i).get_dados(), (**i).get_tamanho_dados());
            buffer[(**i).get_tamanho_dados()] = '\0';
            s.append(buffer);
         }
      }

   return s;
}

/* debug */
void
Mensagem::imprime_pacotes () 
{
   std::list<Pacote*>::iterator i;
   unsigned int num;
   for (i=pacotes.begin(), num=1; i!=pacotes.end(); i++, num++) {
      printf ("#%u: ", static_cast<unsigned int>((**i).get_sequencia()));
      (**i).print();
   }
}

