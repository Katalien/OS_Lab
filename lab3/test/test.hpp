#pragma once

#include <vector>
#include <map>
#include "../structure/set.hpp"
#include "../structure/fine_grained_set.hpp"

enum TestType{
    READ,
    WRITE,
    GENERAL
};

enum DataType{
        RAND,
        FIX
};

class TestArgs{
    public:
        Set* set = nullptr;
        std::vector<int>* data = nullptr;
        std::vector<int>* checkData = nullptr;
        pthread_mutex_t thread;
};

class Args{
    public:
        int readers;
        int readings;
        int writers;
        int writings;
};

enum Result{
    SUCCESS,
    FAIL
};

class Test{
public:
    static Result runTests(int numThreads, int numElements);

private:

    static int const repeats = 10;
    static int const attempts = 20;

    Test();
    static std::map<int, std::vector<int>> generateData(DataType type, int numThreads, int numElements);

    static Result testReaders( DataType dataType, int numThreads, int numElements);
    static void* testRead(void* args);

    static Result testWriters( DataType dataType, int numThreads, int numElements);
    static void* testWrite(void* args);

    static Result testGeneral( DataType dataType, int numThreads, int numElements);

    static TestArgs* createArgs(std::map<int, std::vector<int>> data, int index, Set* set, TestType testType);
    static bool checkSet(Set* set, std::vector<int>& vector);

    // 

 
    
    static bool checkGeneralTest(std::vector<std::vector<int>> data);
    static void* testReadWrite(void* args);
   
    // static std::vector<Args> getNumsAllTest(int numReaders, int numReadings, int numWriters, int numWritings, int readers, int readings, int writers, int writings);
    // static std::vector<Args> withoutRepeat(std::vector<Args> args);
    static std::map<int, std::vector<int>> reformatData(std::map<int, std::vector<int>>, int readers, int readings);
};