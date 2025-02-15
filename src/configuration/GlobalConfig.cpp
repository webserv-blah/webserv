#include "GlobalConfig.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <map>

void GlobalConfig::printGlobalConfig() {
    for (size_t i = 0; i < servers.size(); i++) {
        std::cout << "Server " << i + 1 << ":\n";
        std::cout << "  Host: " << servers[i].host << "\n";
        std::cout << "  Port: " << servers[i].port << "\n";
        std::cout << "  Server Names: ";
        for (size_t j = 0; j < servers[i].serverNames.size(); j++) {
            std::cout << servers[i].serverNames[j];
            if (j < servers[i].serverNames.size() - 1)
                std::cout << ", ";
        }
        std::cout << "\n";

        std::cout << "  Request Handling:\n";
        std::cout << "    Methods: ";
        for (size_t j = 0; j < servers[i].reqHandling.methods.size(); j++) {
            std::cout << servers[i].reqHandling.methods[j];
            if (j < servers[i].reqHandling.methods.size() - 1)
                std::cout << ", ";
        }
        std::cout << "\n";

        std::cout << "    Error Pages:\n";
        for (std::map<int, std::string>::iterator it = servers[i].reqHandling.errorPages.begin();
             it != servers[i].reqHandling.errorPages.end(); ++it) {
            std::cout << "      " << it->first << " -> " << it->second << "\n";
        }
        std::cout << "    Return URL: " << servers[i].reqHandling.returnUrl << "\n";
        std::cout << "    Return Status: " << servers[i].reqHandling.returnStatus << "\n";
        std::cout << "    Root: " << servers[i].reqHandling.root << "\n";
        std::cout << "    Index File: " << servers[i].reqHandling.indexFile << "\n";
        std::cout << "    Upload Path: " << servers[i].reqHandling.uploadPath << "\n";
        std::cout << "    CGI Extension: " << servers[i].reqHandling.cgiExtension << "\n";

        std::cout << "    Client Max Body Size: ";
        if (servers[i].reqHandling.clientMaxBodySize.isSet())
            std::cout << servers[i].reqHandling.clientMaxBodySize.value();
        else
            std::cout << "N/A";
        std::cout << "\n";

        std::cout << "    Auto Index: ";
        if (servers[i].reqHandling.autoIndex.isSet())
            std::cout << (servers[i].reqHandling.autoIndex.value() ? "true" : "false");
        else
            std::cout << "N/A";
        std::cout << "\n";

        if (!servers[i].locations.empty()) {
            std::cout << "  Locations:\n";
            for (size_t j = 0; j < servers[i].locations.size(); j++) {
                std::cout << "    Location " << j + 1 << ":\n";
                std::cout << "      Path: " << servers[i].locations[j].path << "\n";
                std::cout << "      Request Handling:\n";
                std::cout << "        Methods: ";
                for (size_t k = 0; k < servers[i].locations[j].reqHandling.methods.size(); k++) {
                    std::cout << servers[i].locations[j].reqHandling.methods[k];
                    if (k < servers[i].locations[j].reqHandling.methods.size() - 1)
                        std::cout << ", ";
                }
                std::cout << "\n";

                std::cout << "        Error Pages:\n";
                for (std::map<int, std::string>::iterator it = servers[i].locations[j].reqHandling.errorPages.begin();
                     it != servers[i].locations[j].reqHandling.errorPages.end(); ++it) {
                    std::cout << "          " << it->first << " -> " << it->second << "\n";
                }
                std::cout << "        Return URL: " << servers[i].locations[j].reqHandling.returnUrl << "\n";
                std::cout << "        Return Status: " << servers[i].locations[j].reqHandling.returnStatus << "\n";
                std::cout << "        Root: " << servers[i].locations[j].reqHandling.root << "\n";
                std::cout << "        Index File: " << servers[i].locations[j].reqHandling.indexFile << "\n";
                std::cout << "        Upload Path: " << servers[i].locations[j].reqHandling.uploadPath << "\n";
                std::cout << "        CGI Extension: " << servers[i].locations[j].reqHandling.cgiExtension << "\n";

                std::cout << "        Client Max Body Size: ";
                if (servers[i].locations[j].reqHandling.clientMaxBodySize.isSet())
                    std::cout << servers[i].locations[j].reqHandling.clientMaxBodySize.value();
                else
                    std::cout << "N/A";
                std::cout << "\n";

                std::cout << "        Auto Index: ";
                if (servers[i].locations[j].reqHandling.autoIndex.isSet())
                    std::cout << (servers[i].locations[j].reqHandling.autoIndex.value() ? "true" : "false");
                else
                    std::cout << "N/A";
                std::cout << "\n";
            }
        }
        std::cout << "\n";
    }
}
