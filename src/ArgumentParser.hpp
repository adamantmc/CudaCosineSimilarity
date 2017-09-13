#include <map>
#include <vector>

class ArgumentParser {

public:
    ArgumentParser();
    void parse(int argc, char **argv);
    void printArguments();
    
    bool getFlag(std::string key);
    std::string getStr(std::string key);
    double getDouble(std::string key);
    int getInt(std::string key);

private:
    std::map<std::string, std::string> arg_map;
    std::vector<std::string> flag_map;
};
