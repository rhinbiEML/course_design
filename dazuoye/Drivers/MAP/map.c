#include "./MAP/map.h"

List *create_map(void) {
    List *list = (List *)malloc(sizeof(List));
    list->head = (Node *)malloc(sizeof(Node));
    list->tail = (Node *)malloc(sizeof(Node));
    list->head->prev = NULL;
    list->head->next = list->tail;
    list->tail->prev = list->head;
    list->tail->next = NULL;
    return list;
}

void add_node(List *list, Node *new_node) {
    new_node->next = list->tail;
    new_node->prev = list->tail->prev;
    list->tail->prev->next = new_node;
    list->tail->prev = new_node;
}

void delete_node(List *list, Node *node) {
    if (node == list->head || node == list->tail) {
        return;
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);
}

void add_path(List *list, uint16_t screen_width, uint16_t path_position) {
    // 在链表中添加新节点
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->x = path_position;
    new_node->y = 0;
    add_node(list, new_node);
}