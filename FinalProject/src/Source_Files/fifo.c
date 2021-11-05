/*
 * fifo.c
 *
 *  Created on: Oct 22, 2021
 *      Author: Lena
 */


#include "fifo.h"

struct fifo_t* create_fifo(int size) {
    struct fifo_t *fifo = malloc(sizeof(struct fifo_t));
    struct node_t *head, *lastCreated;
    head = create_new_node();
    lastCreated = head;
    for(int i = 1; i < size; i++){
        struct node_t* newNode = create_new_node();
        lastCreated->next = newNode;
        lastCreated = newNode;
    }
    lastCreated->next = head;
    fifo->head = head;
    fifo->size = size;
    return fifo;
}

struct node_t* create_new_node() {
    struct node_t* newNode = malloc(sizeof(struct node_t));
    newNode->button_transition = 0;
    return newNode;
}

int pop(struct fifo_t* fifo) {
    int out = fifo->head->button_transition;
    fifo->head->button_transition = 0;
    fifo->head = fifo->head->next;
    return out;
}

int push(struct fifo_t* fifo, int transition) {
    struct node_t* index = fifo->head;
    int i = 0;
    int full = 0;
    if(index->button_transition == 0){
        index->button_transition = transition;
        return 1; //The transition has been pushed!
    }
    else{
        while(index->next->button_transition != 0){
            index = index->next;
            if(i >= fifo->size){
                full = 1;
            }
            i++;
        }
       if(!full){
           index->next->button_transition = transition;
           return 1; //The transition has been pushed!
        }
        else{
            return 0; //The fifo is full!
        }
    }
}

int is_empty(struct fifo_t* fifo) {
    if(fifo->head->button_transition == 0){
        return 1;
    } else return 0;
}

void empty_fifo(struct fifo_t* fifo) {
    while (!is_empty(fifo)){
        pop(fifo);
    }
}
