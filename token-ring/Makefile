CC	= gcc

# flags
#
# MAQUINAS_NA_REDE: n�mero de m�quinas no anel para a rede funcionar.
# info_bastao: mostra informa��es de quando o bast�o � recolocado no anel
# conta_bastao: mostra quantas vezes o bast�o passou pela m�quina nos �ltimos 10s              
# msg_formatada: na chegada de uma nova mensagem, a mesma � impressa da forma
#                junto com os campos do protocolo mostrando a origem da mensagem.
# quebra_msg: mensagens maiores que 255 caracteres s�o quebradas em outras msgs.
#
CFLAGS 	= -g -pthread -DMAQUINAS_NA_REDE=4 -Dinfo_bastao

.PHONY: all clean

all: tr

clean: 
	find . -name "*.o" -exec rm {} \;
	if [ -e rede_anel ]; then rm rede_anel; fi

mensagem.o: mensagem.c

tokenring.o: tokenring.c mensagem.o

tr: maquina.c mensagem.o tokenring.o
	$(CC) $(CFLAGS) -o rede_anel maquina.c mensagem.o tokenring.o
