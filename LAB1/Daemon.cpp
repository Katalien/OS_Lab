#include <fcntl.h>
#include <sys/stat.h>
#include <csignal>
#include <ctime>
#include <fstream>
#include <thread>
#include <fstream>
#include "Daemon.hpp"
#include "Config.hpp"

void signalHandler(int signal) {
    switch (signal) {
    case SIGTERM:
        syslog(LOG_INFO, "Process terminated");
        Daemon::getInstance().stop();
        closelog();
        break;
    case SIGHUP:
        syslog(LOG_INFO, "Read config");
        if (!Config::getInstance().readConfig())
            syslog(LOG_INFO, "Config is not in the correct format");
        break;
    default:
        syslog(LOG_INFO, "Unknown signal found");
        break;
    }
}

void Daemon::createDaemon(std::filesystem::path &configPath) {
    openlog("Daemon", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Read config");
    Config::getInstance().setConfigPath(configPath);
    if (!Config::getInstance().readConfig()) {
        syslog(LOG_INFO, "Config file doesn't exists ot it is not in the correct format");
    }
    destructOldPid();
    createPid();
    std::signal(SIGHUP, signalHandler);
    std::signal(SIGTERM, signalHandler);
    isRunning = true;
 }

void Daemon::run() {
    while (isRunning) {
        if (Config::getInstance().isConfigReaded()) {
            syslog(LOG_INFO, "START RUN PROCESS");
            createLogFiles();
            sleep(Config::getInstance().getSleepDuration());
        }
        else {
            syslog(LOG_INFO, "Config not read");
        }
        std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
    }
}

void Daemon::createLogFiles() {
    syslog(LOG_INFO, "Start create log files");
    do {
        std::string path = Config::getInstance().getPath();
        std::string ext = Config::getInstance().getExt();

        std::string logFileName = path + "/log.txt"; 
        std::ofstream logFile(logFileName, std::ios::app);  

        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        logFile << "-----------------------------------------" << std::endl;
        logFile << "Current Time: " << std::ctime(&time);
        logFile << "Folder: " << path << std::endl;
        logFile << "Extension: " << ext << std::endl;

      if (pathExist(path)) {
        try {
                for (const auto& entry : std::filesystem::directory_iterator(path)) {
                            if (entry.path().extension() == ext && entry.path().filename() != "log.txt") {
                                logFile << "File: " << entry.path().filename() << ", Size: " << std::filesystem::file_size(entry.path()) << " bytes" << std::endl;
                            }
                    
                }
            }
            catch (const std::exception& e) {
                syslog(LOG_ERR, "%s", e.what());
            }
        }

    } while (Config::getInstance().next());
}



bool Daemon::pathExist(const std::string& path) const {
    bool exists = std::filesystem::exists(path);

    if (!exists) {
        syslog(LOG_INFO, "No such folder %s", path.c_str());
    }
    return exists;
}

void Daemon::destructOldPid() {
    syslog(LOG_INFO, "Destruct old pid");
    std::ifstream file;

    file.open(PID_PATH);
    if (!file) {
        syslog(LOG_INFO, "Can't check that old pid");
        return;
    }

    int pid;
    if (file >> pid){
        if (kill(pid, 0) == 0)
            kill(pid, SIGTERM);
    }

    file.close();
}
    
void Daemon::createPid() {
    syslog(LOG_INFO, "Fork");

    pid_t pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    if ((chdir("/")) < 0){
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Write new pid");
    std::ofstream file(PID_PATH);
    if (!file.is_open()){
        syslog(LOG_ERR, "Can't open pid file");
        exit(EXIT_FAILURE);
    }
    file << getpid();
    file.close();

    dup2(open("/dev/null", O_RDONLY), STDIN_FILENO);
    dup2(open("/dev/null", O_WRONLY), STDOUT_FILENO);
    dup2(open("/dev/null", O_WRONLY), STDERR_FILENO);
}
