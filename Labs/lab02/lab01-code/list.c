#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"

void list_init(list_t *list) {
    list->head = NULL;
    list->size = 0;
}

void list_add(list_t *list, const char* data) {

    assert(data != NULL);
    
    if (list->head == NULL) {
        list->head = (node_t*) malloc(sizeof(node_t));

        assert(list->head != NULL);
        strcpy(list->head->data, data);
    } else {
        node_t * current = list->head;
        
        while (current->next != NULL)
            current = current->next; 

        current->next = (node_t*) malloc(sizeof(node_t));
        
        assert(current->next != NULL);
        strcpy(current->next->data, data);
    }  

    list->size++;
}

int list_size(const list_t *list) {
    
    return list->size;
}

char *list_get(const list_t *list, int index) {
    
    if (list->head == NULL || index < 0 || index > list_size(list))
        return NULL;

    node_t * current = list->head;
    
    for (int i = 0; i < index; i++)
        current = current->next;

    return current->data;
}

void list_clear(list_t* list) {

    while (list->head != NULL) {
        node_t *temp = list->head;
        list->head = list->head->next;
        list->size--;
        free(temp);
    }
}

int list_contains(const list_t *list, const char *query) {
    
    assert(query != NULL);

    if (list->head != NULL) {
        node_t *current = list->head;

        while (current != NULL) {
            if (!strcmp(current->data, query)) {
                return 1;
            }
                
            current = current->next;
        }
    }
    return 0;
}

void list_print(const list_t *list) {
    int i = 0;
    node_t *current = list->head;
    while (current != NULL) {
        printf("%d: %s\n", i, current->data);
        current = current->next;
        i++;
    }
}
