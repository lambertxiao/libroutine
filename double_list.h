#ifndef LIBROUTINE_DOUBLELIST_H_
#define LIBROUTINE_DOUBLELIST_H_

template <typename T>
class Node {
 public:
  T data;
  Node<T>* next;
  Node<T>* prev;

  Node(T data) {
    this->data = data;
    this->next = nullptr;
    this->prev = nullptr;
  }
};

template <typename T>
class DoublyLinkedList {
 public:
  Node<T>* head;
  Node<T>* tail;

  DoublyLinkedList() {
    this->head = new Node<T>();
    this->tail = new Node<T>();
    this->head->next = this->tail;
    this->tail->prev = this->head;
  }

  void addFront(Node<T> newNode) {
    newNode->next = this->head->next;
    newNode->prev = this->head;
    this->head->next->prev = newNode;
    this->head->next = newNode;
  }

  void addBack(Node<T> newNode) {
    newNode->next = this->tail;
    newNode->prev = this->tail->prev;
    this->tail->prev->next = newNode;
    this->tail->prev = newNode;
  }

  void deleteNode(Node<T>* node) {
    if (node == this->head || node == this->tail) {
      return;
    } else {
      node->prev->next = node->next;
      node->next->prev = node->prev;
    }
  }
};
#endif
