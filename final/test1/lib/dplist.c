/**
 * \author Yuhan Sun
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list 

#ifdef DEBUG
#define DEBUG_PRINTF(...) 									                                        \
        do {											                                            \
            fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	    \
            fprintf(stderr,__VA_ARGS__);								                            \
            fflush(stderr);                                                                         \
                } while(0)
#else
#define DEBUG_PRINTF(...) (void)0
#endif


#define DPLIST_ERR_HANDLER(condition, err_code)                         \
    do {                                                                \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");      \
            assert(!(condition));                                       \
        } while(0)


/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};



dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element) {

    //TODO: add your code here
    if(*list==NULL) return;
    if((*list)->head == NULL)
    {
        free((*list)->head);
        *list = NULL;
        free(*list);
        return;
    }
    dplist_node_t* list_node = (*list)->head;
    dplist_node_t* list_next_node;
    while(list_node->next!=NULL)
    {
        list_next_node = list_node->next;
        if(free_element)
        (*list)->element_free(&list_node->element);
        free(list_node);
        list_node = list_next_node;
    }
    if(free_element)
    (*list)->element_free(&list_node->element);
    free(list_node);
    *list = NULL;
    free(*list);
    return;

}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {

    //TODO: add your code here
    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;

    list_node = malloc(sizeof(dplist_node_t));
    DPLIST_ERR_HANDLER(list_node == NULL, DPLIST_MEMORY_ERROR);
    if(insert_copy == true)
    {
        list_node->element = list->element_copy(element);
    }
    else
    {
        list_node->element = element;
    }

    // pointer drawing breakpoint
    if (list->head == NULL) { // covers case 1
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
        // pointer drawing breakpoint
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        // pointer drawing breakpoint
        if (index < dpl_size(list)) { // covers case 4
            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            ref_at_index->prev->next = list_node;
            ref_at_index->prev = list_node;
            // pointer drawing breakpoint
        } else { // covers case 3
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;

}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {

    //TODO: add your code here
    if(list == NULL)
    {
        return NULL;
    }
    if(list->head == NULL)
    {
        return list;
    }
    dplist_node_t* list_node = list->head;
    if(index <= 0 )
    {
        if(free_element)
        list->element_free(&list_node->element);
        if(list_node->next != NULL)
        {
            list_node->next->prev=NULL;
            list->head = list->head->next;
        }
            
        else 
        {
           list->head = NULL;
        }
        free(list_node);
        return list;
    }
    else if(index >= dpl_size(list))
    {
        list_node = dpl_get_reference_at_index(list,dpl_size(list));
        if(free_element)
        list->element_free(&list_node->element);
        list_node = list_node->prev;
        (list_node -> next) = NULL;
        free(list_node -> next);
        list_node->next = NULL;
        
    }   
    else
    {
        list_node = dpl_get_reference_at_index(list,index);
        if(free_element)
        list->element_free(&list_node->element);
        list_node->prev->next = list_node->next;
        list_node->next->prev = list_node->prev;
        list_node = NULL;
        free(list_node);
        
    }
    return list;
}

int dpl_size(dplist_t *list) {

    //TODO: add your code here
    if(list == NULL)
    {
        return -1;
    }
    else
    {
        int counter = 0;
        if(list->head == NULL)
        {
            return counter;
        }
        dplist_node_t* list_node = list->head;
        while(list_node->next != NULL)
        {
            list_node = list_node -> next;
            counter = counter + 1;
        }
        counter++;
        return counter;
    }

}

void *dpl_get_element_at_index(dplist_t *list, int index) {

    //TODO: add your code here
    if(list == NULL || list->head == NULL)
    {
        return 0; 
    }
    dplist_node_t* list_node;
    list_node = list->head;
    if(index <= 0)
    {
        
    }
    else if(index >= dpl_size(list))
    {
        list_node = dpl_get_reference_at_index(list,dpl_size(list));
    }
    else
    {
        list_node = dpl_get_reference_at_index(list,index);
    }
    return list_node->element;

}

int dpl_get_index_of_element(dplist_t *list, void *element) {

    //TODO: add your code here
    if(list == NULL || list->head == NULL)
    {
        return -1;
    }
    dplist_node_t* list_node = list->head;
    int found = -1; 
    int counter = -1; 
    while(found==-1)
    {
        found = list->element_compare(list_node->element,element);
        if(list_node->next!=NULL)
        {list_node = list_node->next;}
        else break;
        counter = counter + 1;
    }
    if(found==0)
    {
        return counter;
    }
    else
    {
        return -1;
    }

}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {

    //TODO: add your code here
    int count;
    dplist_node_t *dummy;
    //DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    if(list==NULL) return NULL;
    if (list->head == NULL) return NULL;
    for (dummy = list->head, count = 0; dummy->next != NULL; dummy = dummy->next, count++) {
        if (count >= index) return dummy;
    }
    return dummy;

}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {

    //TODO: add your code here
    if(list == NULL || list->head == NULL|| reference == NULL)
    {
        return NULL;
    }
    dplist_node_t* list_node = list->head;
    while (list_node != reference && list_node->next != NULL)
    {
        list_node = list_node->next;
    }
    if(list_node == reference)
    {
        return list_node->element;
    }
    else
    {
        return NULL;
    }
}


