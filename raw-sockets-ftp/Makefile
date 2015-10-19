#------------------------------------ flags ----------------------------------
#----     compilador    ----
CXX      := @g++

#---- flags do programa ----
# SLEEP_MILI: tempo em milisegundos para atualizacao da barra de progresso
#             de trocas de arquivos. (valor padrao: 2000)
#
# ESPERA_ENVIO_US: tempo de espera em microsegundos entre dois pacotes serem
#             enviados. (valor padrao: 0)
#
# TROCA_MSGS: metodo para controle de fluxo, disponiveis:
#      PARA_E_ESPERA
#      JANELAS_DESLIZANTES
#
# LS_COMO_LL: os comandos 'ls' e 'lls' sao executados com a opcao "-l --color".
#
CXXFLAGS := \
	-g \
	-ansi \
	-pedantic \
	-pthread \
	-Wall \
	-Wextra \
	-Wformat \
	-Wdeprecated \
	-Wdeprecated-declarations \
	-Wdiv-by-zero \
	-Winline \
	-Woverflow \
	-Wnon-virtual-dtor \
	-Woverlength-strings \
	-Wlogical-op \
	-Wpragmas -Wshadow \
	-Wredundant-decls \
	-Wstrict-aliasing \
	-Wstrict-overflow \
	-Wstrict-null-sentinel \
	-Wundef \
	-Wunknown-pragmas \
	-Wswitch \
	-Wswitch-default \
	-Wswitch-enum \
	-Wcast-align \
	-Wcast-qual -Wconversion \
	\
	-DLS_COMO_LL \
	-DTROCA_MSGS=PARA_E_ESPERA

#---------------------------- nomes dos executaveis --------------------------
CLIENTE_EXEC  := cliente
SERVIDOR_EXEC := servidor

#---------------------- regras que nao nomeiam executaveis -------------------
.PHONY: all clean

#-------------------------------- regras gerais ------------------------------
all: cliente_app servidor_app
	@find . -maxdepth 1 -name "*.o" -exec rm -r {} \;

clean:
	if test -e $(CLIENTE_EXEC); then rm -r $(CLIENTE_EXEC); fi
	if test -e $(SERVIDOR_EXEC); then rm -r $(SERVIDOR_EXEC); fi
	find . -maxdepth 1 -name "*.o" -exec rm -r {} \;

#-------------------------------- executaveis --------------------------------
cliente_app: ftp_cliente.cc cliente.o socket.o mensagem.o pacote.o \
	     diretorio.o janela_deslizante.o
	$(CXX) $(CXXFLAGS) -o $(CLIENTE_EXEC) ftp_cliente.cc cliente.o socket.o mensagem.o pacote.o diretorio.o janela_deslizante.o

servidor_app: ftp_servidor.cc servidor.o socket.o mensagem.o pacote.o \
	      diretorio.o janela_deslizante.o
	$(CXX) $(CXXFLAGS) -o $(SERVIDOR_EXEC) ftp_servidor.cc servidor.o socket.o mensagem.o pacote.o diretorio.o janela_deslizante.o

#---------------------------------- objetos ----------------------------------
pacote.o: pacote.cc

diretorio.o: diretorio.cc

janela_deslizante.o: janela_deslizante.cc pacote.o mensagem.o socket.o

mensagem.o: mensagem.cc pacote.o diretorio.o

socket.o: socket.cc pacote.o

cliente.o: cliente.cc socket.o mensagem.o pacote.o janela_deslizante.o \
	diretorio.o

servidor.o: servidor.cc socket.o mensagem.o pacote.o janela_deslizante.o \
	diretorio.o

