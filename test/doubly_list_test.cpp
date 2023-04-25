#include "../doubly_list.h"

struct Node : public DoublyLinkedListNode<Node> {};
class NodeList : public DoublyLinkedList<Node> {};

int main() {
  auto list = new NodeList();

  for (int i = 0; i < 10; i++) {
    list->add_back(new Node());
  }

  auto list2 = new NodeList();

  auto curr = list->head->next;
  while (curr != list->tail) {
    auto node = curr;
    curr = curr->next;
    
    list->delete_node(node);
    list2->add_back(node);
  }

  LOG_DEBUG("loop list1------");
  auto curr2 = list->head->next;
  while (curr2 != list->tail) {
    printf("%p in link:%p\n", curr2, curr2->link_);
    curr2 = curr2->next;
  }

  LOG_DEBUG("loop list2------");
  auto curr3 = list2->head->next;
  while (curr3 != list2->tail) {
    printf("%p in link:%p\n", curr3, curr3->link_);
    curr3 = curr3->next;
  }
  return 0;
}
