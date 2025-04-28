// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include <stdio.h>      // mensagens de erro
#include <stdlib.h>     // biblioteca contem o malloc
#include <valgrind/valgrind.h> //Biblioteca para uso do vg_id do valgrind
#include <signal.h>     // usada para gerenciar os tratadores de sinal
#include <sys/time.h>   // usada para gerenciar os quantum e interrupcoes

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				                // identificador da tarefa
  ucontext_t context ;			      // contexto armazenado da tarefa
  short status ;			            // pronta, rodando, suspensa, ...
  int vg_id ;		                  // ID da pilha da tarefa no Valgrind
  char* stack;                    // Ponteiro para a pilha alocada
  int prioridade;                 // Prioridade estatica da tarefa 
  int nice;                       // Valor do Nice(que deve estar entre -20 e +20)
  int tipo;                       // Tipo de tarefa, pode ser de SISTEMA (0) ou USUARIO (1)
  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif

