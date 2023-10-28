#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

struct PathAndExt {
    std::string path;
    std::string ext;
};

class Config {
private:
    std::string configPath;

    std::vector<PathAndExt> pathAndExtList;

    std::vector<PathAndExt>::iterator curIterator;

    bool configReaded = false;

public:
    ~Config() = default;

    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    void setConfigPath(const std::string& configPath);

    bool readConfig();

    bool next();

    std::string getPath() const;

    std::string getExt() const ;

    bool isConfigReaded() const;
    
    unsigned getSleepDuration() const;

private:
    unsigned sleepDuration = 1;

    Config() = default;

    Config(Config& other) = delete;
    
    void operator=(const Config& other) = delete;
};