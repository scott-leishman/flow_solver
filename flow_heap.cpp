#include <cassert>
#include <iostream>

#include "flow_heap.h"
#include "utest.h"

//////////////////////////////////////////////////////////////////////

FlowHeap::FlowHeap(size_t capacity) : _capacity(capacity) {
}

//////////////////////////////////////////////////////////////////////

FlowHeapPriorityQueue::FlowHeapPriorityQueue(size_t capacity)
: FlowHeap(capacity) {
  _start  = (tree_node_t**)new tree_node_t[_capacity];
  _count = 0;
}

FlowHeapPriorityQueue::~FlowHeapPriorityQueue() {
  delete[] _start;
}

//////////////////////////////////////////////////////////////////////
// Indexing macros for heaps
#define HEAPQ_PARENT_INDEX(i) (((i)-1)/2)
#define HEAPQ_LCHILD_INDEX(i) ((2*(i))+1)

void
FlowHeapPriorityQueue::enqueue(tree_node_t* node) {
  assert(_count < _capacity);

  size_t i = _count++;
  size_t pi = HEAPQ_PARENT_INDEX(i);
  
  _start[i] = node;
  
  while (i > 0 && node_compare(_start[pi], _start[i]) > 0) {
    tree_node_t* tmp = _start[pi];
    _start[pi] = _start[i];
    _start[i] = tmp;
    i = pi;
    pi = HEAPQ_PARENT_INDEX(i);
  }          
}

tree_node_t*
FlowHeapPriorityQueue::dequeue() {
  assert(!empty());

  tree_node_t* rval = _start[0];
  --_count;

  if (_count) {
    _start[0] = _start[_count];
    repair(0);
  }

  return rval;
}

bool
FlowHeapPriorityQueue::empty() const{
  return _count == 0;
}

const tree_node_t*
FlowHeapPriorityQueue::peek() const {
  assert(!empty());
  return _start[0];
}


int
FlowHeapPriorityQueue::node_compare(const tree_node_t* a,
                 const tree_node_t* b) const {

  uint64_t af = a->cost_to_come + a->cost_to_go;
  uint64_t bf = b->cost_to_come + b->cost_to_go;

  if (af != bf) {
    return af < bf ? -1 : 1;
  } else {
    return a < b ? -1 : 1;
  }
}

void
FlowHeapPriorityQueue::repair(size_t i) {
  size_t li = HEAPQ_LCHILD_INDEX(i);
  size_t ri = li + 1;
  size_t smallest = i;

  if (li < _count &&
      node_compare(_start[i], _start[li]) > 0) {
    smallest = li;
  }

  if (ri < _count &&
      node_compare(_start[smallest], _start[ri]) > 0) {
    smallest = ri;
  }

  if (smallest != i){
    tree_node_t* tmp = _start[i];
    _start[i] = _start[smallest];
    _start[smallest] = tmp;
    repair(smallest);
  }    
}

// Check that the heap property is maintained. Useful for debugging.
int
FlowHeapPriorityQueue::valid() const {
  for (size_t i=1; i<_count; ++i) {
    const tree_node_t* tc = _start[i];
    const tree_node_t* tp = _start[HEAPQ_PARENT_INDEX(i)];
    if (node_compare(tp, tc) > 0) {
      return 0;
    }
  }
  return 1;
}


//////////////////////////////////////////////////////////////////////

FlowHeapFifoQueue::FlowHeapFifoQueue(size_t capacity) : FlowHeap(capacity) {
  _start  = (tree_node_t**)new tree_node_t[_capacity];

  _count = 0;
  _next = 0;
}

FlowHeapFifoQueue::~FlowHeapFifoQueue() {
  delete[] _start;
}

void
FlowHeapFifoQueue::enqueue(tree_node_t* node) {
  assert(_count < _capacity);
  _start[_count++] = node;
}

tree_node_t*
FlowHeapFifoQueue::dequeue() {
  assert(_next < _count);
  return _start[_next++];
}

bool
FlowHeapFifoQueue::empty() const {
  return _next == _count;
}

const tree_node_t*
FlowHeapFifoQueue::peek() const {
  assert(!empty());
  return _start[_next];
}


//////////////////////////////////////////////////////////////////////

FlowHeapStdPriorityQueue::FlowHeapStdPriorityQueue(size_t capacity) : FlowHeap(capacity) {
} 

FlowHeapStdPriorityQueue::~FlowHeapStdPriorityQueue() {
}

void
FlowHeapStdPriorityQueue::enqueue(tree_node_t* node) {
  _nodes.push(node);
}

tree_node_t*
FlowHeapStdPriorityQueue::dequeue() {
  assert(!empty());
  tree_node_t* returnval = _nodes.top();
  _nodes.pop();
  return returnval;
}

bool
FlowHeapStdPriorityQueue::empty() const {
  return _nodes.empty();
}

const tree_node_t*
FlowHeapStdPriorityQueue::peek() const {
  assert(!empty());
  return _nodes.top();
}

//////////////////////////////////////////////////////////////////////

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

// Compare fuction to select node with minimum cost.
struct {
    bool operator()(const tree_node_t* a, const tree_node_t* b) const {
        uint64_t af = a->cost_to_come + a->cost_to_go;
        uint64_t bf = b->cost_to_come + b->cost_to_go;
        return af > bf;
    }
} tree_node_t_compare;

UTEST_F(HeapTestFixture, prioity_queue_class_simple)
{
    FlowHeapPriorityQueue q(5);
    EXPECT_TRUE(q.empty());
    q.enqueue(&utest_fixture->node1);
    q.enqueue(&utest_fixture->node2);
    q.enqueue(&utest_fixture->node3);
    EXPECT_FALSE(q.empty());

    // expect nodes in cost order from min to max: node1, node3, node2
    tree_node_t *node{nullptr};
    node = (tree_node_t*)q.dequeue();
    EXPECT_EQ(node, &utest_fixture->node1);
    node = (tree_node_t*)q.dequeue();
    EXPECT_EQ(node, &utest_fixture->node3);
    node = (tree_node_t*)q.dequeue();
    EXPECT_EQ(node, &utest_fixture->node2);

    EXPECT_TRUE(q.empty());
}

UTEST_F(HeapTestFixture, fifo_class_simple)
{
    FlowHeapFifoQueue q(5);
    EXPECT_TRUE(q.empty());
    q.enqueue(&utest_fixture->node1);
    q.enqueue(&utest_fixture->node2);
    q.enqueue(&utest_fixture->node3);
    EXPECT_FALSE(q.empty());

    // expect nodes in fifo order from min to max: node1, node2, node3
    tree_node_t *node{nullptr};
    node = (tree_node_t*)q.dequeue();
    EXPECT_EQ(node, &utest_fixture->node1);
    node = (tree_node_t*)q.dequeue();
    EXPECT_EQ(node, &utest_fixture->node2);
    node = (tree_node_t*)q.dequeue();
    EXPECT_EQ(node, &utest_fixture->node3);
    
    EXPECT_TRUE(q.empty());
}

UTEST_F(HeapTestFixture, std_queue_class_simple)
{
    FlowHeapStdPriorityQueue q(5);
    EXPECT_TRUE(q.empty());
    q.enqueue(&utest_fixture->node1);
    q.enqueue(&utest_fixture->node2);
    q.enqueue(&utest_fixture->node3);
    EXPECT_FALSE(q.empty());

    // expect nodes in cost order from min to max: node1, node3, node2
    tree_node_t *node{nullptr};
    node = (tree_node_t*)q.dequeue();
    EXPECT_EQ(node, &utest_fixture->node1);
    node = (tree_node_t*)q.dequeue();
    EXPECT_EQ(node, &utest_fixture->node3);
    node = (tree_node_t*)q.dequeue();
    EXPECT_EQ(node, &utest_fixture->node2);

    EXPECT_TRUE(q.empty());
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

UTEST_F(HeapScaleFixture, priority_queue_class_scale)
{
    FlowHeapPriorityQueue pq(10000);
    EXPECT_TRUE(pq.empty());

    for (int i = 0; i < 10000; i++) {
        pq.enqueue(&utest_fixture->nodes[i]);
    }

    for (int i = 0; i < 10000; i++) {
        // EXPECT_EQ(pq.peek(), &utest_fixture->nodes[i]);
        pq.dequeue();
    }
}

UTEST_F(HeapScaleFixture, fifo_class_scale)
{
    FlowHeapFifoQueue pq(10000);
    EXPECT_TRUE(pq.empty());

    for (int i = 0; i < 10000; i++) {
        pq.enqueue(&utest_fixture->nodes[i]);
    }

    for (int i = 0; i < 10000; i++) {
        // EXPECT_EQ(pq.peek(), &utest_fixture->nodes[i]);
        pq.dequeue();
    }
}

UTEST_F(HeapScaleFixture, std_queue_class_scale)
{
    FlowHeapStdPriorityQueue pq(10000);
    EXPECT_TRUE(pq.empty());

    for (int i = 0; i < 10000; i++) {
        pq.enqueue(&utest_fixture->nodes[i]);
    }

    for (int i = 0; i < 10000; i++) {
        // EXPECT_EQ(pq.peek(), &utest_fixture->nodes[i]);
        pq.dequeue();
    }
}

