CC = gcc
CFLAGS = -Wall -g -std=c99 $(DEBUG_FLAGS) # gerar "warnings" detalhados e infos de depuração
 
obj = $(prog).o ppos.o queue.o
prog = testep4

# regra default (primeira regra)
all:	$(prog)

# Se DEBUG=1, define a flag -DDEBUG para o compilador
debug:	CFLAGS += -DDEBUG
debug:	all

# regras de ligacao
$(prog):	$(obj)
 
# regras de compilação
$(prog).o:	$(prog).c
ppos.o:	ppos_core.c
	$(CC) $(CFLAGS) -c -o ppos.o ppos_core.c 

queue.o: queue.c
	$(CC) $(CFLAGS) -c -o queue.o queue.c

# remove arquivos temporários
clean:
	-rm -f $(obj) *~
 
# remove tudo o que não for o código-fonte
purge: clean
	-rm -f $(prog) 
