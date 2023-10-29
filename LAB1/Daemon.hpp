#pragma once
#include <vector>
#include <regex>
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <fstream>
#include <filesystem>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/stat.h>
#include <chrono>
#include <ctime>
#include "Config.hpp"

struct Data {
    std::string directory; 
    std::string file;      

    Data(std::string dir, std::string fl) :
        directory(dir), file(fl) {}

    Data(const Data&) = default;
    Data& operator=(const Data&) = default;
    Data(Data&&) = default;
    Data& operator = (Data&&) = default;
};

class Daemon {
public:
    Daemon(Daemon const&) = delete;
    Daemon& operator = (Daemon const&) = delete;
    Daemon(Daemon&&) = delete;
    Daemon& operator = (Daemon&&) = delete;
    Daemon() = default;

    static Daemon& getInstance() {
        static Daemon instance;
        return instance;
    }

    void stop() {
        isRunning = false;  
    }

    void createDaemon(std::string configPath);

    void run();

    void createLogFiles();

    void signalHandler(int signal);

private:
    std::chrono::seconds sleepTime = std::chrono::seconds(20);

    const std::filesystem::path PID_PATH = std::filesystem::path{ "/var/run/daemon.pid" };

    std::filesystem::path configPath;

    std::filesystem::path directoryPath;

    bool isTerminated = false;
  
    std::vector<Data> data;

    bool isRunning = false;

    bool pathExist(const std::string& path) const;

    void destructOldPid();

    void createPid();

};