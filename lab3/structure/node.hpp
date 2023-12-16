#pragma once
#include <pthread.h>

class Node{
public:
    Node(int value, Node* nextNode);
    ~Node();

    void setNextNode(Node* node);
    Node* getNextNode();
    int getValue() const;
    void lock();
    void unlock();
    
private:
    Node* nextNode;
    int value;
    pthread_mutex_t thread;
};