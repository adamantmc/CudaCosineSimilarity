#include "CSVParser.hpp"
#include <iostream>
#include <fstream>

CSVParser::CSVParser(char delimiter) {
    this->delimiter = delimiter;
}

std::vector<std::vector<std::string> > CSVParser::parse(char *filename) {
    this->contents.clear();
    
    std::ifstream input(filename);

    std::string line;
    std::vector<std::string> tokens;


    while(std::getline(input, line)) {
        tokens = this->tokenize(line);
        this->contents.push_back(tokens);
    }

    return contents;
}

std::vector<std::string> CSVParser::tokenize(std::string line) {
    std::vector<std::string> tokens;

    // Start and end for each token
    int start = 0;
    int end = 0;

    // Parse line by getting all tokens split by the delimiter
    for(int i = 0; i < line.size(); i++) {
        if(line[i] == this->delimiter || i == line.size() - 1) {
            // Found delimiter, add substring to tokens and update
            // start and end indexes
            end = i-1;
            tokens.push_back(line.substr(start, end-start+1));
            start = i+1;
        }
    }

    return tokens;
}
