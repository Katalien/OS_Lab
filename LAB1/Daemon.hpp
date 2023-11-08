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

class Daemon {
public:
    Daemon(Daemon const&) = delete;
    Daemon& operator = (Daemon const&) = delete;
    Daemon(Daemon&&) = delete;
    Daemon& operator = (Daemon&&) = delete;

    static Daemon& getInstance() {
        static Daemon instance;
        return instance;
    }

    void createDaemon(std::filesystem::path &configPath);

    void run();

private:
    std::chrono::seconds sleepTime = std::chrono::seconds(20);

    void createLogFiles();

    const std::filesystem::path PID_PATH{"/var/run/daemon.pid"};

    std::filesystem::path configPath;

    std::filesystem::path directoryPath;

    bool isTerminated = false;

    bool isRunning = false;

    bool pathExist(const std::string& path) const;

    void destructOldPid();

    void createPid();

    Daemon() = default;

};