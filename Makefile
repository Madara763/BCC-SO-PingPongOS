CFLAGS = -Wall -g -std=c99  # gerar "warnings" detalhados e infos de depuração
 
obj = testefila.o queue.o

# regra default (primeira regra)
all:	testefila

# regras de ligacao
testefila:	$(obj)
 
# regras de compilação
testefila.o:	testefila.c
queue.o:	queue.c

# remove arquivos temporários
clean:
	-rm -f $(obj) *~
 
# remove tudo o que não for o código-fonte
purge: clean
	-rm -f testafila 