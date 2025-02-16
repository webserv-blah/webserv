#include "GlobalConfig.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <map>

void GlobalConfig::print() {
    for (size_t i = 0; i < servers_.size(); i++) {
        std::cout << "Server " << i + 1 << ":\n";
        std::cout << "  Host: " << servers_[i].host_ << "\n";
        std::cout << "  Port: " << servers_[i].port_ << "\n";
        std::cout << "  Server Names: ";
        for (size_t j = 0; j < servers_[i].serverNames_.size(); j++) {
            std::cout << servers_[i].serverNames_[j];
            if (j < servers_[i].serverNames_.size() - 1)
                std::cout << ", ";
        }
        std::cout << "\n";

        std::cout << "  Request Handling:\n";

        std::cout << "    Error Pages:\n";
        for (std::map<int, std::string>::iterator it = servers_[i].reqHandling_.errorPages_.begin();
             it != servers_[i].reqHandling_.errorPages_.end(); ++it) {
            std::cout << "      " << it->first << " -> " << it->second << "\n";
        }
        std::cout << "    Return URL: " << servers_[i].reqHandling_.returnUrl_ << "\n";
        std::cout << "    Return Status: " << servers_[i].reqHandling_.returnStatus_ << "\n";
        std::cout << "    Root: " << servers_[i].reqHandling_.root_ << "\n";
        std::cout << "    Index File: " << servers_[i].reqHandling_.indexFile_ << "\n";
        std::cout << "    Upload Path: " << servers_[i].reqHandling_.uploadPath_ << "\n";
        std::cout << "    CGI Extension: " << servers_[i].reqHandling_.cgiExtension_ << "\n";

        std::cout << "    Client Max Body Size: ";
        if (servers_[i].reqHandling_.clientMaxBodySize_.isSet())
            std::cout << servers_[i].reqHandling_.clientMaxBodySize_.value();
        else
            std::cout << "N/A";
        std::cout << "\n";

        std::cout << "    Auto Index: ";
        if (servers_[i].reqHandling_.autoIndex_.isSet())
            std::cout << (servers_[i].reqHandling_.autoIndex_.value() ? "on" : "off");
        else
            std::cout << "on";
        std::cout << "\n";

        if (!servers_[i].locations_.empty()) {
            std::cout << "  Locations:\n";
            for (size_t j = 0; j < servers_[i].locations_.size(); j++) {
                std::cout << "    Location " << j + 1 << ":\n";
                std::cout << "      Path: " << servers_[i].locations_[j].path_ << "\n";
                std::cout << "      Request Handling:\n";
                std::cout << "        Methods: ";
                for (size_t k = 0; k < servers_[i].locations_[j].reqHandling_.methods_.size(); k++) {
                    std::cout << servers_[i].locations_[j].reqHandling_.methods_[k];
                    if (k < servers_[i].locations_[j].reqHandling_.methods_.size() - 1)
                        std::cout << ", ";
                }
                std::cout << "\n";

                std::cout << "        Error Pages:\n";
                for (std::map<int, std::string>::iterator it = servers_[i].locations_[j].reqHandling_.errorPages_.begin();
                     it != servers_[i].locations_[j].reqHandling_.errorPages_.end(); ++it) {
                    std::cout << "          " << it->first << " -> " << it->second << "\n";
                }
                std::cout << "        Return URL: " << servers_[i].locations_[j].reqHandling_.returnUrl_ << "\n";
                std::cout << "        Return Status: " << servers_[i].locations_[j].reqHandling_.returnStatus_ << "\n";
                std::cout << "        Root: " << servers_[i].locations_[j].reqHandling_.root_ << "\n";
                std::cout << "        Index File: " << servers_[i].locations_[j].reqHandling_.indexFile_ << "\n";
                std::cout << "        Upload Path: " << servers_[i].locations_[j].reqHandling_.uploadPath_ << "\n";
                std::cout << "        CGI Extension: " << servers_[i].locations_[j].reqHandling_.cgiExtension_ << "\n";

                std::cout << "        Client Max Body Size: ";
                if (servers_[i].locations_[j].reqHandling_.clientMaxBodySize_.isSet())
                    std::cout << servers_[i].locations_[j].reqHandling_.clientMaxBodySize_.value();
                else
                    std::cout << "N/A";
                std::cout << "\n";

                std::cout << "        Auto Index: ";
                if (servers_[i].locations_[j].reqHandling_.autoIndex_.isSet())
                    std::cout << (servers_[i].locations_[j].reqHandling_.autoIndex_.value() ? "on" : "off");
                else
                    std::cout << "on";
                std::cout << "\n";
            }
        }
        std::cout << "\n";
    }
}