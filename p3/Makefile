CFLAGS = -Wall -g -std=c99 $(DEBUG_FLAGS) # gerar "warnings" detalhados e infos de depuração
 
obj = $(prog).o ppos.o
prog = testep3

# Se DEBUG=1, define a flag -DDEBUG para o compilador
DEBUG_FLAGS = -DDEBUG


# regra default (primeira regra)
all:	$(prog)

# regras de ligacao
$(prog):	$(obj)
 
# regras de compilação
$(prog).o:	$(prog).c
ppos.o:	ppos_core.c
	gcc $(CFLAGS) -c -o ppos.o ppos_core.c

# remove arquivos temporários
clean:
	-rm -f $(obj) *~
 
# remove tudo o que não for o código-fonte
purge: clean
	-rm -f $(prog) 