#ifndef FLOW_HEAP_H
#define FLOW_HEAP_H

#include "game_state.h"

// Search node for A* / BFS.
typedef struct tree_node_struct {
  game_state_t state;              // Current game state
  uint64_t cost_to_come;             // Cost to come (ignored for BFS)
  uint64_t cost_to_go;               // Heuristic cost (ignored for BFS)
  struct tree_node_struct* parent; // Parent of this node (may be NULL)
} tree_node_t;

// Strategy is to pre-allocate a big block of nodes in advance, and
// hand them out in order.
typedef struct node_storage_struct {
  tree_node_t* start; // Allocated block
  size_t capacity;    // How many nodes did we allocate?
  size_t count;       // How many did we give out?
} node_storage_t;

// Data structure for heap based priority queue
typedef struct heapq_struct {
  tree_node_t** start; // Array of node pointers
  size_t capacity;     // Maximum allowable queue size
  size_t count;        // Number enqueued
} heapq_t;

// First in, first-out queue implemented as an array of pointers.
typedef struct fifo_struct {
  tree_node_t** start; // Array of node pointers
  size_t capacity;     // Maximum number of things to enqueue ever
  size_t count;        // Total enqueued (next one will go into start[count])
  size_t next;         // Next index to dequeue
} fifo_t;

// Union struct for passing around queues.
typedef union queue_union {
  heapq_t heapq;
  fifo_t  fifo;
} queue_t;

// Function pointers for either type of queue
queue_t (*queue_create)(size_t) = 0;
void (*queue_enqueue)(queue_t*, tree_node_t*) = 0;
tree_node_t* (*queue_deque)(queue_t*) = 0;
void (*queue_destroy)(queue_t*) = 0;
int (*queue_empty)(const queue_t*) = 0;
const tree_node_t* (*queue_peek)(const queue_t*) = 0;

#endif // FLOW_HEAP_H