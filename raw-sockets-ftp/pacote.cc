#include "pacote.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>

Pacote::Pacote() 
{
   set_marcador ();
   set_tamanho_dados (static_cast<unsigned char> (0));
   set_sequencia (static_cast<unsigned char>(0));
   set_tipo (I);
   set_paridade (static_cast<unsigned char>(0));
}

Pacote::Pacote (enum pacote_t t, void *d, unsigned char tam_dados, 
                unsigned char seq) 
{
   set_marcador ();
   set_tamanho_dados (tam_dados);
   set_sequencia (seq);
   set_tipo (t);
   set_dados (static_cast<unsigned char*>(d), tam_dados);
   set_paridade (gera_paridade ());
}

Pacote::Pacote (Pacote &pac) 
{
   set_marcador (pac.get_marcador ());
   set_tamanho_dados (static_cast<unsigned char>(pac.get_tamanho_dados ()));
   set_sequencia (pac.get_sequencia ());
   set_tipo (pac.get_tipo ());
   set_dados (pac.get_dados (), get_tamanho_dados ());
   set_paridade (pac.get_paridade ());
}

Pacote::Pacote (const void *buffer) 
{
   from_buffer (buffer);
}

Pacote::~Pacote () 
{
   void();
}

/* metodos p/ paridade */
unsigned char 
Pacote::gera_paridade () 
{
   unsigned char res = 0;
   unsigned int j, soma;
   int i;
   for (i=7; i>=0; i--) {
      res = static_cast<unsigned char>(res << 1);

      soma = 0;
      for (j=0; j< static_cast<unsigned int>(get_tamanho_dados ()); j++)
         // i-esimo bit sendo 1 implica incremento da soma
         // de forma a obter a soma (e consequentemente a 
         // paridade) do alinhamento vertical (ou, em cascata) 
         // dos bytes do campo <dados> e, posteriormente, do 
         // campo <tipo>
         if ((dados[j] & (1 << i)) >> i == 1) 
            soma++;

      if ((tipo & (1 << i)) >> i == 1) 
         soma++;

      res = static_cast<unsigned char>(res|static_cast<unsigned char>(soma%2));
   }

   return res;
}

bool 
Pacote::checa_paridade() 
{
   return gera_paridade() == paridade;
}

bool 
Pacote::corrompido() 
{
   return gera_paridade() != paridade;
}

/* metodos p/ buffer */
void *
Pacote::to_buffer() 
{
   unsigned char *p = new unsigned char [get_tamanho_pacote()];
   unsigned char temp, *b = p;
   if (!memcpy(p  , &(temp=get_marcador()) , 1)) return NULL;
   if (!memcpy(++p, &(temp=get_tamanho() ) , 1)) return NULL;
   if (!memcpy(++p, &(temp=get_sequencia()), 1)) return NULL;
   if (!memcpy(++p, &(temp=get_tipo())     , 1)) return NULL;
   ++p;
   if (get_tamanho_dados() > 0 && !memcpy(p, &dados, get_tamanho_dados())) return NULL;
   if (!memcpy(p+get_tamanho_dados(), &(temp=get_paridade()) , 1) ) return NULL;

   /*fprintf (stdout, "to_buffer: m=%u t=%u seq=%u tipo=%c dados+paridade=%s paridade=%u\n",
          static_cast<unsigned int>(*b), 
          static_cast<unsigned int>(*(b+1)),
          static_cast<unsigned int>(*(b+2)),
          static_cast<char>(*(b+3)),
          reinterpret_cast<char*>(b+4),
          static_cast<unsigned int>(*(b+4+get_tamanho_dados())));*/

   return b;
}

void 
Pacote::from_buffer(const void *buffer) 
{
   unsigned char *p = static_cast<unsigned char*>(const_cast<void *>(buffer));
   set_marcador(*p); 
   p++;
   set_tamanho_dados(static_cast<unsigned char>(*p - 3)); 
   p++; // tam = |seq| + |tipo| + |dados| + |par| = |dados| + 3
   set_sequencia(*p); p++;
   set_tipo(*p); p++;
   set_dados(p, get_tamanho_dados());
   set_paridade(*(p+get_tamanho_dados()));
}

/* set */
void 
Pacote::set_marcador(unsigned char m) 
{
   marcador = m;
}

void
Pacote::set_tamanho_dados(unsigned char t_dados) 
{
   // "tamanho considera seq + tipo + dados + paridade".
   // tamanho = |seq| + |tipo| + |dados| + |paridade|
   //         = |dados| + 3
   //         = t_dados + 3
   tamanho = static_cast<unsigned char>(t_dados+3);
}

void
Pacote::set_sequencia(unsigned char seq) 
{
   sequencia = seq;
}

void 
Pacote::set_tipo(unsigned char t) 
{
   tipo = t;
}

void
Pacote::set_dados(unsigned char *d, unsigned int tam) 
{
   unsigned int i;
   for (i=0; i<tam; i++)
      dados[i] = d[i];
}

void
Pacote::set_paridade(unsigned char par) 
{
   paridade = par;
}

/* get */
unsigned char
Pacote::get_marcador() 
{
   return marcador;
}

unsigned char
Pacote::get_tamanho() 
{
   return tamanho;
}

unsigned char
Pacote::get_sequencia() 
{
   return sequencia;
}

unsigned char
Pacote::get_tipo() 
{
   return tipo;
}

unsigned char *
Pacote::get_dados() 
{
   return dados; // == &dados[0]
}

unsigned char 
Pacote::get_paridade() 
{
   return paridade;
}

/* propriedades do pacote */
unsigned int
Pacote::get_tamanho_pacote() 
{
   // "tamanho considera seq + tipo + dados + paridade"
   // |pacote| = |marcador| + |tam| + |seq| + |tipo| + |dados| + |par|
   //          = |marcador| + |tam| + tamanho
   //          = 2 + tamanho
   return 2 + tamanho;
}

unsigned int
Pacote::get_tamanho_dados() 
{
   // "tamanho considera seq + tipo + dados + par".
   // t_dados = tamanho - |seq| - |tipo| - |par|
   //         = tamanho - 3
   return static_cast<unsigned int> (tamanho - 3);
}

/* debug */
void
Pacote::print(bool nl, bool mostra_dados) 
{
   if (!checa_paridade()) printf ("{corrompido}: ");
   printf ("marca=%u ",   static_cast<unsigned int> (marcador));
   printf ("tam=%u " ,    static_cast<unsigned int> (tamanho));
   printf ("seq=%u " ,    static_cast<unsigned int> (sequencia));
   printf ("tipo=%c ",    static_cast<unsigned char>(tipo));
   printf ("dados[%u]={", static_cast<unsigned int> (get_tamanho_dados()));

   if (mostra_dados) {
      unsigned int i;
      for (i=0; i<get_tamanho_dados(); i++)
         printf ("%c", dados[i]);
   } else 
   if (get_tamanho_dados() > 0) {
      printf("...");
   }

   printf ("} paridade=%u%c", static_cast<unsigned int>(paridade), 
           nl ? '\n' : '\0');
}

void 
Pacote::print_buffer(unsigned char *buffer) 
{
   unsigned char *b = buffer;
   unsigned char b_marcador, b_tam, b_seq, b_tipo;
   unsigned char *b_dados = new unsigned char [get_tamanho_dados()];
   //unsigned char *b_dados = (unsigned char *)malloc(sizeof(unsigned char) * get_tamanho_dados());
   unsigned char b_paridade;

   // copy
   memcpy(&b_marcador,   b, 1);
   memcpy(&b_tam,      ++b, 1);
   memcpy(&b_seq,      ++b, 1);
   memcpy(&b_tipo,     ++b, 1);
   memcpy(b_dados,     ++b, get_tamanho_dados());
   memcpy(&b_paridade, b+get_tamanho_dados(), 1);

   // print
   printf ("#marca=%u ",  static_cast<unsigned int> (b_marcador));
   printf ("tam=%u " ,    static_cast<unsigned int> (b_tam));
   printf ("seq=%u " ,    static_cast<unsigned int> (b_seq));
   printf ("tipo=%c ",    static_cast<unsigned char>(b_tipo));
   printf ("dados[%u]={", static_cast<unsigned int> (get_tamanho_dados()));

   unsigned int i;
   for (i=0; i<get_tamanho_dados(); i++)
      printf ("%c", *(b_dados+i));

   printf ("} paridade=%u\n", static_cast<unsigned int>(paridade));
}

void 
Pacote::print_byte(unsigned char byte) 
{
   int i;
   for (i=7; i>=0; i--)
      printf ("%u", (byte & (1 << i))>>i == 1);
   printf ("\n");
}
