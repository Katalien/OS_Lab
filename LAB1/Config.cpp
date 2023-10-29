#include "Config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>


void Config::setConfigPath(const std::string& configPath) {
    this->configPath = configPath;
}
    
bool Config::readConfig() {
    pathAndExtList.clear();
    configReaded = false;
    std::ifstream file;
    file.open(configPath);
    if (!file)
        return false;
    try {
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::vector<std::string> tokens;
            std::string token;
            while (std::getline(ss, token, ' ')) {
                tokens.push_back(token);
            }
            PathAndExt cur;
            cur.path = tokens[0];
            std::filesystem::path currentPath = tokens[0];
            std::filesystem::path absolutePath = std::filesystem::absolute(tokens[0]);
            cur.path = absolutePath;
            cur.ext = tokens[1];
            cur.ext = cur.ext.substr(0, cur.ext.size()-1);;
            pathAndExtList.push_back(cur);
        }
    }
    catch (...) {
        return false;
    }

    if (pathAndExtList.empty()) {
        return false;
    }
    curIterator = pathAndExtList.begin();
    configReaded = true;
    return true;
}

bool Config::next() {
    curIterator++;
    if (curIterator == pathAndExtList.end()) {
        curIterator = pathAndExtList.begin();
        return false;
    }
    return true;
}

std::string Config::getPath() const {
    return (*curIterator).path;
}

std::string Config::getExt() const {
    return (*curIterator).ext;
}

bool Config::isConfigReaded() const {
    return configReaded;
}

unsigned Config::getSleepDuration() const {
    return sleepDuration;
}