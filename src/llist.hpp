#ifndef LIST_H
#define LIST_H

namespace my_server {

struct llist_node {
	void *data;
	struct llist_node *next;
};

class Llist {

public:

Llist();
~Llist();

/**
 * Insert at the head of a linked list
 */
void *insert(void *data);
/**
 * Append to the end of a list
 */
void *append(void *data);
/**
 * Return the last element in a list
 */
void *head();
/**
 * Return the last element in a list
 */
void *tail();
/**
 * Return the number of elements in the list
 */
int count();
/**
 * Allocates and returns a new NULL-terminated array of pointers to data
 * elements in the list.
 *
 * NOTE: This is a read-only array! Consider it an array view onto the linked
 * list.
 */
void **array_get() const;
/**
 * For each item in the list run a function
 */
template<typename F>
void foreach(F f, void *arg)
{
    llist_node* p = _head;
    llist_node* next;

    while(p)
    {
        next = p->next;
        f(p->data, arg);
        p = next;
    }
}
/**
 * Find an element in the list
 *
 * cmpfn should return 0 if the comparison to this node's data is equal.
 */
template<typename Compare>
void *find(void *data, Compare cmp)
{
    for (llist_node* n = _head; n != nullptr; n = n->next) {
        if (cmp(data, n->data) == 0) {
            return n->data;
        }
    }
    return nullptr;
}

/**
 * Delete an element in the list
 *
 * cmpfn should return 0 if the comparison to this node's data is equal.
 *
 * NOTE: does *not* free the data--it merely returns a pointer to it
 */
template<typename Compare>
void *l_delete(void *data, Compare cmp)
{
    struct llist_node *current = _head;
    struct llist_node *prev = nullptr;

    while (current)
    {
        if (cmp(data, current->data) == 0)
        {
            void* found_data = current->data;
            if (!prev) //head
            { 
                _head = current->next;
            } else { // non-head
                prev->next = current->next;
            }
            delete current;
            _count--;
            return found_data;
        }
        prev = current;
        current = current->next;
    }

    return nullptr;
}

/**
 * Frees an array allocated with llist_array_get().
 *
 * NOTE: This does not modify the linked list or its data.
 */
static inline void array_free(void** a) {
    delete[] a;
}

private:

llist_node* _head;
int _count;

};

}


#endif