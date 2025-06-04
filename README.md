# PingPongOS
## Subprojetos
link com a espacificacao: https://wiki.inf.ufpr.br/maziero/doku.php?id=so:projetos_desenvolvidos

### [Biblioteca de Filas](https://wiki.inf.ufpr.br/maziero/doku.php?id=so:biblioteca_de_filas) (p0)
Consiste na implementacao das funcoes declaradas no arquivo queue.h.
No geral, sao funcoes de manipulacao de uma fila circular duplamente encadeada generica, com o objetivo de usar casting com filas especificas. 
#### Arquivos:
```
  queue.h 
  queue.c 
  testafila.c 
  testafila 
```
### [Trocas de Contexto](https://wiki.inf.ufpr.br/maziero/doku.php?id=so:trocas_de_contexto) (p1)
Consiste em estudar o código em ``` contexts.c ``` e responder algumas perguntas(especificado no enunciado).
Respostas em ```respostas_pq.md```.

### [Gestão de Tarefas](https://wiki.inf.ufpr.br/maziero/doku.php?id=so:gestao_de_tarefas) (p2)
Construir as funções básicas de gestão de tarefas usando as funções de troca de contexto vistas no projeto anterior. 
#### Arquivos:
```
ppos_data.h
ppos.h
ppos_core.c
```
### [Despachante de tarefas](https://wiki.inf.ufpr.br/maziero/doku.php?id=so:dispatcher) (p3)
Consiste em implementar funcoes e estruturas para gerenciar trocas de contexto e status das terefas.
#### Arquivos:
```
queue.h
queue.c
ppos_data.h
ppos.h
ppos_core.c
```
### [Escalonamento por prioridades](https://wiki.inf.ufpr.br/maziero/doku.php?id=so:escalonador_por_prioridades) (p4)
Consiste em implentar o escalonamnto de prioridades das tarefas.
#### Arquivos:
```
queue.h
queue.c
ppos_data.h
ppos.h
ppos_core.c
```
### [Preempção e compartilhamento de tempo](https://wiki.inf.ufpr.br/maziero/doku.php?id=so:preempcao_por_tempo) (p5)
Consiste em implementar limites de tempo para as tarefas usartem a CPU, anteriormente o sistema era cooperativo, agora uma tarefa pode ser retirada da CPU antes de terminar sua execucao.
#### Arquivos:
```
queue.h
queue.c
ppos_data.h
ppos.h
ppos_core.c
```
### [Contabilização de tarefas](https://wiki.inf.ufpr.br/maziero/doku.php?id=so:contabilizacao) (p6)
Agora vamos contabilizar o tempo de CPU que cada tarefa usou.
#### Arquivos:
```
queue.h
queue.c
ppos_data.h
ppos.h
ppos_core.c
```

### [Tarefas Suspensas](https://wiki.inf.ufpr.br/maziero/doku.php?id=so:tarefas_suspensas) (p8)  
Implementamos a suspensao de tarefas e tarefas podem esparar o termino da execucao de outra.  
#### Arquivos:
```
queue.h
queue.c
ppos_data.h
ppos.h
ppos_core.c
```

### [Tarefas Dormindo](https://wiki.inf.ufpr.br/maziero/doku.php?id=so:tarefas_dormindo) (p9)  
Parecido com o anterior, mas agora a tarefas espera por um certo tempo e nao pelo termino de outra task.  
#### Arquivos:  
```
queue.h
queue.c
ppos_data.h
ppos.h
ppos_core.c
```
