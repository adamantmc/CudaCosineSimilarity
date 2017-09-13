#ifndef CSV_PARSER_H_
#define CSV_PARSER_H_

#include <string>
#include <vector>

class CSVParser {

public:
    CSVParser(char delimiter = ',');
    std::vector<std::vector<std::string> > parse(char *filename);

private:
    char delimiter;
    std::vector<std::vector<std::string> > contents;

    std::vector<std::string> tokenize(std::string);

};

#endif // CSV_PARSER_H_
