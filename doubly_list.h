#ifndef LIBROUTINE_DOUBLELIST_H_
#define LIBROUTINE_DOUBLELIST_H_

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
    newNode->link_ = this;
    newNode->next = this->tail;
    newNode->prev = this->tail->prev;
    this->tail->prev->next = newNode;
    this->tail->prev = newNode;
    this->cnt++;
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
    this->cnt--;

    return first;
  }

  void delete_node(T* node) {
    if (node == this->head || node == this->tail) {
      return;
    } else {
      node->prev->next = node->next;
      node->next->prev = node->prev;
      node->prev = nullptr;
      node->next = nullptr;
      this->cnt--;
    }
  }

  int size() {
    return this->cnt;
  }
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
};
#endif
