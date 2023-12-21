#pragma once

#include "set.hpp"
#include "node.hpp"

class FineGrainedSet : public Set {
public:

    FineGrainedSet();
    ~FineGrainedSet();
    bool add(const int& value) override;
    bool remove(const int& value) override;
    bool contains(const int& value) override;
    bool isEmpty() override;

private:

    Node* head;
};