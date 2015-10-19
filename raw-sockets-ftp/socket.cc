#include "socket.h"
#include <cstdio>
#include <cerrno>             /* contém `errno' */
#include <ctime>              /* contém time() */
#include <features.h>
#include <signal.h>
#include <unistd.h>           /* contém usleep() */
#include <cstdlib>            /* contém malloc() */
#include <cstring>            /* contém memcpy() */
#include <unistd.h>           /* contém close() */
#include <net/ethernet.h>     /* contém ETH_P_ALL */
#include <net/if.h>           /* contém struct ifreq */
#include <sys/socket.h>       /* contém socket(2) */
#include <sys/ioctl.h>        /* contém ioctl() */
#include <bits/ioctls.h>      /* constantes para ioctl() */
#include <netpacket/packet.h> /* contém struct sockaddr_ll */
#include <arpa/inet.h>        /* contém htons() */
#include <list>
#include <pthread.h>

/* tempo de espera para enviar um pacote. */
#ifndef ESPERA_ENVIO_US
#define ESPERA_ENVIO_US 0
#endif

#define minimo(A, B) A < B ? A : B

// estatisticas dos pacotes
unsigned int 
   pacotes_ok,          /* pacotes consistentes */
   pacotes_corrompidos; /* erros de recebimento + pacotes corrompidos. */

/*
   Socket.
*/
Socket::Socket()
{
   pacotes_ok = 0;
   pacotes_corrompidos = 0;

   timeout_exp[0]  = 1;
   timeout_exp[1]  = 2;
   timeout_exp[2]  = 4;
   timeout_exp[3]  = 8;
   timeout_exp[4]  = 16;
   timeout_exp[5]  = 32;
   timeout_exp[6]  = 64;
   timeout_exp[7]  = 128;
   timeout_exp[8]  = 256;
   timeout_exp[9]  = 512;
   timeout_exp[10] = 1024;
   timeout_exp[11] = 1024;
   timeout_exp[12] = 1024;
   timeout_exp[13] = 1024;
   timeout_exp[14] = 1024;
   timeout_exp[15] = 1024;

   houve_erro = true;
   raw_socket = -1;
}

Socket::~Socket() 
{
   // encerra socket
   if (!erro() && close (raw_socket) != 0)
      printf ("erro: [close()] socket nao encerrado.\n");
   else
      printf ("Socket encerrado.\n");

   // estatisticas dos pacotes recebidos
   if (pacotes_ok + pacotes_corrompidos > 0)
      fprintf(stdout, "-- estatisticas dos pacotes recebidos --\
          \nsem erros: %u\ncorrompidos: %u\n",
          pacotes_ok, pacotes_corrompidos);
   else
      fprintf(stdout, "-- estatisticas dos pacotes recebidos--\
          \nsem erros: --\ncorrompidos: --\n");
}

/* propriedades */
int 
Socket::get_fd() 
{
   return raw_socket;
}

bool 
Socket::erro() 
{
   return houve_erro;
}

bool
Socket::cria_socket (std::string& disp) 
{
   char *dispositivo = new char[disp.length()];
   strcpy (dispositivo, disp.c_str());

   if (!dispositivo) 
      return false;

   /* 
     Cria um socket (endpoint) para comunicação. Argumentos (ver packet(7)):

     PF_PACKET: protocolo de acesso direto a interface de rede (opera em nível
     abaixo de tcp/ip).

     SOCK_RAW:  usaremos a semântica de comunicação RAW, conforme 
     especificado. Dessa forma, os pacotes são passados de socket a socket sem
     serem alterados (SOCK_RAW não é compatível com AF_INET/SOCK_PACKET em 
     linux 2.0).

     htons(ETH_P_ALL): como queremos receber TODOS os tipos de pacote (isso
     inclui pacotes de protocolos IEEE8.02.15.4, Apple Talk sobre PPP, 
     DEC DDCMP, etc) usamos ETH_P_ALL (ver <linux/if_ethernet.h>, 90-111). Por
     fim, htons(3) converte o unsigned short int para "network byte order".
   */
   raw_socket = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL));
   if (raw_socket == -1) {
      printf ("erro: [socket()] uid > 0 (tente modo root).\n");
      return false;
   }

   /* 
     A seguir, buscaremos o índice da interface de rede e o colocaremos em
     ifr_ifindex da estrutura ifreq por meio da função ioctl()

     struct ifreq
     {
     char ifr_name[IFNAMSIZ]; // interface name
     union 
     {
     struct sockaddr ifr_addr;
     struct sockaddr ifr_dstaddr;
     struct sockaddr ifr_broadaddr;
     struct sockaddr ifr_netmask;
     struct sockaddr ifr_hwaddr;
     short           ifr_flags;
     int             ifr_ifindex;
     int             ifr_metric;
     int             ifr_mtu;
     struct ifmap    ifr_map;
     char            ifr_slave[IFNAMSIZ];
     char            ifr_newname[IFNAMSIZ];
     char            *ifr_data;
     };
     }

     (o acesso aos membros dessa união interna é visível a ifreq (exempli 
     gratia, um dos membros da união é ifreq.ifr_mtu)). A chamada de ioctl()
     requer UID 0 ou um usuário que tenha privilégio de administrador de rede.
     Como ioctl() é uma função bastante genérica para configuração de 
     dispositivos de rede, o parâmetro que permite ioctl() realizar a busca
     do índice desta interface de rede é SIOCGIFINDEX (ver netdevice(7), seção
     Ioctls, subseção SIOCGIFINDEX). Seu último argumento, também genérico
     (void *), é a estrutura a qual ele vai preencher, no caso ifreq.
   */
   struct ifreq ifr;
   memset (&ifr, 0, sizeof (struct ifreq));
   strncpy (static_cast<char*> (ifr.ifr_name), dispositivo, strlen (dispositivo));
   if (ioctl (raw_socket, SIOCGIFINDEX, &ifr) == -1) {
     printf ("erro: [ioctl()] interface de rede inexistente.\n");
     return false;
   }

   /* 
     Vamos pegar pacotes dessa interface usando bind() especificando o 
     endereço pela estrutura sockaddr_ll

     struct sockaddr_ll 
     {
     unsigned short int  sll_family;
     unsigned short int  sll_protocol;
     int                 sll_ifindex;
     unsigned short int  sll_hatype;
     unsigned short char sll_pkttype;
     unsigned short char sll_halen;
     unsigned short char sll_addr[8];
     }

     de forma a associar o socket a essa interface. Para esse propósito é 
     necessário preencher apenas sll_protocol e sll_ifindex, conforme 
     packet(7), linhas 51-52, além, obviamente, de sll_family. Do contrário,
     provoca erro em bind(2).
   */
   struct sockaddr_ll sall;
   memset (&sall, 0, sizeof (struct sockaddr_ll));
   sall.sll_family = AF_PACKET;
   sall.sll_ifindex = static_cast<int> (ifr.ifr_ifru.ifru_ivalue);
   sall.sll_protocol = static_cast<unsigned short int> (htons (ETH_P_ALL));
   if (bind (raw_socket, reinterpret_cast<struct sockaddr*>(&sall), sizeof (sall)) == -1) {
      printf ("erro: [bind()] socket nao associado a %s.\n", dispositivo);
      return false;
   }

   /* 
     Existem dois modos de configuração da camada física, um é
     o modo multicasting e outro o modo promíscuo. Para configuração,
     há uma chamada de setsockopt(2) que espera uma estrutura packet_mreq com
     os parâmetros dessa configuração.

     Usaremos o modo ``promíscuo'' (ver netdevice(7), Socket Options), que 
     permite receber TODOS os pacotes que chegam na placa. Desse modo, nenhum 
     pacote será barrado. Trataremos os dados por meio do protocolo e com 
     verificação de erros, mais adiante, pois os sockets criados com protocolo
     AF_PACKET não fazem verificação de erro.

     struct packet_mreq
     {  
     int            mr_ifindex;    // interface index
     unsigned short int mr_type;   // action
     unsigned short int mr_alen;   // address length
     unsigned char  mr_address[8]; // physical layer address
     }

     Opcões:
     PACKET_ADD_MEMBERSHIP,
     PACKET_DROP_MEMBERSHIP,
     PACKET_RECV_MEMBERSHIP,
     PACKET_RX_RING,
     PACKET_STATISTICS.

     Segundo packet(7), seção Socket Options, o modo ``promíscuo'' funciona
     com SOL_PACKET/PACKET_ADD_MEMBERSHIP, que promove a associação do socket
     com a interface de rede, bem como SOL_PACKET/PACKET_DROP_MEMBERSHIP para
     desfazer essa associação.

     Ainda sobre a estrutura packet_mreq, mr_address e mr_alen são necessáios
     apenas no modo multicast, que não é o caso. Portanto, no modo 
     ``promíscuo'' apenas são necessários mr_ifindex e mr_type.
   */
   struct packet_mreq pmr;
   memset (&pmr, 0, sizeof (struct packet_mreq));
   pmr.mr_ifindex = static_cast<short int> (ifr.ifr_ifru.ifru_ivalue);
   pmr.mr_type = PACKET_MR_PROMISC;
#ifndef SOL_PACKET     /* Necessário para glibc <= 2.1 */
#define SOL_PACKET 263 /* veja em packet(7), seção BUGS. */
#endif
   if (setsockopt (raw_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &pmr,
                   sizeof (struct packet_mreq)) == -1) {
      printf ("erro: [setsockopt()] impossivel setar modo promiscuo.\n");
      return false;
   }

   printf ("Socket ~ %s\n", dispositivo);
   houve_erro = false;
   return true;
}

void
Socket::incr_pacotes_ok()
{
   pacotes_ok++;
}

void
Socket::incr_pacotes_corrompidos()
{
   pacotes_corrompidos++;
}


/*
   I/O socket.
 */
bool
Socket::enviar(Pacote &p)
{
   usleep(ESPERA_ENVIO_US);

   ssize_t bytes_enviados;
   if ((bytes_enviados=send (raw_socket, p.to_buffer(), p.get_tamanho_pacote(), 0)) < 1) 
      return false;

   if (static_cast<unsigned int>(bytes_enviados) != p.get_tamanho_pacote())
      return false;

   //printf("send: ");
   //p.print();

   return true;
}

enum resp_t
Socket::receber(Pacote &p, unsigned int tentativa)
{
   unsigned char *b = new unsigned char [258]; // sentinela
   if (!b) return ERRO;

   limpa_buffer();

   struct timeval tout;
   fd_set rfd;

   srand((unsigned int) time(NULL));

   tout.tv_sec  = 1 + (rand() % timeout_exp[minimo(tentativa, 15)]);
   tout.tv_usec = 0;

   FD_ZERO(&rfd);
   FD_SET(raw_socket, &rfd);

   if (select(1 + raw_socket, &rfd, NULL, NULL, &tout)) {
      if (recv(raw_socket, b, 258, 0) < 0)
         return ERRO;

      p.from_buffer(b);
      if (p.corrompido()) {
         pacotes_corrompidos++;
         return CORROMPIDO;
      }

      //printf("recv: ");
      //p.print();

      pacotes_ok++;
      return OK;
   } else
      return TIMEOUT;
}

void
Socket::limpa_buffer()
{
   struct timeval tout;
   fd_set rfd;

   tout.tv_sec  = 0;
   tout.tv_usec = 0;

   FD_ZERO(&rfd);
   FD_SET(raw_socket, &rfd);

   // numero de descritores de arquivos
   int num_fds = select(1 + raw_socket, &rfd, NULL, NULL, &tout); 
   if (num_fds > 0)
      recv(raw_socket, NULL, num_fds, 0); // limpa o buffer
}
