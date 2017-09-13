#include "ArgumentParser.hpp"
#include <iostream>
#include <algorithm>
#include <string>

ArgumentParser::ArgumentParser() {

}

void ArgumentParser::parse(int argc, char **argv) {
    for(int i = 1; i < argc; i++) {
        std::string arg(argv[i]);

        size_t equal_pos = arg.find("=");
        if(equal_pos != std::string::npos) {
            arg_map[arg.substr(0, equal_pos)] = arg.substr(equal_pos+1);
        }
        else {
            flag_map.push_back(arg);
        }
    }
}

void ArgumentParser::printArguments() {
    for(std::map<std::string, std::string>::const_iterator it = arg_map.begin(); it != arg_map.end(); it++) {
        std::cout << it->first << " : " << it->second << std::endl;
    }

    for(std::vector<std::string>::const_iterator it = flag_map.begin(); it != flag_map.end(); it++) {
        std::cout << "Flag : " << *it << std::endl;
    }
}

bool ArgumentParser::getFlag(std::string key) {
    std::vector<std::string>::iterator it;

    it = std::find(flag_map.begin(), flag_map.end(), key);

    return it != flag_map.end();
}

std::string ArgumentParser::getStr(std::string key) {
    return arg_map[key];
}

double ArgumentParser::getDouble(std::string key) {
    std::string val = getStr(key);

    if(!val.empty()) {
        return std::stod(val);
    }

    return NULL;
}

int ArgumentParser::getInt(std::string key) {
    std::string val = getStr(key);

    if(!val.empty()) {
        return std::stoi(val);
    }

    return NULL;
}
