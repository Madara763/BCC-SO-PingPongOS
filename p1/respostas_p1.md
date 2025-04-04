# Respostas das [questoes](https://wiki.inf.ufpr.br/maziero/doku.php?id=so:trocas_de_contexto)

## Questao 1:

Para explicar as funcoes e importante definirmos o que e o *ucontext_t*: Basicamente e uma struct que complementa o *contexto* salvo por um *mcontext_t*, contendo informacoes extras como um conjunto de *sinais* definidos por *sigset_t*, a pilha do processo/thread definido por *stack_t* e um um ponteiro para um *ucontext_t* que assumira caso esse termine.

### getcontext(*ucontext_t)
Salva os dados do processo atual no local apontado.

### setcontext(*ucontext_t)
Restaura os valores do contexto recebido no contexto atual.

### swapcontext(*ucontext_t, *ucontext_t)
Faz um troca do contexto atual e um recebido pelo segundo argumento, salva o contexto atual no endereco do primeiro argumento e restaura o contexto do segundo.

### makecontext(*ucontext_t, ...)
Essa funcao recebe um ponteiro para um *ucontext_t* e uma lista de parametros, permite alterar a proxima funcao a ser chamada ao restaurar o contexto.

## Questao 2
Ja respondemos essa questao no inicio da questao 1. :)

## Questao 3
Vamos iniciar a explicacao da funcao main().

``` getcontext (&ContextPing) ; ``` 
Salva o contexto atual na variavel global *ucontext_t ContextPing* .

```
char *stack ;
...
stack = malloc (STACKSIZE) ;
```
Apesar de nao manipular um ucontext_t e importante para entendermos mais a frente. 
Aloca uma regiao de memoria de *STACKSIZE* bytes na heap, stack aponta para o primeiro byte dessa regiao (por isso stack e um char*, para apontar apenas para o primeiro byte).

```
if (stack)
{
  ContextPing.uc_stack.ss_sp = stack ;
  ContextPing.uc_stack.ss_size = STACKSIZE ;
  ContextPing.uc_stack.ss_flags = 0 ;
  ContextPing.uc_link = 0 ;
}
else
{
  perror ("Erro na criação da pilha: ") ;
  exit (1) ;
}
```
Esse bloco verifica se a alocacao em *stack* deu certo.
Se deu certo, altera o ponteiro da pilha salvo em ContextPing apontando para a regiao apontada por *stack*, tambem altera algumas flags, como tamanho da pilha e status de uso, tambem altera o valor do uc_link como 0, indicando que ao encerrar esse contexto ele nao sera substituido por outro.
Se a pilha nao foi alocada corretamente, encerra o programa.

```
makecontext (&ContextPing, (void*)(*BodyPing), 1, "    Ping") ;
```
Altera o contexto salvo anteriormente em *ContextPing*, passa a funcao *void BodyPing*, com a 'string' "    Ping".

```
getcontext (&ContextPong) ;

stack = malloc (STACKSIZE) ;
if (stack)
{
  ContextPong.uc_stack.ss_sp = stack ;
  ContextPong.uc_stack.ss_size = STACKSIZE ;
  ContextPong.uc_stack.ss_flags = 0 ;
  ContextPong.uc_link = 0 ;
}
else
{
  perror ("Erro na criação da pilha: ") ;
  exit (1) ;
}

makecontext (&ContextPong, (void*)(*BodyPong), 1, "        Pong") ;
```
Faz a mesma coisa que ja foi vista acima, mas agora com outra variavel de contexto, a *ContextPong*.

```
swapcontext (&ContextMain, &ContextPing) ;
```
Nessa linha o contexto atual e salvo em *ContextMain*, e o contexto em *ContextPing* e restaurado, e como o seus ponto de incio foi setado para ser a funcao BodyPing, iniciando o contexto com a execucao da funcao BodyPing.

```
void BodyPing (void * arg)
{
  int i ;

  printf ("%s: inicio\n", (char *) arg) ;

  for (i=0; i<4; i++)
  {
    printf ("%s: %d\n", (char *) arg, i) ;
    swapcontext (&ContextPing, &ContextPong) ;
  }
  printf ("%s: fim\n", (char *) arg) ;

  swapcontext (&ContextPing, &ContextMain) ;
}
```
Aqui inicia uma sequencia de trocas de contexto dentro do *for* entre o *ContextPing* e o *ContextPong*, onde o *ContextPing* imprime na tela, troca o contexto com o *ContextPong* que imprime na tela, e isso se repete 4 vezes, ate o *ContextPing* imprimir "fim ..." e trocar o contexto com o *ContextMain* novamente.

```
swapcontext (&ContextMain, &ContextPong) ;
```
Troca com o contexto *ContextPong* que ja executou o *for* em BodyPong 4 vezes, com o i igual a 3, e sai do *for*, imprime "fim ..." e troa de contexto com *ContextMain* que  imprime "main: fim\n" e encerra o processo

