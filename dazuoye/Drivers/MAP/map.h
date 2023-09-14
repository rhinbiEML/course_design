#include "stdlib.h"
#include "./SYSTEM/sys/sys.h"
typedef struct Node {
    uint16_t x;
    uint16_t y;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct List {
    Node *head;
    Node *tail;
} List;

List* create_map(void);
void add_node(List *list, Node *new_node);
void delete_node(List *list, Node *node);
void add_path(List *list, uint16_t screen_width, uint16_t path_position);
