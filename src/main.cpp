#include "GlobalConfig.hpp"
#include "ConfigParser.hpp"

int main(int argc, char** argv) {
    GlobalConfig globalConfig;

    ConfigParser::parse(globalConfig, argv[1]);
    globalConfig.print();
    return 0;
}