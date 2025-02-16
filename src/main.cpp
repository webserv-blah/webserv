#include "GlobalConfig.hpp"
#include "ConfigParser.hpp"

int main(int argc, char** argv) {
    GlobalConfig globalConfig;

    ConfigParser::parse(globalConfig, "config.txt");
    globalConfig.print();
    return 0;
}