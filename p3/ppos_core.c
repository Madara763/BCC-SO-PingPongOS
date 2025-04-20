// PingPongOS - PingPong Operating System
// Davi Garcia Lazzarin
// 19/04/25

#include "ppos.h"


//------------------------------------------------------------------------------

//Formato da estrutura que define um Task Control Block (TCB) declarado no .h
/*
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				                // identificador da tarefa
  ucontext_t context ;			      // contexto armazenado da tarefa
  short status ;			            // pronta, rodando, suspensa, ...
  int vg_id ;		                  // ID da pilha da tarefa no Valgrind
  // ... (outros campos serão adicionados mais tarde)
} task_t ;
*/

//------------------------------------------------------------------------------

// Defines =====================================================================
#define STATUS_MAIN 0
#define STATUS_INI 0
#define STACK_TAM 128*1024

//status das tarefas
#define PRONTA 0
#define TERMINADA 1
#define SUSPENSA 2

// Variaveis Globais============================================================

task_t contextoMain, *contextoAnterior = NULL, *contextoAtual = NULL, despachante_ptr;
int ID_Global=0;              //Contem o valor do prox id de task que sera criado
queue_t *fila_tasks = NULL;           //Cria a fila de tarefas
queue_t *fila_tasks_prontas = NULL;   //Cria a fila de tarefas

// Funções Internas=============================================================

// Inicializa uma task, recebe ponteiros *prev, *next, e status
// Retorna o id da tarefa, retorna um valor negativo em caso de erro
// O cria_stack indica se precisa alocar a pilha para o novo contexto, 0 se nao precisa 1 se precisa

int task_cria(task_t *task, task_t *prev, task_t *next, short status, short cria_stack){
  if(!task)
    return -1;

  task->next = next;
  task->prev = prev;
  task->id = ID_Global;
  task->status = status;

  getcontext(&(task->context)); 

  if(cria_stack){
    char *stack = malloc(STACK_TAM);
    if(stack){
      task->context.uc_stack.ss_size = STACK_TAM;
      task->context.uc_stack.ss_sp = stack;
      task->context.uc_stack.ss_flags = 0;
      task->context.uc_link = 0 ;
    }
    else{
      perror ("Erro na criação da pilha: ") ;
      exit (1) ;
    }
  }

  // registra a pilha da tarefa no Valgrind
  task->vg_id = VALGRIND_STACK_REGISTER (task->context.uc_stack.ss_sp, task->context.uc_stack.ss_sp + STACK_TAM);
  ID_Global ++;

  return task->id;
}

//Libera a memoria da task atual
//Retira o regitro do valgrind
void free_stack_val(){

  // libera a pilha da tarefa
  free (contextoAtual->context.uc_stack.ss_sp) ;

  // dezfaz o registro da pilha no Valgrind
  VALGRIND_STACK_DEREGISTER (contextoAtual->vg_id);
}

//Atualiza o ponteiro de proxima tarefa, coloca a atual no final da fila
//Retorna um ponteiro para a tarefa a ser executada no momento
task_t* scheduler(){
  return (task_t*)fila_tasks;
}

//Funcao e chamada sempre que uma tarefa eh encerrada
//Coloca a proxima tarefa em execucao
//Se nao houver mais tarefas para tomar cpu, encerra o programa chmando o tes_exit(0)
void despachante(void *ptr){
  // retira o dispatcher da fila de prontas, para evitar que ele ative a si próprio
  queue_remove (&fila_tasks, (queue_t*) &despachante_ptr);

  //debug
  #ifdef DEBUG
  if(fila_tasks)
    printf("DEBUG: (despachante) primeiro na fila_task->id = %d\n", ((task_t*) fila_tasks)->id);
  else
    printf("DEBUG: (despachante) primeiro na fila_task = NULL\n");
  #endif

  //queue_size(fila_tasks);
  //exit(0);

  task_t* proxima;
  while(queue_size(fila_tasks)){
    //debug
    #ifdef DEBUG
    printf("DEBUG: (despachante) queue_size(fila_tasks) = %d\n",queue_size(fila_tasks));
    #endif 
    proxima = scheduler();
    
    if(proxima){
      task_switch(proxima);
      
      switch (contextoAtual->status)
      {
      case PRONTA:  //Remove da fila de prontas e coloca para exec dnv
        //debug
        #ifdef DEBUG
        printf("DEBUG: (despachante) switch: PRONTA\n");
        #endif 
        queue_remove(&fila_tasks, (queue_t*)contextoAtual);
        if(contextoAtual != &despachante_ptr){
          queue_append(&fila_tasks, (queue_t*)contextoAtual);
        }
          break;
      case TERMINADA: //Remove a tarefa da fila de prontas e ja era
        //debug
        #ifdef DEBUG
        printf("DEBUG: (despachante) switch: TERMINADA\n");
        #endif 
        queue_remove(&fila_tasks, (queue_t*)contextoAtual);
        break;
      case SUSPENSA:
        break;

      default:
        break;
      }

    }//if proxima    
  }//while existir tarefas para serer executadas

  //debug
  #ifdef DEBUG
  printf("DEBUG: (despachante) Encerrando o despachante\n");
  #endif 
  free_stack_val();
  exit(0);

};

// Funções Gerais ==============================================================

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init (){

  //inicia o contexto da main
  task_cria(&contextoMain, NULL, NULL, STATUS_MAIN, 0);

  //Aponta o contexto atual para o main
  contextoAtual = &contextoMain;

  //debug
  #ifdef DEBUG
  printf("DEBUG: (ppos_init) id main = %d\n", contextoMain.id);
  printf("DEBUG: (ppos_init) endereco main = %p\n", &contextoMain);
  #endif

  //inicia o contexto do despachante
  task_init(&despachante_ptr, despachante, NULL);

  // desativa o buffer da saida padrao (stdout), usado pela função printf 
  setvbuf (stdout, 0, _IONBF, 0) ;

}

// gerencia de tarefas =========================================================

// Inicializa uma nova tarefa. Retorna um ID> 0 ou erro.
// Recebe:
// descritor da nova tarefa
// funcao corpo da tarefa
// argumentos para a tarefa
int task_init (task_t *task, void  (*start_func)(void *),	void   *arg) {

  if(!task || !start_func )
    return -1;
  
  task_cria(task, NULL, NULL, PRONTA, 1);

  makecontext(&(task->context), (void *) start_func , 1, arg );
  
  //debug
  #ifdef DEBUG
  printf("DEBUG: (task_init) id criado = %d\n", task->id);
  #endif
  
  //Adiciona a task criada na fila
  queue_append(&fila_tasks, (queue_t*)task);
  
  //queue_size((queue_t*)&fila_tasks);

  return task->id;
}			

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id () {
  return contextoAtual->id;
}

// Termina a tarefa corrente com um status de encerramento
void task_exit (int exit_code) {
  //debug
  #ifdef DEBUG
  printf("DEBUG: (task_exit) id atual = %d\n", contextoAtual->id);
  printf("DEBUG: (task_exit) id despachante = %d\n", despachante_ptr.id);
  #endif

  //Se outra task chamar o task_exit, retorna para o despachante
  //Se o despachante chamar, encerra o programa
  if(contextoAtual != &despachante_ptr){
    free_stack_val();
    contextoAtual->status = TERMINADA;
  }

  //Volta para o despachante
  task_yield();
}

// alterna a execução para a tarefa indicada
// Retorna 0 se der certo e um valor negativo em caso de erro
int task_switch (task_t *task) {
  if(!task)
    return -1;

  //debug
  #ifdef DEBUG
  printf("DEBUG: (task_swith) id atual = %d\n", contextoAtual->id);
  printf("DEBUG: (task_swith) id novo = %d\n", task->id);
  #endif

  contextoAnterior = contextoAtual;
  contextoAtual = task;

  //debug
  #ifdef DEBUG
  printf("DEBUG: (task_swith) Trocando\n");
  #endif
  swapcontext(&(contextoAnterior->context), &(task->context));

  return 0;
}

// a tarefa atual libera o processador para outra tarefa
void task_yield (){
  
  //debug
  #ifdef DEBUG
  printf("DEBUG: (task_yield) id atual = %d\n", contextoAtual->id);
  #endif

  if(contextoAtual->status == TERMINADA)
    queue_remove(&fila_tasks, (queue_t*)contextoAtual);
  else{
    contextoAtual->status = PRONTA;
    queue_remove(&fila_tasks, (queue_t*)contextoAtual);
    queue_append(&fila_tasks, (queue_t*)contextoAtual);    
  }

  //debug
  #ifdef DEBUG
  printf("DEBUG: (task_yield) Chamando o task_switch\n");
  #endif
  task_switch(&despachante_ptr);
}

