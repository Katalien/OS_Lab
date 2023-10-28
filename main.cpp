#include <iostream>
#include <syslog.h>
#include "Daemon.hpp"
#include "Config.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "You must enter an argument" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string path = "./config.txt";
    Daemon::getInstance().createDaemon(argv[1]);
    Daemon::getInstance().run();
    return EXIT_SUCCESS;
}
