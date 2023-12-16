#pragma once

class Set {
public:
    virtual bool add(const int& item) = 0;
    virtual bool remove(const int& item) = 0;
    virtual bool contains(const int& item) = 0;
    virtual bool isEmpty() = 0;
};