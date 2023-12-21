#include <iostream>
#include <random>
#include <thread>
#include "test.hpp"

Result Test::runTests(int numThreads, int numElements) {
    std::cout << "FIXED DATA TESTS:" << std::endl;
    if (Test::testReaders(DataType::FIX, numThreads, numElements) == Result::FAIL){
        return Result::FAIL;
    }
    if (Test::testWriters(DataType::FIX, numThreads, numElements) == Result::FAIL){
        return Result::FAIL;
    }
    if (Test::testGeneral(DataType::FIX, numThreads, numElements) == Result::FAIL){
        return Result::FAIL;
    }
    std::cout << "RANDOM DATA TESTS:" << std::endl;
    if (Test::testReaders(DataType::RAND, numThreads, numElements) == Result::FAIL){
        return Result::FAIL;
    }
    if (Test::testWriters(DataType::RAND,numThreads, numElements) == Result::FAIL){
        return Result::FAIL;
    }
    if (Test::testGeneral(DataType::RAND, numThreads, numElements) == Result::FAIL){
        return Result::FAIL;
    }
    return Result::SUCCESS;
}

Result Test::testReaders(DataType dataType, int numThreads, int numElements) {
    Set* set = new FineGrainedSet();
    std::vector <pthread_t> threads(numThreads);
    std::cout << "TestReaders: start test" << std::endl;
    long time = 0;
    for (int i = 0; i < repeats; ++i) {
        std::map<int, std::vector<int>> data = generateData(dataType, numThreads, numElements);
        for (auto elem: data) {
            int size = (int)elem.second.size();
            for (int i = 0; i < size; ++i) {
                set->add(elem.second[i]);
            }
        }
        auto start_time = clock();
        for (int j = 0; j < numThreads; ++j) {
            TestArgs* readArgs = createArgs(data, j, set, TestType::READ);
            pthread_create(&threads[j], nullptr, testRead, readArgs);
        }
        for (int j = 0; j < numThreads; ++j) {
            pthread_join(threads[j], nullptr);
        }
        auto end_time = clock();
        if (!set->isEmpty()) {
            std::cout << "TestReaders: Set is not empty. Error" << std::endl;
            return Result::FAIL;
        }
        time += end_time - start_time;
    }
    double average_time = (double) time / repeats;
    std::cout << "TestReaders Success!: Average time: " << average_time << std::endl << std::endl;
    return Result::SUCCESS;
}

void* Test::testRead(void *args) {
    auto *data = (TestArgs*) args;
    for (int i = 0; i < (int)data->data->size(); i++) {
        for (int j = 0; j < attempts; j++) {
            if (data->set->remove((*data->data)[i])) {
                break;
            }
        }
    }
    return nullptr;
}

std::map<int, std::vector<int>> Test::generateData(DataType type,int numThreads, int numElements) {
    std::map<int, std::vector<int>> result = {};
    std::random_device rd;
    std::mt19937 mersenne(rd());
    for (int i = 0; i < numThreads; i++) {
        std::vector<int> vec;
        vec.reserve(numElements);  
        if (type == DataType::RAND){
            for (int j = 0; j < numElements; j++) {
                vec.push_back(static_cast<int>(mersenne()) % 100000);
            }
        }
        else if (type == DataType::FIX){
            for (int j = 0; j < numElements; j++) {
                vec.push_back(i * numElements + j);
            }
        }
        result[i] = std::move(vec);  
    }
    return result;
}


Result Test::testWriters( DataType dataType, int numThreads, int numElements) {
    std::vector <pthread_t> threads(numThreads);
    std::cout << "TestWriters: start" << std::endl;
    long time = 0;

    for (int j = 0; j < repeats; j++) {
        Set *set = new FineGrainedSet();
        auto data = generateData(dataType, numThreads, numElements);

        auto start_time = clock();
        for (int i = 0; i < numThreads; i++) {
            TestArgs* writeStruct = createArgs(data, i, set, TestType::WRITE);
            pthread_create(&threads[i], nullptr, testWrite, writeStruct);
        }
        for (int i = 0; i < numThreads; i++) {
            pthread_join(threads[i], nullptr);
        }
        auto end_time = clock();

        time += end_time - start_time;

        for (int i = 0; i < numThreads; i++) {
            if (!checkSet(set, data[i])) {
                std::cout << "TestWriters: checkSet failed" << std::endl << std::endl;
                return Result::FAIL;
            }
        }
    }
    double average_time = (double) time / repeats;
    std::cout << "TestWriters Success!  Average time: " << average_time << std::endl << std::endl;
    return Result::SUCCESS;
}

void *Test::testWrite(void *args) {
    auto *data = (TestArgs*) args;
    for (int i = 0; i < (int)data->data->size(); i++) {
        for (int j = 0; j < attempts; j++) {
            if (data->set->add((*data->data)[i])) {
                break;
            }
        }
    }
    return nullptr;
}

TestArgs *Test::createArgs(std::map<int, std::vector<int>> data, int index, Set *set, TestType testType) {
    auto args = new TestArgs();
    args->set = set;
    args->data = new std::vector<int>(data[index].size());
    for (int i = 0; i < (int)data[index].size(); ++i) {
        (*args->data)[i] = data[index][i];
    }
    if (testType == TestType::GENERAL){
        args->checkData = new std::vector<int>(data[index].size());
    }
    return args;
}

bool Test::checkSet(Set *set, std::vector<int> &vector) {
    for (int &i: vector) {
        if (!set->contains(i)) {
            std::cout << "checkSet error: element is not in the set" << std::endl;
            return false;
        }
    }
    return true;
}

Result Test::testGeneral(DataType dataType, int numThreads, int numElements) {
    std::cout << "GeneralTest: start" << std::endl;
    long time = 0;
    int numReaders = numThreads/2;
    int numWriters = numThreads - numReaders;
    int numReading = numElements/2;
    int numWriting = numElements - numReading;
   
    std::vector <Args> possibleNums = getNumsAllTest(numReaders, numReading, numWriters, numWriting, 1, 1, 1, 1);
    possibleNums = deleteRepeats(possibleNums);

    for (int i = 0; i < repeats; i++) {

        auto start_time = clock();
        for (int j = 0; j < (int) possibleNums.size(); j++) {
            int readers = possibleNums[j].readers;
            int readings = possibleNums[j].readings;
            int writers = possibleNums[j].writers;
            int writings = possibleNums[j].writings;

            std::vector <pthread_t> readThreads(readers);
            std::vector <pthread_t> writeThreads(writers);

            Set *set = new FineGrainedSet();;
            auto dataWrite = generateData(dataType, writers, writings);
            auto dataRead = reformatData(dataWrite, readers, readings);

            for (int q = 0; q < writers; q++) {
                TestArgs *writeStruct = createArgs(dataWrite, q, set, TestType::WRITE);
                pthread_create(&writeThreads[q], nullptr, testWrite, writeStruct);
            }

            for (int q = 0; q < writers; q++) {
                pthread_join(writeThreads[q], nullptr);
            }

            for (int q = 0; q < readers; q++) {
                TestArgs *rwStruct = createArgs(dataRead, q, set, TestType::GENERAL);
                pthread_create(&readThreads[q], nullptr, testReadWrite, rwStruct);
            }

            std::vector <std::vector<int>> check(readers);

            for (int q = 0; q < readers; q++) {
                void* temp = nullptr;
                pthread_join(readThreads[q], &temp);
                std::vector<int>* vec = (std::vector<int>*)((temp));
                check[q] = std::vector<int>(readings);
                for(auto num = 0; num < static_cast<int>(vec->size()); num++){
                    check[q].at(num) = vec->at(num);
                }
            }

            if (!checkGeneralTest(check)) {
                std::cout << "GeneralTest: invalid result at iteration" << std::endl;
                return Result::FAIL;
            }
        }
        auto end_time = clock();

        time += end_time - start_time;
    }

    double average_time = (double) time / (repeats * static_cast<int>(possibleNums.size()));
    std::cout << "GeneralTest Success! Average time: " << average_time << std::endl;
    return Result::SUCCESS;
}


bool Test::checkGeneralTest(std::vector <std::vector<int>> data) {
    for (int i = 0; i < (int)data.size(); i++) {
        for (int j = 0; j < (int)data[i].size(); j++) {
            if (data[i][j] != 1) {
                return false;
            }
        }
    }
    return true;
}

void *Test::testReadWrite(void *args) {
    auto *data = (TestArgs*) args;
    for (int i = 0; i < (int)data->data->size(); i++) {
        for (int j = 0; j < attempts; j++) {
            if (data->set->remove((*data->data)[i])) {
                pthread_mutex_lock(&data->thread);
                data->checkData->at(i)++;
                pthread_mutex_unlock(&data->thread);
                break;
            }
        }
    }
    return data->checkData;
}


std::map<int, std::vector<int>> Test::reformatData(std::map<int, std::vector<int>> data, int readers, int readings) {
    std::map<int, std::vector<int>> result{};
    std::vector<int> vector(0);
    for (int i = 0; i < (int)data.size(); i++) {
        for (int j = 0; j < (int)data[i].size(); j++) {
            vector.push_back(data[i][j]);
        }
    }
    for (int i = 0; i < readers; i++) {
        for (int j = 0; j < readings; j++) {
            result[i].push_back(vector[i * readings + j]);
        }
    }
    return result;
}

std::vector <Args> Test::getNumsAllTest(int numReaders, int numReadings, int numWriters, int numWritings, int readers, int readings,
                     int writers, int writings) {
    std::vector <Args> result;
    if (readers * readings == writers * writings) {
        struct Args args = {readers, readings, writers, writings};
        result.push_back(args);
    }
    if (readers < numReaders) {
        std::vector <Args> r = getNumsAllTest(numReaders, numReadings, numWriters, numWritings, readers + 1,
                                                    readings,
                                                    writers, writings);
        for (int i = 0; i < (int)r.size(); i++) {
            result.push_back(r[i]);
        }
    }
    if (readings < numReadings) {
        std::vector <Args> r = getNumsAllTest(numReaders, numReadings, numWriters, numWritings, readers,
                                                    readings + 1,
                                                    writers, writings);
        for (int i = 0; i < (int)r.size(); i++) {
            result.push_back(r[i]);
        }
    }
    if (writers < numWriters) {
        std::vector <Args> r = getNumsAllTest(numReaders, numReadings, numWriters, numWritings, readers, readings,
                                                    writers + 1, writings);
        for (int i = 0; i < (int)r.size(); i++) {
            result.push_back(r[i]);
        }
    }
    if (writings < numWritings) {
        std::vector <Args> r = getNumsAllTest(numReaders, numReadings, numWriters, numWritings, readers, readings,
                                                    writers, writings + 1);
        for (int i = 0; i < (int)r.size(); i++) {
            result.push_back(r[i]);
        }
    }
    return result;
}

std::vector <Args> Test::deleteRepeats(std::vector <Args> args) {
    std::vector <Args> result(0);
    for (int i = 0; i < (int)args.size(); i++) {
        bool have = false;
        for (int j = 0; j < (int)result.size(); j++) {
            if (result[j].readers == args[i].readers && result[j].readings == args[i].readings &&
                result[j].writers == args[i].writers && result[j].writings == args[i].writings) {
                have = true;
                break;
            }
        }
        if (!have) {
            result.push_back(args[i]);
        }
    }
    return result;
}