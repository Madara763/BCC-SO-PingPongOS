// PingPongOS - PingPong Operating System
// Davi Garcia Lazzarin
// 02/04/25

#include"ppos.h"
#include"ppos_data.h"

#include <stdio.h>
//------------------------------------------------------------------------------
//Formato da estrutura que define um Task Control Block (TCB) declarado no .h
/*
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  // ... (outros campos serão adicionados mais tarde)
} task_t ;
*/
//------------------------------------------------------------------------------
// Defines =====================================================================
#define STATUS_MAIN 0
#define STACK_TAM 1024

// Variaveis Globais============================================================

task_t contextoMain, contextoAnterior;
int ID_Global=0; //contem o valor do prox id de task que sera criado


// Funções Gerais ==============================================================

// Inicializa uma task, recebe ponteiros *prev, *next, e status
// Retorna o id da tarefa, retorna um valor negativo em caso de erro
// O cria_stack indica se precisa alocar a pilha para o novo contexto, 0 se nao precisa 1 se precisa

int task_cria(task_t *task, task_t *prev, task_t *next, short status, short cria_stack){
  if(!task)
    return -1;

  task->next = next;
  task->prev = prev;
  task->id = ID_Global;
  getcontext(&(task->context)); 
  task->status = status;

  if(cria_stack){
    char *stack = maloc(STACK_TAM);
    if(stack){
      task->context.uc_stack.ss_size = STACK_TAM;
      task->context.uc_stack.ss_sp = stack;
      task->context.uc_stack.ss_flags = 0;
    }
    else{
      perror ("Erro na criação da pilha: ") ;
      exit (1) ;
    }
  }

  ID_Global ++;

  return task->id;
}

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init (){
  
  //inicia o contexto da main
  task_cria(&contextoMain, NULL, NULL, STATUS_MAIN, 0);
  
  // desativa o buffer da saida padrao (stdout), usado pela função printf 
  setvbuf (stdout, 0, _IONBF, 0) ;

}

// gerência de tarefas =========================================================

// Inicializa uma nova tarefa. Retorna um ID> 0 ou erro.
// Recebe:
// descritor da nova tarefa
// funcao corpo da tarefa
// argumentos para a tarefa
int task_init (task_t *task, void  (*start_func)(void *),	void   *arg) {

  if(!task || !start_func )
    return -1;
  




}			

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id () ;

// Termina a tarefa corrente com um status de encerramento
void task_exit (int exit_code) ;

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) ;

