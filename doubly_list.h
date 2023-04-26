#ifndef LIBROUTINE_DOUBLELIST_H_
#define LIBROUTINE_DOUBLELIST_H_

#include "logger.h"

template <typename T>
class DoublyLinkedList {
 public:
  T* head;
  T* tail;
  int cnt;

  DoublyLinkedList() {
    this->head = new T();
    this->tail = new T();
    this->head->next = this->tail;
    this->tail->prev = this->head;
    this->cnt = 0;
  }

  ~DoublyLinkedList() {
    delete this->head;
    delete this->tail;
  }

  void add_front(T* newNode) {
    newNode->link_ = this;
    newNode->next = this->head->next;
    newNode->prev = this->head;
    this->head->next->prev = newNode;
    this->head->next = newNode;
    this->cnt++;
  }

  void add_back(T* newNode) {
    if (this->head == newNode || this->tail == newNode) {
      LOG_ERROR("add invalid node %p head %p tail:%p", newNode, this->head, this->tail);
      return;
    }
    newNode->link_ = this;
    newNode->next = this->tail;
    newNode->prev = this->tail->prev;
    this->tail->prev->next = newNode;
    this->tail->prev = newNode;
    this->cnt++;

    // LOG_DEBUG("add node:%p in link:%p prev %p next %p", newNode, this, newNode->prev, newNode->next, this->head, this->tail);
  }

  T* pop_front() {
    if (this->head->next == this->tail) {
      return nullptr;
    }
    auto first = this->head->next;
    this->head->next = first->next;
    first->next->prev = this->head;
    first->prev = nullptr;
    first->next = nullptr;
    first->link_ = nullptr;
    this->cnt--;

    // LOG_DEBUG("pop node:%p from link:%p", first, this);

    return first;
  }

  void delete_node(T* node) {
    // LOG_DEBUG("delete node:%p in link:%p", node, this);
    if (node->link_ != this) {
      LOG_ERROR("no match link node, node:%p node->link:%p link:%p", node, node->link_, this);
      return;
    }
    if (node == this->head || node == this->tail) {
      LOG_DEBUG("cannot delete head or tail node");
      return;
    }

    if (node->next == node || node->prev == node) {
      LOG_ERROR("invalid node %p prev %p next %p", node, node->next, node->prev);
      return;
    }

    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = nullptr;
    node->next = nullptr;
    node->link_ = nullptr;
    this->cnt--;
  }

  void print_nodes() {
    auto curr = this->head->next;
    while (curr != this->tail) {
      LOG_DEBUG("print node %p in %p", curr, curr->link_);
      curr = curr->next;
    }
  }

  int size() { return this->cnt; }
};

template <typename T>
class DoublyLinkedListNode {
 public:
  T* next;
  T* prev;
  DoublyLinkedList<T>* link_;

  DoublyLinkedListNode() {
    this->next = nullptr;
    this->prev = nullptr;
    this->link_ = nullptr;
  }

  ~DoublyLinkedListNode() {
    this->next = nullptr;
    this->prev = nullptr;
    this->link_ = nullptr;
  }
};
#endif
