#include <new> 

#include "llist.hpp"

namespace my_server {

Llist::Llist()
    :_head(nullptr),
     _count(0) {}

Llist::~Llist()
{
    llist_node* current = _head;
    while (current)
    {
        llist_node* next = current->next;
        delete current;
        current = next;
    }
}

void *Llist::insert(void *data)
{
    llist_node* n = new (std::nothrow) llist_node;
    if (!n) {
        return nullptr;
    }
    n->data = data;
	n->next = _head;
	_head = n;
    _count++;

    return data;
}

void *Llist::append(void *data)
{
    struct llist_node *tail = _head;

    // If list is empty, just insert
	if (tail == nullptr) {
		return insert(data);
	}

    llist_node* n = new (std::nothrow) llist_node;
    if (!n) {
		return nullptr;
	}

    while (tail->next != nullptr) {
		tail = tail->next;
	}

    n->data = data;
    tail->next = n;
    _count++;

    return data;
}

void *Llist::head()
{
    if (!_head)
    {
        return nullptr;
    }
    return _head->data;
}

void *Llist::tail()
{
    struct llist_node *tail = _head;
    if (!tail) {
		return nullptr;
	}

    while (tail->next != nullptr) {
		tail = tail->next;
	}

    return tail->data;
}

std::size_t Llist::count()
{
    return _count;
}

void **Llist::array_get() const
{
    if (!_head)
    {
        return nullptr;
    }
    // Allocate an array with an extra element for the null terminator.
    void** a = new void*[_count + 1];
    int i = 0;
    for (llist_node* n = _head; n; n = n->next) {
        a[i++] = n->data;
    }
    a[i] = nullptr;
    return a;
}

}