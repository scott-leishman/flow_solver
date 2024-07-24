#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <cstddef>
#include <iostream>

#include "flow_heap.h"
#include "utest.h"

// Various cell types, all but freespace have a color
enum {
  TYPE_FREE = 0, // Free space
  TYPE_PATH = 1, // Path between init & goal
  TYPE_INIT = 2, // Starting point
  TYPE_GOAL = 3  // Goal position
};

// Enumerate cardinal directions so we can loop over them
// RIGHT is increasing x, DOWN is increasing y.
enum {
  DIR_LEFT  = 0,
  DIR_RIGHT = 1,
  DIR_UP    = 2,
  DIR_DOWN  = 3
};

// Search termination results
enum {
  SEARCH_SUCCESS = 0,
  SEARCH_UNREACHABLE = 1,
  SEARCH_FULL = 2,
  SEARCH_IN_PROGRESS = 3,
};

// Represent the contents of a cell on the game board
typedef uint8_t cell_t;

// Represent a position within the game board
typedef uint8_t pos_t;

// Match color characters to ANSI color codes
typedef struct color_lookup_struct {
  char input_char;   // Color character
  char display_char; // Punctuation a la nethack
  const char* ansi_code;    // ANSI color code
  const char* fg_rgb;
  const char* bg_rgb;
} color_lookup_t;

// Used for auto-sorting colors
typedef struct color_features_struct {
  int index;
  int user_index;
  int wall_dist[2];
  int min_dist;
} color_features_t;

// Disjoint-set data structure for connected component analysis of free
// space (see region_ functions).
typedef struct region_struct {
  // Parent index (or INVALID_POS) if no non-free space
  pos_t parent;
  // Rank (see wikipedia article).
  uint8_t rank;
} region_t;

//////////////////////////////////////////////////////////////////////
// Compare total cost for nodes, used by heap functions below.

int node_compare(const tree_node_t* a,
                 const tree_node_t* b) {

  uint64_t af = a->cost_to_come + a->cost_to_go;
  uint64_t bf = b->cost_to_come + b->cost_to_go;

  if (af != bf) {
    return af < bf ? -1 : 1;
  } else {
    return a < b ? -1 : 1;
  }

}

//////////////////////////////////////////////////////////////////////
// Create a binary heap to store the given # of nodes

queue_t heapq_create(size_t max_nodes) {
  queue_t rval;
  rval.heapq.start = (tree_node_t**)malloc(sizeof(tree_node_t*) * max_nodes);
  if (!rval.heapq.start) {
	  std::cerr << "out of memory creating heapq!" << std::endl;
    exit(1);
  }
  rval.heapq.count = 0;
  rval.heapq.capacity = max_nodes;
  return rval;
}

//////////////////////////////////////////////////////////////////////
// Indexing macros for heaps

#define HEAPQ_PARENT_INDEX(i) (((i)-1)/2)
#define HEAPQ_LCHILD_INDEX(i) ((2*(i))+1)

//////////////////////////////////////////////////////////////////////
// For debugging, not used presently

int heapq_valid(const queue_t* q) {
  for (size_t i=1; i<q->heapq.count; ++i) {
    const tree_node_t* tc = q->heapq.start[i];
    const tree_node_t* tp = q->heapq.start[HEAPQ_PARENT_INDEX(i)];
    if (node_compare(tp, tc) > 0) {
      return 0;
    }
  }
  return 1;
}

//////////////////////////////////////////////////////////////////////
// Is heap queue empty?

int heapq_empty(const queue_t* q) {
  return q->heapq.count == 0;
}

//////////////////////////////////////////////////////////////////////
// Peek at the next item to be removed

const tree_node_t* heapq_peek(const queue_t* q) {
  assert(!heapq_empty(q));
  return q->heapq.start[0];
}

//////////////////////////////////////////////////////////////////////
// Enqueue a node onto the heap

void heapq_enqueue(queue_t* q, tree_node_t* node) {

  assert(q->heapq.count < q->heapq.capacity);

  size_t i = q->heapq.count++;
  size_t pi = HEAPQ_PARENT_INDEX(i);
  
  q->heapq.start[i] = node;
  
  while (i > 0 && node_compare(q->heapq.start[pi], q->heapq.start[i]) > 0) {
    tree_node_t* tmp = q->heapq.start[pi];
    q->heapq.start[pi] = q->heapq.start[i];
    q->heapq.start[i] = tmp;
    i = pi;
    pi = HEAPQ_PARENT_INDEX(i);
  }
                      
}

//////////////////////////////////////////////////////////////////////
// Helper function used for dequeueing

void _heapq_repair(queue_t* q, size_t i) {

  size_t li = HEAPQ_LCHILD_INDEX(i);
  size_t ri = li + 1;
  size_t smallest = i;

  if (li < q->heapq.count &&
      node_compare(q->heapq.start[i], q->heapq.start[li]) > 0) {
    smallest = li;
  }

  if (ri < q->heapq.count &&
      node_compare(q->heapq.start[smallest], q->heapq.start[ri]) > 0) {
    smallest = ri;
  }

  if (smallest != i){
    tree_node_t* tmp = q->heapq.start[i];
    q->heapq.start[i] = q->heapq.start[smallest];
    q->heapq.start[smallest] = tmp;
    _heapq_repair(q, smallest);
  }    

}

//////////////////////////////////////////////////////////////////////
// Pop a node off the heap

tree_node_t* heapq_deque(queue_t* q) {

  assert(!heapq_empty(q));

  tree_node_t* rval = q->heapq.start[0];
  --q->heapq.count;

  if (q->heapq.count) {
    q->heapq.start[0] = q->heapq.start[q->heapq.count];
    _heapq_repair(q, 0);
  }
  
  return rval;
  
}

//////////////////////////////////////////////////////////////////////
// Free memory allocated for heap

void heapq_destroy(queue_t* q) {
  free(q->heapq.start);
}

//////////////////////////////////////////////////////////////////////
// FIFO via flat array

queue_t fifo_create(size_t max_nodes) {
  queue_t rval;
  rval.fifo.start = (tree_node_t**)malloc(sizeof(tree_node_t*) * max_nodes);
  if (!rval.fifo.start) {
    std::cerr << "out of memory creating fifo!" << std::endl;
    exit(1);
  }
  rval.fifo.count = 0;
  rval.fifo.capacity = max_nodes;
  rval.fifo.next = 0;
  return rval;
}

//////////////////////////////////////////////////////////////////////
// Check if empty

int fifo_empty(const queue_t* q) {
  return q->fifo.next == q->fifo.count;
}

//////////////////////////////////////////////////////////////////////
// Push node into FIFO

void fifo_enqueue(queue_t* q, tree_node_t* n) {
  assert(q->fifo.count < q->fifo.capacity);
  q->fifo.start[q->fifo.count++] = n;
}

//////////////////////////////////////////////////////////////////////
// Peek at current FIFO node

const tree_node_t* fifo_peek(const queue_t* q) {
  assert(!fifo_empty(q));
  return q->fifo.start[q->fifo.next];
}

//////////////////////////////////////////////////////////////////////
// Dequeue node from FIFO

tree_node_t* fifo_deque(queue_t* q) {
  assert(!fifo_empty(q));
  return q->fifo.start[q->fifo.next++];
}

//////////////////////////////////////////////////////////////////////
// De-allocate storage for FIFO

void fifo_destroy(queue_t* q) {
  free(q->fifo.start);
}

//////////////////////////////////////////////////////////////////////
// Call this before calling the generic queue functions above.

void queue_setup(bool use_search_best_first) {

  if (use_search_best_first) {

    queue_create = heapq_create;
    queue_enqueue = heapq_enqueue;
    queue_deque = heapq_deque;
    queue_destroy = heapq_destroy;
    queue_empty = heapq_empty;
    queue_peek = heapq_peek;

  } else {

    queue_create = fifo_create;
    queue_enqueue = fifo_enqueue;
    queue_deque = fifo_deque;
    queue_destroy = fifo_destroy;
    queue_empty = fifo_empty;
    queue_peek = fifo_peek;

  }

}

// utest fixture to create three tree_node_t nodes that will be used in
// the folowing tests.
struct HeapTestFixture {
  tree_node_t node1;
  tree_node_t node2;
  tree_node_t node3;
};

UTEST_F_SETUP(HeapTestFixture) {
    utest_fixture->node1.cost_to_come = 1;
    utest_fixture->node1.cost_to_go = 2;

    utest_fixture->node2.cost_to_come = 20;
    utest_fixture->node2.cost_to_go = 21;

    utest_fixture->node3.cost_to_come = 10;
    utest_fixture->node3.cost_to_go = 11;
}

UTEST_F_TEARDOWN(HeapTestFixture) {
  // nothing to do
}

UTEST_F(HeapTestFixture, best_first)
{
    constexpr bool use_search_best_first = true;
    constexpr bool use_fifo = false;
    constexpr size_t max_nodes = 5;

    // initialise queue
    queue_setup(use_search_best_first);
    queue_t q = queue_create(max_nodes);

    // enqueue nodes
    queue_enqueue(&q, &utest_fixture->node1);
    queue_enqueue(&q, &utest_fixture->node2);
    queue_enqueue(&q, &utest_fixture->node3);

    // expect nodes in cost order from min to max: node1, node3, node2
    tree_node_t *node{nullptr};
    node = (tree_node_t*)queue_deque(&q);
    EXPECT_EQ(node, &utest_fixture->node1);
    node = (tree_node_t*)queue_deque(&q);
    EXPECT_EQ(node, &utest_fixture->node3);
    node = (tree_node_t*)queue_deque(&q);
    EXPECT_EQ(node, &utest_fixture->node2);

    EXPECT_TRUE(queue_empty(&q));

    queue_destroy(&q);
}


UTEST_F(HeapTestFixture, fifo)
{
    constexpr bool use_search_best_first = true;
    constexpr bool use_fifo = false;
    constexpr size_t max_nodes = 5;

    // initialise queue
    queue_setup(use_fifo);
    queue_t q = queue_create(max_nodes);

    // enqueue nodes
    queue_enqueue(&q, &utest_fixture->node1);
    queue_enqueue(&q, &utest_fixture->node2);
    queue_enqueue(&q, &utest_fixture->node3);

    // expect nodes in fifo order from min to max: node1, node2, node3
    tree_node_t *node{nullptr};
    node = (tree_node_t*)queue_deque(&q);
    EXPECT_EQ(node, &utest_fixture->node1);
    node = (tree_node_t*)queue_deque(&q);
    EXPECT_EQ(node, &utest_fixture->node2);
    node = (tree_node_t*)queue_deque(&q);
    EXPECT_EQ(node, &utest_fixture->node3);

    EXPECT_TRUE(queue_empty(&q));

    queue_destroy(&q);
}

#include <queue>

UTEST(FlowHeap, std_priority_queue)
{
    std::priority_queue<int> pq;
    pq.push(3);
    pq.push(1);
    pq.push(2);
    EXPECT_EQ(pq.top(), 3);
    pq.pop();
    EXPECT_EQ(pq.top(), 2);
    pq.pop();
    EXPECT_EQ(pq.top(), 1);
    pq.pop();
    EXPECT_TRUE(pq.empty());
}

UTEST_F(HeapTestFixture, tree_node_t)
{
    std::priority_queue<tree_node_t *> pq;
    EXPECT_TRUE(pq.empty());

    pq.push(&utest_fixture->node1);
    pq.push(&utest_fixture->node2);
    pq.push(&utest_fixture->node3);

    EXPECT_EQ(pq.top(), &utest_fixture->node3);
    pq.pop();
    EXPECT_EQ(pq.top(), &utest_fixture->node2);
    pq.pop();
    EXPECT_EQ(pq.top(), &utest_fixture->node1);
}

// Compare fuction to select node with minimum cost.
struct {
    bool operator()(const tree_node_t* a, const tree_node_t* b) const {
        uint64_t af = a->cost_to_come + a->cost_to_go;
        uint64_t bf = b->cost_to_come + b->cost_to_go;
        return af > bf;
    }
} tree_node_t_compare;

UTEST_F(HeapTestFixture, std_priority_queue_tree_node_t)
{
    std::priority_queue<tree_node_t *, std::vector<tree_node_t *>,
        decltype(tree_node_t_compare)>
        pq(tree_node_t_compare);
    EXPECT_TRUE(pq.empty());

    pq.push(&utest_fixture->node1);
    pq.push(&utest_fixture->node2);
    pq.push(&utest_fixture->node3);

    // expect nodes in cost order from min to max: node1, node3, node2
    EXPECT_EQ(pq.top(), &utest_fixture->node1);
    pq.pop();
    EXPECT_EQ(pq.top(), &utest_fixture->node3);
    pq.pop();
    EXPECT_EQ(pq.top(), &utest_fixture->node2);
}

struct HeapScaleFixture {
  tree_node_t nodes[10000];
};

UTEST_F_SETUP(HeapScaleFixture) {
    for (int i = 0; i < 10000; i++) {
//        utest_fixture->nodes[i].cost_to_come = 10000 - i;
//        utest_fixture->nodes[i].cost_to_go = i + 1;
        utest_fixture->nodes[i].cost_to_come = i;
        utest_fixture->nodes[i].cost_to_go = i + 1;
    } 
}

UTEST_F_TEARDOWN(HeapScaleFixture) {
  // nothing to do
}

UTEST_F(HeapScaleFixture, std_priority_queue_tree_node_t)
{
    std::priority_queue<tree_node_t *, std::vector<tree_node_t *>,
        decltype(tree_node_t_compare)>
        pq(tree_node_t_compare);
    EXPECT_TRUE(pq.empty());

    for (int i = 0; i < 10000; i++) {
        pq.push(&utest_fixture->nodes[i]);
    }

    for (int i = 0; i < 10000; i++) {
        // EXPECT_EQ(pq.top(), &utest_fixture->nodes[i]);
        pq.pop();
    }
}

UTEST_F(HeapScaleFixture, best_first) {
    constexpr bool use_search_best_first = true;
    constexpr bool use_fifo = false;
    constexpr size_t max_nodes = 10000;

    // initialise queue
    queue_setup(use_search_best_first);
    queue_t q = queue_create(max_nodes);

    for (int i = 0; i < 10000; i++) {
        queue_enqueue(&q, &utest_fixture->nodes[i]);
    }

    for (int i = 0; i < 10000; i++) {
        tree_node_t *node = (tree_node_t*)queue_deque(&q);
        // EXPECT_EQ(node, &utest_fixture->nodes[i]);
    }

    // queue_destroy(&q);
}

UTEST_F(HeapScaleFixture, fifo) {
    constexpr bool use_search_best_first = true;
    constexpr bool use_fifo = false;
    constexpr size_t max_nodes = 10002;

    // initialise queue
    queue_setup(use_fifo);
    queue_t q = queue_create(max_nodes);

    for (int i = 0; i < 10000; i++) {
        queue_enqueue(&q, &utest_fixture->nodes[i]);
    }

    for (int i = 0; i < 10000; i++) {
        tree_node_t *node = (tree_node_t*)queue_deque(&q);
        // EXPECT_EQ(node, &utest_fixture->nodes[i]);
    }

    // queue_destroy(&q);
}