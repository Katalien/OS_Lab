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
    // std::cout << "TestReaders: start test" << std::endl;
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
    // std::cout << "TestWriters: start" << std::endl;
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
    std::cout << "Test General: start" << std::endl;
    
    if (numThreads > (int)std::thread::hardware_concurrency()){
        numThreads = (int)std::thread::hardware_concurrency();
        std::cout << "MAX THREADS: " << numThreads << std::endl << std::endl;
    }
    long time = 0;
    for (int i = 0; i < repeats; i++) {
        auto start_time = clock();
        for (size_t writers_num = 1, readers_num = numThreads - 1; readers_num > 0; ++writers_num, --readers_num) {
            for (int reading = 1; reading < numElements; ++reading){
                int writing = readers_num * reading/writers_num;
                std::vector<pthread_t> writeThreads(writers_num);
                std::vector<pthread_t> readThreads(readers_num);

                Set *set = new FineGrainedSet();
                auto dataWrite = generateData(dataType, writers_num, reading);
                auto dataRead = reformatData(dataWrite, readers_num, writing);

                for (int q = 0; q < (int)writers_num; q++) {
                    TestArgs *writeStruct = createArgs(dataWrite, q, set, TestType::WRITE);
                    pthread_create(&writeThreads[q], nullptr, testWrite, writeStruct);
                }

                for (int q = 0; q < (int)writers_num; q++) {
                    pthread_join(writeThreads[q], nullptr);
                }

                for (int q = 0; q < (int)readers_num; q++) {
                    TestArgs *rwStruct = createArgs(dataRead, q, set, TestType::GENERAL);
                    pthread_create(&readThreads[q], nullptr, testReadWrite, rwStruct);
                }
                std::vector <std::vector<int>> check(readers_num);

                for (int q = 0; q < (int)readers_num; q++) {
                    void* temp = nullptr;
                    pthread_join(readThreads[q], &temp);
                    std::vector<int>* vec = (std::vector<int>*)((temp));
                    std::cout << numElements<<std::endl;
                    check[q] = std::vector<int>(reading);
                    for(auto num = 0; num < static_cast<int>(vec->size()); num++){
                        check[q].at(num) = vec->at(num);
                    }
                }

                if (!checkGeneralTest(check)) {
                    std::cout << "TestGeneral: checkReadWriteTest failed" << std::endl;
                    return Result::FAIL;
                }
            }
        }
        auto end_time = clock();

        time += end_time - start_time;
    }
    double average_time = (double) time / (repeats * numThreads);
    std::cout << "TestGeneral Success! Average time: " << average_time << std::endl;
    return Result::SUCCESS;
}

bool Test::checkGeneralTest(std::vector <std::vector<int>> data) {
    for (int i = 0; i < (int)data.size(); i++) {
        for (int j = 0; j < (int)data[i].size(); j++) {
            std::cout << data[i][j] << " ";
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


// std::vector <Test::Args> Test::withoutRepeat(std::vector <Args> args) {
//     std::vector <Test::Args> result(0);
//     for (int i = 0; i < (int)args.size(); i++) {
//         bool have = false;
//         for (int j = 0; j < (int)result.size(); j++) {
//             if (result[j].readers == args[i].readers && result[j].readings == args[i].readings &&
//                 result[j].writers == args[i].writers && result[j].writings == args[i].writings) {
//                 have = true;
//                 break;
//             }
//         }
//         if (!have) {
//             result.push_back(args[i]);
//         }
//     }
//     return result;
// }