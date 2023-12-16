#include <cstdint>
#include "fine_grained_set.hpp"

FineGrainedSet::FineGrainedSet(){
    int value = INT32_MIN;
    Node* nextNode = new Node(value, nullptr);
    head = new Node(value, nextNode);
}

bool FineGrainedSet::add(const int& addedValue) {
    head->lock();
    Node* prev = head;
    Node* current = head->getNextNode();
    current->lock();

    while(current->getValue() < addedValue && current->getNextNode() != nullptr){
        prev->unlock();
        prev = current;
        current = current->getNextNode();
        current->lock();
    }

    if(current->getValue() == addedValue){
        prev->unlock();
        current->unlock();
        return false;
    }

    else {
        Node* newNode = new Node(addedValue, current);
        if(newNode == nullptr){
            prev->unlock();
            current->unlock();
            return false;
        }
        prev->setNextNode(newNode);
        prev->unlock();
        current->unlock();
        return true;
    }
}

bool FineGrainedSet::remove(const int& value) {
    head->lock();
    Node* prev = head;
    Node* current = prev->getNextNode();
    current->lock();

    while(value > current->getValue() && current->getNextNode() != nullptr ){
        prev->unlock();
        prev = current;
        current = current->getNextNode();
        current->lock();
    }

    if(current->getValue() == value){
        Node* tmp = current;
        Node* next = current->getNextNode();
        prev->setNextNode(next);
        delete tmp;
        prev->unlock();
        current->unlock();
        return true;
    }

    else{
        prev->unlock();
        current->unlock();
        return false;
    }
}

bool FineGrainedSet::contains(const int& value) {
    head->lock();
    Node* prev = head;
    Node* current = prev->getNextNode();
    current->lock();

    while(value > current->getValue() && current->getNextNode() != nullptr ){
        prev->unlock();
        prev = current;
        current = current->getNextNode();
        current->lock();
    }

    if(current->getValue() == value){
        prev->unlock();
        current->unlock();
        return true;
    }
    else{
        prev->unlock();
        current->unlock();
        return false;
    }
}

bool FineGrainedSet::isEmpty() {
    return head->getNextNode()->getNextNode() == nullptr;
}