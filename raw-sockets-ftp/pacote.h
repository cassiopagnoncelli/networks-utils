#ifndef pacote_h
#define pacote_h

/* marca do protocolo */
#define MARCA 0x7e // 0x7e = 0111 1110

/* tipos de pacote */
enum 
pacote_t {
   /* controle de aceitacao de pacotes */
   Y = 'Y', // yes (acknowledged)
   N = 'N', // no (not acknowledged)
   E = 'E', // erro
   /* gerais */
   X = 'X', // mostra na tela
   D = 'D', // dados
   /* comandos */
   G = 'G', // get
   P = 'P', // put
   C = 'C', // cd
   L = 'L', // ls
   /* arquivos */
   F = 'F', // descritor de arquivos
   Z = 'Z', // final de transm. grupo de arquivos
   /* definicoes a mais */
   S = 'S', // proximo passo (Step)
   J = 'J', // mudanca do tamanho da janela deslizante
   /* argumento invalido */
   I = 0
};

/* 
   Pacote eh a divisao 

               +----------+-----+-----+------+-------------+----------+
   campos:     | marcador | tam | seq | tipo | dados (...) | paridade |
               +----------+-----+-----+------+-------------+----------+
   tam(bytes): |    1     |  1  |  1  |   1  |   0 a 252   |    1     |
               +----------+-----+-----+------+-------------+----------+
*/
#define PACOTE_MAX_TAM_DADOS 252 // tamanho maximo (bytes) do campo dados
class Pacote {
private:
/* campos do pacote */
     unsigned char marcador;
     unsigned char tamanho; // em bytes
     unsigned char sequencia;
     unsigned char tipo;
     unsigned char dados[PACOTE_MAX_TAM_DADOS];
     unsigned char paridade;
public:
/* construtores/destrutor */
     Pacote ();
     Pacote (enum pacote_t t, void *d, unsigned char tam_dados, 
             unsigned char seq = 0);
     Pacote (Pacote &pac);
     Pacote (const void *buffer);
    ~Pacote ();
/* paridade */
     unsigned char  gera_paridade (); 
     bool           checa_paridade ();
     bool           corrompido ();
/* buffer */
     void*          to_buffer (); 
     void*          to_buffer_rev (); //!*
     void           from_buffer (const void *buffer); 
     void           from_buffer_rev (const void *buffer); //!*
/* set */
     void           set_marcador (unsigned char m = MARCA);
     void           set_tamanho_dados (unsigned char t_dados = 0);
     void           set_sequencia (unsigned char seq);
     void           set_tipo (unsigned char t);
     void           set_dados (unsigned char *d, 
                               unsigned int tam = PACOTE_MAX_TAM_DADOS);
     void           set_paridade (unsigned char par);
/* get */
     unsigned char  get_marcador ();
     unsigned char  get_tamanho (); 
     unsigned char  get_sequencia ();
     unsigned char  get_tipo ();
     unsigned char* get_dados ();
     unsigned char  get_paridade ();
/* propriedades do pacote */
     unsigned int   get_tamanho_pacote (); // tamanho em bytes
     unsigned int   get_tamanho_dados ();  // tamanho em bytes
/* debug */
     void           print (bool nl=true, bool mostra_dados=true);
     void           print_buffer (unsigned char *buffer);
     void           print_byte (unsigned char byte);
};

#endif /* pacote.h */
