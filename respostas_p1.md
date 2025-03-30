# Respostas das [questoes].(https://wiki.inf.ufpr.br/maziero/doku.php?id=so:trocas_de_contexto).

## Questao 1:

Para explicar as funcoes e importante definirmos o que e o *ucontext_t*: Basicamente e uma struct que complementa o *contexto* salvo por um *mcontext_t*, contendo informacoes extras como um conjunto de *sinais* definidos por *sigset_t*, a pilha do processo/thread definido por *stack_t* e um um ponteiro para um *ucontext_t* que assumira caso esse termine.

### getcontext(*ucontext_t)
Salva os dados do processo atual no local apontado.

### setcontext(*ucontext_t)
Restaura os valores do contexto recebido no contexto atual.

### swapcontext(*ucontext_t, *ucontext_t)
Faz um troca do contexto atual e um recebido pelo segundo argumento, salva o contexto atual no endereco do primeiro argumento e restaura o contexto do segundo.

### makecontext(*ucontext_t, ...)



