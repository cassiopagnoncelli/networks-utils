#------------------------------ compilador -------------------------------------
CC	:= gcc
#-------------- regras que não geram executáveis de mesmo nome -----------------
.PHONY: default clean

default: all

all: servidor cliente

clean:
	@$(RM) -v cliente servidor *.o

#------------ regras que geram executáveis de mesmo nome -----------------------
cliente: cliente.c msg.o

servidor: servidor.c msg.o calc.o

msg.o: msg.c

calc.o: calc.c
