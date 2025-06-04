// PingPongOS - PingPong Operating System
// Davi Garcia Lazzarin
// 19/04/25

#define _POSIX_C_SOURCE 200809L // Necessario para compilar com struct sigaction action;   

#include "ppos.h"


//------------------------------------------------------------------------------

/* Estrutura define alguns campos que usaremos para monitorar o tempo no PPOS
typedef struct ppos_tempo_t
{
  __uint64_t tmp_criacao;         // Tempo do sistema em que a tarefa foi criada
  __uint64_t tmp_exec;            // Tempo em MS desde a criacao da tarefa ate seu encerramento
  __uint64_t tmp_cpu;             // Tempo em MS que a tarefa ficou com a CPU
} ppos_tempo_t;

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  struct task_t *dependentes;     // fila de tarefas que estao suspensas aguaradando o termino dessa
  int id ;				                // identificador da tarefa
  ucontext_t context ;			      // contexto armazenado da tarefa
  short status ;			            // pronta, rodando, suspensa, ...
  int vg_id ;		                  // ID da pilha da tarefa no Valgrind
  char* stack;                    // Ponteiro para a pilha alocada
  int prioridade;                 // Prioridade estatica da tarefa 
  int nice;                       // Valor do Nice(que deve estar entre -20 e +20)
  int tipo;                       // Tipo de tarefa, pode ser de SISTEMA (0) ou USUARIO (1)
  int exit_code;                  // Codigo de encerramento da task (padrao eh 0)
  ppos_tempo_t tempo;             // Define campos para monitoramento de tempos da tarefa
  __uint64_t ativacoes;           // Quantas vezes a tarefa assumiu a CPU

  int acordar;                    // Tempo em que a tarefa tem que acordar (se 0 eh pq nao esta dormindo)
 
  // ... (outros campos serão adicionados mais tarde)
} task_t ;
*/
//------------------------------------------------------------------------------

// Defines =====================================================================
#define STATUS_MAIN 0
#define STATUS_INI 0
#define STACK_TAM 64*1024
#define QUANTUM_MAX 20

//status das tarefas
#define PRONTA 0
#define TERMINADA 1
#define SUSPENSA 2

//tipo de tarefa
#define SISTEMA 0
#define USUARIO 1

//Delta de prioridade (esse valor eh subtraido)
#define DELTA_PRIORIDADE 1

// Variaveis Globais============================================================

task_t contextoMain, *contextoAnterior = NULL, *contextoAtual = NULL, despachante_ptr;
int ID_Global=0;                         // Contem o valor do prox id de task que sera criado
queue_t *fila_tasks = NULL;              // Cria a fila de tarefas
queue_t *fila_tasks_terminadas = NULL;   // Cria a fila de tarefas finalizadas para liberar memoria
queue_t *fila_tasks_dormindo = NULL;     // Cria a fila de tarefas que estao dormindo por um periodo de tempo

struct sigaction action;                 // Definida para usar um tratador de sinal
struct itimerval timer;                  // Usada para lancar os sinais

__uint64_t clock;                        // Relogio do sistema
__uint64_t tmp_ini_task_atual;           // Recebe o momento em que a tarefa atual pegou a CPU 
int quantum;                             // Quantum da tarefa atual (vai de 20 a 0)


// Funções Internas=============================================================

// retorna o relógio atual (em milisegundos)
unsigned int systime (){
  return clock;
}

// define a prioridade dinamica de uma tarefa (ou a tarefa atual)
void task_setnice (task_t *task, int nice){
  if(nice <= 20 && nice >= -20){
    if(task){
      task->nice = nice;
    }
    else
      contextoAtual->nice = nice;
  }
}

// retorna a prioridade dinamica de uma tarefa (ou a tarefa atual)
int task_getnice (task_t *task) {
  if(task)
    return task->nice;

  return contextoAtual->nice;
}

// Inicializa uma task, recebe ponteiros *prev, *next, e status
// Retorna o id da tarefa, retorna um valor negativo em caso de erro
// O cria_stack indica se precisa alocar a pilha para o novo contexto, 0 se nao precisa 1 se precisa

int task_cria(task_t *task, task_t *prev, task_t *next, short status, short cria_stack){
  if(!task)
    return -1;

  task->next = next;
  task->prev = prev;
  task->dependentes = NULL;
  task->id = ID_Global;
  task->status = status;
  task->prioridade = 0;
  task->nice = 0;
  task->tipo = USUARIO;
  task->exit_code = 0;

  task->acordar = 0;

  task->tempo.tmp_criacao = systime();
  task->tempo.tmp_cpu = 0;
  task->tempo.tmp_exec = 0;

  task->ativacoes = 0;

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
void free_stack_val(task_t* task){
  //debug
  #ifdef DEBUG
  printf("DEBUG: (free_stack_val) Liberando a pilha\n");
  #endif 
  // libera a pilha da tarefa
  free(task->context.uc_stack.ss_sp) ;

  // dezfaz o registro da pilha no Valgrind
  VALGRIND_STACK_DEREGISTER (task->vg_id);
}

//Atualiza a prioridade com um valor delta
void task_atualiza_nice(task_t *task){

  //Percorre toda a fila 
  int tamfila = queue_size(fila_tasks);

  task_t *aux = (task_t*)fila_tasks;
  
  for(int i = 0; i < tamfila; i++){
    
    if( aux != task){ aux->nice -= DELTA_PRIORIDADE; }

    aux = aux->next;
  }
}

//Trata os sinais recebidos
void tratador_ticks(int signum){
  
  clock++; // Incrementa o relogio do sisteam em 1 MS

  if(contextoAtual->tipo != SISTEMA && signum == SIGALRM){
    if(quantum > 1)
      quantum --;
    else
      task_yield();
  }
  //else {} N faz nada pq o uma tarefa de sistema esta rodando ou o sinal foi outro

}

//Funcao atualiza o tempo que resta para as funcoes continuarem dormindo
//Se o tempo acabar , remove elas da fila de dormindo e insere na fica de prontas
void despertador(int ultima_alvorada){
  
  //debug
  #ifdef DEBUG
  //printf("DEBUG: (despertador) verificacao geral no tempo: %d \n", systime());
  #endif

  task_t *prox, *aux = (task_t*)fila_tasks_dormindo;

  //Percorre toda a fila 
  int tamfila = queue_size(fila_tasks_dormindo);
  for(int i = 0; i < tamfila; i++){
    prox = aux->next; 
    //Verifica se ja deu a hora de acordar
    if(aux->acordar <= systime()){
      //debug
      #ifdef DEBUG
      printf("DEBUG: (despertador) Acordando id = %d no tempo = %d, devia acordar em %d\n", aux->id, systime(), aux->acordar);
      #endif
      task_awake(aux, (task_t**) &fila_tasks_dormindo);
    }
    aux = prox;
  }
}

//Atualiza o ponteiro de proxima tarefa, coloca a atual no final da fila
//Retorna um ponteiro para a tarefa a ser executada no momento
//Se prioridade dinamica (nice) != 0 compara ela, prioriza a menor entre as dinamicas e estaticas
//Ao retornar a tarefa, ja  reseta a prioridade dinamica
task_t* scheduler(){
  //Aux aponta para o inicio da fila
  //Proxima aponta para a task com menor prioridade
  task_t *proxima = (task_t*)fila_tasks;
  task_t *aux = (task_t*)fila_tasks;

  //Percorre toda a fila buscando a menor prioridade
  int tamfila = queue_size(fila_tasks);

  for(int i = 0; i < tamfila; i++){
    if(task_getnice(aux)  < task_getnice(proxima)) //se nice for menor ou igual
      proxima = aux;    

    aux = aux->next;
  }
  
  //debug
  #ifdef DEBUG
  if(proxima)
    printf("DEBUG: (scheduler) Tarefa escolhida -> nice: %d  prio: %d\n", proxima->nice, proxima->prioridade);
  #endif

  if(proxima){
    task_atualiza_nice(proxima);
    task_setnice(proxima, task_getprio(proxima));
  }
  
  return (task_t*)proxima;
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
  
  int ultima_alvorada = systime();
  task_t* proxima;

  while(queue_size(fila_tasks) || queue_size(fila_tasks_dormindo) ){
    //debug
    #ifdef DEBUG
    //printf("DEBUG: (despachante) queue_size(fila_tasks) = %d\n",queue_size(fila_tasks));
    //printf("DEBUG: (despachante) queue_size(fila_tasks_dormindo) = %d\n",queue_size(fila_tasks_dormindo));
    #endif 

    despertador(ultima_alvorada);

    proxima = scheduler();
    
    if(proxima){
      quantum=QUANTUM_MAX;
      task_switch(proxima);
      
      switch (contextoAnterior->status)
      {
      case PRONTA:  //Remove da fila de prontas e coloca para exec dnv (task_yeld faz isso)
        //debug
        #ifdef DEBUG
        printf("DEBUG: (despachante) switch: PRONTA\n");
        #endif 
        
        break;
      case TERMINADA: //Remove a tarefa da fila de prontas e ja era
        //debug
        #ifdef DEBUG
        printf("DEBUG: (despachante) switch: TERMINADA\n");
        #endif 
        if(contextoAtual != &despachante_ptr)
          queue_remove(&fila_tasks, (queue_t*)contextoAnterior);
        
        break;
      case SUSPENSA:
        //debug
        #ifdef DEBUG
        printf("DEBUG: (despachante) task id (%d) esta suspensa\n", contextoAnterior->id);
        #endif
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

  // Libera a memoria das tasks terminadas
  while(queue_size(fila_tasks_terminadas)){
    //debug
    #ifdef DEBUG
    printf("DEBUG: (despachante) queue_size(fila_tasks_finalizadas) = %d\n",queue_size(fila_tasks));
    #endif 
    proxima = (task_t*)fila_tasks_terminadas;
    free_stack_val(proxima);
    queue_remove(&fila_tasks_terminadas, (queue_t*)proxima);
  }//while 

  //Precisa de um outro contexto para liberar a stack do despachante
  //free_stack_val(&despachante_ptr); 
  task_exit(0);
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

  //Ajusta a main e o despachante como tarefas do sistema
  contextoMain.tipo = SISTEMA;
  despachante_ptr.tipo = SISTEMA;

  // desativa o buffer da saida padrao (stdout), usado pela função printf 
  setvbuf (stdout, 0, _IONBF, 0) ;

  //Prepara o tratamento dos sinais
  action.sa_handler = tratador_ticks;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  if(sigaction(SIGALRM, &action, 0) < 0 ){
    perror("Erro em sigaction\n");
    exit(0);
  }

  //Inicia o sistema do temporizador
  timer.it_value.tv_usec = 1;        //atraso inicial em microsegundos
  timer.it_value.tv_sec = 0;          //atraso inicial em segundos
  timer.it_interval.tv_usec = 1000;   //tempo do intervalo em microsegundos
  timer.it_interval.tv_sec = 0;       //tempo do intervalo em segundos 

  clock = 0; // Inicia o relogio do sistema 

  //Arma o temporizador
  if(setitimer(ITIMER_REAL, &timer, 0) < 0){
    perror("Erro em settimer: ");
    exit(1);
  }
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
  printf("DEBUG: (task_exit) id atual = %d saindo com exit_code %d\n", contextoAtual->id, exit_code);
  #endif

  //Salva o tempo de vida da tarefa
  contextoAtual->tempo.tmp_exec = (systime() - contextoAtual->tempo.tmp_criacao);
  //Contabiliza o tempo de execucao
  contextoAtual->tempo.tmp_cpu += (systime() - tmp_ini_task_atual );

  //impressao para o p6
  printf("Task %d exit: execution time %ld ms, processor time %ld ms, %ld activations\n",
          contextoAtual->id, contextoAtual->tempo.tmp_exec,
          contextoAtual->tempo.tmp_cpu, contextoAtual->ativacoes);
  
  //Se outra task chamar o task_exit, retorna para o despachante
  //Se o despachante chamar, encerra o programa
  if(contextoAtual != &despachante_ptr)
    contextoAtual->status = TERMINADA;
  // else  
  //   exit(0);
  
  //Atualiza o exit_code da task
  contextoAtual->exit_code = exit_code;
  
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

  //Salva quando a tarefa atual pegou a CPU
  tmp_ini_task_atual = systime();
  
  task->ativacoes++;

  swapcontext(&(contextoAnterior->context), &(task->context));

  return 0;
}

// operações de escalonamento ==================================================

// Tarefa atual libera o processador para outra tarefa
// Reinsere ela nas filas
void task_yield (){
  
  //debug
  #ifdef DEBUG
  printf("DEBUG: (task_yield) id atual = %d\n", contextoAtual->id);
  #endif
  
  switch(contextoAtual->status)
  {
    case TERMINADA:
      queue_remove(&fila_tasks, (queue_t*)contextoAtual);

      queue_append(&fila_tasks_terminadas, (queue_t*)contextoAtual);  
      
      //debug
      #ifdef DEBUG
      printf("DEBUG: (task_yield) id atual = %d\n", contextoAtual->id);
      #endif

      //Acorda as que dependiam dela
      if(contextoAtual->dependentes){
        //debug
        #ifdef DEBUG
        printf("DEBUG: (task_yield) id atual = %d tem dependentes\n", contextoAtual->id);
        #endif

        task_t *aux, *ini = contextoAtual->dependentes;
        if(ini)
          aux = ini->next;

        while(aux != ini){
          task_awake(aux, &contextoAtual->dependentes);
        }
        task_awake(ini, &contextoAtual->dependentes);
      }
      break;

    default:
      //Contabiliza o tempo de execucao
      contextoAtual->tempo.tmp_cpu += (systime() - tmp_ini_task_atual );

      contextoAtual->status = PRONTA; 
      queue_remove(&fila_tasks, (queue_t*)contextoAtual);
      queue_append(&fila_tasks, (queue_t*)contextoAtual); 
      break;
  }

  //debug
  #ifdef DEBUG
  printf("DEBUG: (task_yield) Chamando o task_switch\n");
  #endif
  
  //Incrementa a ontagem de cpu do despachante
  task_switch(&despachante_ptr);
}

// define a prioridade estática e a dimanica de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio){
  if(prio <= 20 && prio >= -20){
    if(task){
      task->prioridade = prio;
      task->nice = prio;
    }
    else{
      contextoAtual->prioridade = prio;
      contextoAtual->nice = prio;
    }
  }
}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task) {
  if(task)
    return task->prioridade;

  return contextoAtual->prioridade;
}

// suspende a tarefa atual,
// transferindo-a da fila de prontas para a fila "queue"
void task_suspend (task_t **queue) {
  if (queue == NULL) {
    fprintf(stderr, "Tarefa %d está tentando esperar uma tarefa que não existe.\n", task_id());
    exit(1);
  }

  #ifdef DEBUG
  printf("DEBUG: (task_suspend) id atual = %d\n", contextoAtual->id);
  #endif

  queue_remove((queue_t**)&fila_tasks, (queue_t*)contextoAtual);  // Remove da fila de prontas
  contextoAtual->status = SUSPENSA;
  queue_append((queue_t**)queue, (queue_t*)contextoAtual);        // Adiciona na fila passada
  task_switch(&despachante_ptr);
}


// acorda a tarefa indicada,
// trasferindo-a da fila "queue" para a fila de prontas
void task_awake (task_t *task, task_t **queue) {
  //debug
  #ifdef DEBUG
  if (*queue)
    printf("DEBUG: (task_awake) acordando id = %d\n", task->id);
  else
    printf("DEBUG: (task_awake) acordando id = nulo\n");
  #endif 

  if(*queue){
    queue_remove((queue_t**)queue, (queue_t*)task); //Remove da fila 
    task->status = PRONTA;
    queue_append(&fila_tasks, (queue_t*)task);
  }
}

// a tarefa corrente aguarda o encerramento de outra task
int task_wait (task_t *task) {
  //debug
  #ifdef DEBUG
  printf("DEBUG: (task_wait) id atual = %d quer esperar id = %d\n", contextoAtual->id, task->id);
  #endif
  //Se a tarefa n existe
  if(!task) 
    return -1;

  if(task->status == TERMINADA){
    //debug
    #ifdef DEBUG
    printf("task_wait: id (%d) tentando esperar id(%d) que esta ENCERRADA.\n", task_id(),task->id);
    #endif
    return -1;
  }

  //Suspende a tarefa atual
  task_suspend(&(task->dependentes));

  return task->exit_code;
}

// suspende a tarefa corrente por t milissegundos
void task_sleep (int t) {
  //debug
  #ifdef DEBUG
  printf("DEBUG: (task_sleep) id atual = %d quer dormir = %d\n", contextoAtual->id, t);
  #endif

  if(t>0){
    contextoAtual->acordar = systime() + t;
    task_suspend((task_t**)&fila_tasks_dormindo);
  }
  else if(t!=0){
    fprintf(stderr, "Tentando dormir a task %d por tempo negativo\n", task_id());
  }

}