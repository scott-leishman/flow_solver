#ifndef FLOW_HEAP_H
#define FLOW_HEAP_H

#include <queue>
#include "game_state.h"

class FlowHeap {
public:
  virtual void enqueue(tree_node_t* node) = 0;
  virtual tree_node_t* dequeue() = 0;
  virtual bool empty() const = 0;
  virtual const tree_node_t* peek() const = 0;
protected:
  FlowHeap(size_t capacity); //Only subclasses can create this object
  virtual ~FlowHeap() = default;
  
  size_t _capacity;
};

class FlowHeapPriorityQueue : public FlowHeap {
public:
  FlowHeapPriorityQueue(size_t capacity);
  ~FlowHeapPriorityQueue();
  void enqueue(tree_node_t* node) override;  // not-done
  tree_node_t* dequeue() override;
  bool empty() const override;
  const tree_node_t* peek() const override;
private:
  int node_compare(const tree_node_t* a, const tree_node_t* b) const;
  void repair(size_t index);
  int valid() const;

  tree_node_t** _start; // Array of node pointers
  size_t _count;        // Number enqueued
};

class FlowHeapFifoQueue : public FlowHeap {
public:
  FlowHeapFifoQueue(size_t capacity);
  ~FlowHeapFifoQueue();
  void enqueue(tree_node_t* node) override;
  tree_node_t* dequeue() override;
  bool empty() const override;
  const tree_node_t* peek() const override;
private:
  tree_node_t** _start; // Array of node pointers
  size_t _count;        // Total enqueued (next one will go into _start[_count])
  size_t _next;         // Next index to dequeue
};

class FlowHeapStdPriorityQueue : public FlowHeap {
public:
  FlowHeapStdPriorityQueue(size_t capacity);
  ~FlowHeapStdPriorityQueue();
  void enqueue(tree_node_t* node) override;
  tree_node_t* dequeue() override;
  bool empty() const override;
  const tree_node_t* peek() const override;
private:
  // Compare fuction to select node with minimum cost.
  struct tree_node_t_compare {
    bool operator()(const tree_node_t* a, const tree_node_t* b) const {
        uint64_t af = a->cost_to_come + a->cost_to_go;
        uint64_t bf = b->cost_to_come + b->cost_to_go;
        return af > bf;
    }
  };

  std::priority_queue<tree_node_t *,
                      std::vector<tree_node_t *>,
                      tree_node_t_compare> _nodes;

};


#endif // FLOW_HEAP_H