#include <iostream>
#include <thread>
#include "../test/test.hpp"

int main(int argc, char* argv[]) {
    if(argc != 3){
        std::cout << std::thread::hardware_concurrency() << std::endl;
        return 0;
        // std::cout << "Not correct amount of arguments. Should be 2: amount of threads and amount od elements" << std::endl;
        // return 1;
        // for (int num_t = 1; num_t < 8; ++num_t){
        //     for (int num_el = 1; num_el < 10; ++num_el){
        //          Result testsResult = Test::runTests(num_t, num_el);
        //          if (testsResult == Result::SUCCESS){
        //             std::cout<< "ALL TESTS HAVE BEEN RUN SUCCESSFULLY"<< std::endl;
        //         }
        //         else{
        //             std::cout<<"SOME TEST HAVE FAILED" << std::endl;
        //             std::cout << "Threads = " << num_t <<std::endl;
        //             std::cout << "Elements = " << num_el << std::endl << std::endl;
                
        //         }
        //     }
        // }
    }
    int numThreads = std::atoi(argv[1]);
    int numElements = std::atoi(argv[2]);
    std::cout <<"Amount of threads = " << numThreads << std::endl;
    std::cout << "Amount of elements = " << numElements << std::endl <<std::endl;
    try{
        Result testsResult = Test::runTests(numThreads, numElements);
        if (testsResult == Result::SUCCESS){
            std::cout<< "ALL TESTS HAVE BEEN RUN SUCCESSFULLY"<< std::endl;
        }
        else{
            std::cout<<"SOME TEST HAVE FAILED" << std::endl;
        }
    }
    catch (...){
        std::cout << "ERROR: can't run tests" << std::endl;
        return 1;
    }

}