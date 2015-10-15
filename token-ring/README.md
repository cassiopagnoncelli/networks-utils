Compila��o e uso
----------------

   `make all', ou simplesmente `make', para compilar e gerar o execut�vel.
   `make clean' para remover o execut�vel e os arquivos objetos.

   O execut�vel, `rede', tem dois argumentos: host e num_maquina, nessa ordem.

   host: nome da rede onde a m�quina para a qual as mensagens ser�o enviadas
         est� localizada. (e.g., localhost, macalan).

   num_maquina: n�mero �nico que identifica a m�quina corrente na token ring,
                endere�ada de 0 a 3, em ordem cronol�gica de entrada no anel.

   V�rias funcionalidades est�o dispon�veis para uso e s�o configur�veis a
   partir do Makefile, onde tamb�m s�o melhor explicadas. Basicamente elas
   permitem maiores informa��es sobre o bast�o, estat�sticas da passagem do
   bast�o e controle da forma como a entrada de dados do usu�rio � tratada.


Erros e bugs conhecidos
-----------------------

   N�o h� bugs/erros conhecidos.


Dificuldades encontradas e as respectivas solu��es implementadas
----------------------------------------------------------------

   i) problema: uma vez que uma mensagem de texto entra na rede, ela deve
      ser retirada do anel pela m�quina que a colocou. Como saber quem
      enviou a mensagem para poder posteriormente a retirar do anel?

      solu��o mais elegante: o primeiro byte da mensagem indica qual
      m�quina do anel colocou a mensagem na rede. Uma vez que a mensagem
      cuja origem � a mesma da m�quina que a recebeu, esta � descartada
      sem ser passada para a m�quina adjacente.

  ii) problema: como identificar se uma nova mensagem � bast�o ou mensagem 
      de texto?

      solu��o: o segundo byte da mensagem indica o tipo da mensagem, se � 
      bast�o ('B') ou mensagem de texto ('T').

   Assim, definimos um protocolo:
   +--------+------+-----------+-------------------+
   | origem | tipo | tam_dados |      dados        |
   +--------+------+-----------+-------------------+
   com tamanho m�nimo 3 e tamanho m�ximo 258 (255 bytes endere�ados por 
   tam_dados e 3 bytes oriundos dos campos `origem', `tipo' e `tam_dados').

 iii) problema: usar stdin, stdout, recvfrom e sendto de forma concorrente.
 
      solu��o: usamos pthreads e temos basicamente 3 threads: 
        (1) recebe dados do teclado; 
	(2) recebe dados da rede e os processa;
	(3) administra o tempo do bast�o, recolocando-o quando necess�rio.

  iv) problema: calcular o tempo do timeout do bast�o em fun��o do n�mero
      de m�quinas na rede.

      solu��o: Encontrar o menor tempo maior do que o tempo de transmiss�o
      de uma mensagem de texto por todo o anel. Isto �, o menor tempo maior
      maior do que NUM_MAQUINAS_NA_REDE * 2s. Como as medidas s�o est�o 
      sendo feitas em segundos, TIMEOUT = NUM_MAQUINAS_NA_REDE * 2s + 1s.


Observa��es
-----------

   i) Portas n�o s�o especificadas pelo usu�rio, elas s�o calculadas em fun��o
      do identificador da m�quina no anel. Mais concretamente, a m�quina i
      (i=0,1,2,3) recebe dados na porta 2515 + i.

  ii) Cada m�quina tem um limite de 255 caracteres por mensagem, que � o 
      tamanho do buffer. Sempre que um usu�rio entra com uma mensagem maior
      do que essa, a mensagem � quebrada em mensagens menores com tamanho de 
      255 caracteres, sendo tratadas como mensagens independentes e, portanto,
      cada parte � enviada 2s ap�s que um bast�o � recebido. Um bom motivo para
      usar essa abordagem � a seguinte:

      *) mensagens maiores (ou, potencialmente, muito maiores) levam mais tempo
      para ser colocadas na rede (e.g. um arquivo maior do que 50 MB), com isso
      temos um tempo total de 2s somados com a leitura e tempo do arquivo do
      exemplo. Supondo que ocorrer� um timeout da m�quina adjance que aguarda 
      por uma mensagem, ela rep�e um novo bast�o no anel e, a m�quina anterior
      tamb�m envia o bast�o assim que ela colocar toda a mensagem no anel e,
      portanto, temos ter�amos um problema de ter dois bast�es na rede. 


Poss�veis otimiza��es
---------------------

   i) M�dulo de cria��o autom�tica da rede. A porta 2514 pode ser reservada
      para prover informa��es para m�quinas que chegam na rede, como em
      qual porta ela deve receber dados e em qual porta ela deve enviar
      mensagens dentro do host especificado. Assim, o �nico par�metro do
      programa poderia ser o nome da rede para o qual as mensagens ser�o
      enviadas.

