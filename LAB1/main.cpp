#include <iostream>
#include <syslog.h>
#include "Daemon.hpp"
#include "Config.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "You must enter the path to config file" << std::endl;
        return EXIT_FAILURE;
    }
    std::string relPath = argv[1];
    std::filesystem::path absolutePath = std::filesystem::absolute(relPath);
    Daemon::getInstance().createDaemon(absolutePath);
    Daemon::getInstance().run();
    return EXIT_SUCCESS;

}