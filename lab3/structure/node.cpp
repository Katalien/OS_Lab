#include <iostream>
#include "node.hpp"

Node::Node(int value, Node *nextNode): nextNode(nextNode), value(value) {
    thread = PTHREAD_MUTEX_INITIALIZER;
}

Node::~Node(){
    pthread_mutex_destroy(&thread);
}

void Node::setNextNode(Node *node) {
    nextNode = node;
}

Node* Node::getNextNode() {
    return nextNode;
}

int Node::getValue() const {
    return value;
}

void Node::lock() {
    if(pthread_mutex_lock(&this->thread) != 0){
        std::cout << "ERROR: can't lock mutex" << std::endl;
    }
}

void Node::unlock() {
    pthread_mutex_unlock(&this->thread);
}