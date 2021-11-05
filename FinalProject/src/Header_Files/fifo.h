/*
 * fifo.h
 *
 *  Created on: Oct 22, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_FIFO_H_
#define SRC_HEADER_FILES_FIFO_H_


//----------------------------------------------------------------------------------------------------------------------------------
/// @brief Structure which holds the Queue's current head and size
//----------------------------------------------------------------------------------------------------------------------------------
struct fifo_t{
  //Number of storable transitions
  int size;

  //Pointer to the current head of the fifo
  struct node_t* head;
};

//----------------------------------------------------------------------------------------------------------------------------------
/// @brief Structure which holds information on a transition
//----------------------------------------------------------------------------------------------------------------------------------

struct node_t {
    // Button transition info
    int button_transition;

    // Pointer to the next node in the queue
    struct node_t* next;
};

//----------------------------------------------------------------------------------------------------------------------------------
/// @brief Creates a queue.
///
/// @param[in] task The task information
/// @param[in] size The size of the task array
///
/// @return the head of the new queue
//----------------------------------------------------------------------------------------------------------------------------------
struct fifo_t* create_fifo(int size);

//----------------------------------------------------------------------------------------------------------------------------------
/// @brief Create a new node for the fifo
///
/// @return a newly allocated node
//----------------------------------------------------------------------------------------------------------------------------------
struct node_t* create_new_node();

//----------------------------------------------------------------------------------------------------------------------------------
/// @brief Returns and clears the transition at the top of the queue.
///
/// @param fifo The pointer to the fifo struct.
///
/// @return the button transition in the head position
//----------------------------------------------------------------------------------------------------------------------------------
int pop(struct fifo_t* fifo);

//----------------------------------------------------------------------------------------------------------------------------------
/// @brief Push a new transition into the queue
///
/// @param head The head of the queue
/// @param task The task to be put into the queue
///
/// @return "1" if the push was successful, "0" if the push failed
//----------------------------------------------------------------------------------------------------------------------------------
int push(struct fifo_t* fifo, int transition);

//----------------------------------------------------------------------------------------------------------------------------------
/// @brief Determines whether the specified head is empty.
///
/// @param head The head of the FIFO
///
/// @return True if the specified head is empty, False otherwise.
//----------------------------------------------------------------------------------------------------------------------------------
int is_empty(struct fifo_t* fifo);

//----------------------------------------------------------------------------------------------------------------------------------
/// @brief Remove all items from the queue
///
/// @param fifo The pointer to the fifo struct
//----------------------------------------------------------------------------------------------------------------------------------
void empty_fifo(struct fifo_t* fifo);


#endif /* SRC_HEADER_FILES_FIFO_H_ */
