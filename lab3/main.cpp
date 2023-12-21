#include <iostream>
#include <thread>
#include "../test/test.hpp"

int main(int argc, char* argv[]) {
    if(argc != 3){
        std::cout << "Incorrect number of arguments. Should be 2: amount of threads and amount of elements." << std::endl;
        return 0;   
    }
    int numThreads = std::atoi(argv[1]);
    int numElements = std::atoi(argv[2]);
    std::cout <<"Amount of threads = " << numThreads << std::endl;
    std::cout << "Amount of elements = " << numElements << std::endl <<std::endl;
    try{
        Result testsResult = Test::runTests(numThreads, numElements);
        if (testsResult == Result::SUCCESS){
            std::cout<< std::endl << "ALL TESTS HAVE BEEN RUN SUCCESSFULLY"<< std::endl;
        }
        else{
            std::cout<< std::endl << "SOME TEST HAVE FAILED" << std::endl;
        }
    }
    catch (...){
        std::cout << "ERROR: can't run tests" << std::endl;
        return 1;
    }

}