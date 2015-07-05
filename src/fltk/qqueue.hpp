#pragma once

#include "../../liblick/llist.h"
template<typename T>
class qqueue {
    public:
        qqueue();
        bool empty();
        int size();

        void push(T n);
        void pop();
        T front();
    private:
        int lst_size;
        node_t *lst;
        node_t *last;
};

template<typename T>
qqueue<T>::qqueue() {
    lst_size = 0;
    lst = NULL;
    last = NULL;
}


template<typename T>
bool qqueue<T>::empty() {
    return lst_size == 0;
}

template<typename T>
int qqueue<T>::size() {
    return lst_size;
}


template<typename T>
T qqueue<T>::front() {
    return (T)lst->val;
}

template<typename T>
void qqueue<T>::push(T n) {
    node_t *node = new_node(n, NULL);
    if(last)
        last->next = node;
    else
        lst = node;
    last = node;

    ++lst_size;
}

template<typename T>
void qqueue<T>::pop() {
    node_t *next_head = lst->next;
    free(lst);
    lst = next_head;
    if(!lst)
        last = NULL;

    --lst_size;
}
