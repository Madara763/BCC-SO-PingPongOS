// PingPongOS - PingPong Operating System
// Davi Garcia Lazzarin
// 20/03/25

#include"queue.h"
#include<stdio.h>
//------------------------------------------------------------------------------
//Formato da struct declarado no .h
/*
typedef struct queue_t
{
  struct queue_t *prev ;  // aponta para o elemento anterior na fila
  struct queue_t *next ;  // aponta para o elemento seguinte na fila
} queue_t ;
*/

#ifdef DEBUG
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				                // identificador da tarefa
} task_t ;
#endif
//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size (queue_t *queue){
   
  if(!queue)
    return 0; 

  queue_t *ini = queue ;       // aponta elemento que sera tratado como inicial 
  queue_t *aux = queue->next;  // Auxiliar

  //debug
  #ifdef DEBUG
  printf("DEBUG: (queue_size) ini->id = %d\n", ((task_t*) queue)->id);
  #endif

  int tam=1;
  while(aux != ini){
    //debug
    #ifdef DEBUG
    printf("DEBUG: (queue_size) aux->id = %d\n", ((task_t*) aux)->id);
    #endif
    tam++;
    aux = aux->next;
  }

  return tam;
}

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
  queue_t *ini = queue ;  // aponta elemento que sera tratado como inicial
  queue_t *aux = NULL;
  
  if(ini)
    aux = ini->next;   // Auxiliar

  printf("%s [", name);
  print_elem(ini);
  
  while(aux != ini && aux != NULL){
    printf(" ");
    print_elem(aux);
    aux = aux->next;
  }
  
  printf("]\n");
}

//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_append (queue_t **queue, queue_t *elem){
  //Verificacoes
  if(!queue){
    fprintf(stderr, "A fila nao existe\n");
      return -1;
    }
  if(!elem){
    fprintf(stderr, "O elemento nao existe\n");
    return -2;
  }
  if(elem->next || elem->prev ){
    fprintf(stderr, "O elemento esta em outra fila\n");
    return -3;
  }
  //debug
  #ifdef DEBUG
  printf("DEBUG: (queue_append) Verificacoes feitas.\n");
  #endif

  if(!(*queue)){  //Fila vazia
    //debug
    #ifdef DEBUG
    printf("DEBUG: (queue_append) Fila Vazia.\n");
    #endif
    elem->prev = elem;
    elem->next = elem;
    (*queue) = elem;
  }
  else{ //insere no final da fila nao vazia
    queue_t *aux = (*queue)->prev;
    
    //debug
    #ifdef DEBUG
    printf("DEBUG: (queue_append) Insere na Fila.\n");
    #endif

    aux->next = elem;
    elem->prev = aux;
    elem->next = (*queue);
    (*queue)->prev = elem;
  }

  //debug
  #ifdef DEBUG
  printf("DEBUG: (queue_append) Inserido.\n");
  #endif

  return 0;
}

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_remove (queue_t **queue, queue_t *elem) {
  //debug
  #ifdef DEBUG
  printf("DEBUG: (queue_remove) Entrou no remove.\n");
  #endif
  //Verificacoes
  if(!queue){
    fprintf(stderr, "A fila nao existe\n");
      return -1;
    }
  if(!(*queue)){
    fprintf(stderr, "A fila esta vazia\n");
    return -2;
  }
  if(!elem){
    fprintf(stderr, "O elemento nao existe\n");
    return -3;
  }
  
  //Verifica caso especial
  //Se o elemento e o unico na fila 
  if( *queue == elem && elem->next == elem && elem->prev == elem ){
    elem->next = NULL;
    elem->prev = NULL;
    *queue = NULL;
    return 0;
  }

  queue_t *ini = *queue ;         // aponta elemento que sera tratado como inicial
  
  //Se for o primeiro da fila
  if( ini == elem){
    elem->next->prev = elem->prev;
    elem->prev->next = elem->next;
    *queue = elem->next;
    
    elem->next = NULL;
    elem->prev = NULL; 
    return 0;
  }

  //Verifica se o elemento pertence a fila
  //Se achar o elemento na fila, remove ele
  queue_t *aux = (*queue)->next;  // Auxiliar
  
  while( aux != ini && aux != elem){
    aux = aux->next;
  }
  //Se achar o elemento na fila, remove ele
  if( aux == elem){
    elem->next->prev = elem->prev;
    elem->prev->next = elem->next;
    
    elem->next = NULL;
    elem->prev = NULL; 
    return 0;
  }

  //Caso o elemento nao existe na fila
  return -4;

}

