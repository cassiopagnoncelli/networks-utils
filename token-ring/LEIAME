Compilação e uso
----------------

   `make all', ou simplesmente `make', para compilar e gerar o executável.
   `make clean' para remover o executável e os arquivos objetos.

   O executável, `rede', tem dois argumentos: host e num_maquina, nessa ordem.

   host: nome da rede onde a máquina para a qual as mensagens serão enviadas
         está localizada. (e.g., localhost, macalan).

   num_maquina: número único que identifica a máquina corrente na token ring,
                endereçada de 0 a 3, em ordem cronológica de entrada no anel.

   Várias funcionalidades estão disponíveis para uso e são configuráveis a
   partir do Makefile, onde também são melhor explicadas. Basicamente elas
   permitem maiores informações sobre o bastão, estatísticas da passagem do
   bastão e controle da forma como a entrada de dados do usuário é tratada.


Erros e bugs conhecidos
-----------------------

   Não há bugs/erros conhecidos.


Dificuldades encontradas e as respectivas soluções implementadas
----------------------------------------------------------------

   i) problema: uma vez que uma mensagem de texto entra na rede, ela deve
      ser retirada do anel pela máquina que a colocou. Como saber quem
      enviou a mensagem para poder posteriormente a retirar do anel?

      solução mais elegante: o primeiro byte da mensagem indica qual
      máquina do anel colocou a mensagem na rede. Uma vez que a mensagem
      cuja origem é a mesma da máquina que a recebeu, esta é descartada
      sem ser passada para a máquina adjacente.

  ii) problema: como identificar se uma nova mensagem é bastão ou mensagem 
      de texto?

      solução: o segundo byte da mensagem indica o tipo da mensagem, se é 
      bastão ('B') ou mensagem de texto ('T').

   Assim, definimos um protocolo:
   +--------+------+-----------+-------------------+
   | origem | tipo | tam_dados |      dados        |
   +--------+------+-----------+-------------------+
   com tamanho mínimo 3 e tamanho máximo 258 (255 bytes endereçados por 
   tam_dados e 3 bytes oriundos dos campos `origem', `tipo' e `tam_dados').

 iii) problema: usar stdin, stdout, recvfrom e sendto de forma concorrente.
 
      solução: usamos pthreads e temos basicamente 3 threads: 
        (1) recebe dados do teclado; 
	(2) recebe dados da rede e os processa;
	(3) administra o tempo do bastão, recolocando-o quando necessário.

  iv) problema: calcular o tempo do timeout do bastão em função do número
      de máquinas na rede.

      solução: Encontrar o menor tempo maior do que o tempo de transmissão
      de uma mensagem de texto por todo o anel. Isto é, o menor tempo maior
      maior do que NUM_MAQUINAS_NA_REDE * 2s. Como as medidas são estão 
      sendo feitas em segundos, TIMEOUT = NUM_MAQUINAS_NA_REDE * 2s + 1s.


Observações
-----------

   i) Portas não são especificadas pelo usuário, elas são calculadas em função
      do identificador da máquina no anel. Mais concretamente, a máquina i
      (i=0,1,2,3) recebe dados na porta 2515 + i.

  ii) Cada máquina tem um limite de 255 caracteres por mensagem, que é o 
      tamanho do buffer. Sempre que um usuário entra com uma mensagem maior
      do que essa, a mensagem é quebrada em mensagens menores com tamanho de 
      255 caracteres, sendo tratadas como mensagens independentes e, portanto,
      cada parte é enviada 2s após que um bastão é recebido. Um bom motivo para
      usar essa abordagem é a seguinte:

      *) mensagens maiores (ou, potencialmente, muito maiores) levam mais tempo
      para ser colocadas na rede (e.g. um arquivo maior do que 50 MB), com isso
      temos um tempo total de 2s somados com a leitura e tempo do arquivo do
      exemplo. Supondo que ocorrerá um timeout da máquina adjance que aguarda 
      por uma mensagem, ela repõe um novo bastão no anel e, a máquina anterior
      também envia o bastão assim que ela colocar toda a mensagem no anel e,
      portanto, temos teríamos um problema de ter dois bastões na rede. 


Possíveis otimizações
---------------------

   i) Módulo de criação automática da rede. A porta 2514 pode ser reservada
      para prover informações para máquinas que chegam na rede, como em
      qual porta ela deve receber dados e em qual porta ela deve enviar
      mensagens dentro do host especificado. Assim, o único parâmetro do
      programa poderia ser o nome da rede para o qual as mensagens serão
      enviadas.

