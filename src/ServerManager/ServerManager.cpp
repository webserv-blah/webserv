#include "ServerManager.hpp"

#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

ServerManager::~ServerManager() {
    for (std::set<int>::iterator it = listenFds_.begin(); it != listenFds_.end(); ++it) {
        close(*it);
    }
    listenFds_.clear();
}

void ServerManager::setupListeningSockets() {
	// Get the global configuration instance.
	GlobalConfig& globalConfig = const_cast<GlobalConfig&>(GlobalConfig::getInstance());
    // Map to associate a host/port pair with its corresponding socket file descriptor.
    std::map<std::pair<std::string, int>, int> addressToSocket;

    // Iterate over each virtual server configuration defined in the globalConfig.
    for (size_t i = 0; i < globalConfig.servers_.size(); ++i) {
        ServerConfig& server = globalConfig.servers_[i];

        // Get the host and port for this server.
        std::pair<std::string, int> key = std::make_pair(server.host_, server.port_);

        int sockFd;
        // Check if a socket for this host/port combination has already been created.
        if (addressToSocket.find(key) != addressToSocket.end()) {
            sockFd = addressToSocket[key];
        } else {
            // Create a new TCP socket.
            sockFd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockFd < 0) {
				throw std::runtime_error("Failed to create socket");
            }

            // Set the socket to non-blocking mode.
            // Retrieve the current flags for the socket.
            int flags = fcntl(sockFd, F_GETFL, 0);
            if (flags == -1) {
                close(sockFd);
                throw std::runtime_error("Failed to get socket flags");
            }
            // Add the O_NONBLOCK flag to ensure non-blocking operations.
            if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) == -1) {
                close(sockFd);
                throw std::runtime_error("Failed to set socket flags");
            }

            // Set socket option to allow the reuse of local addresses.
            int opt = 1;
            if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                close(sockFd);
                throw std::runtime_error("Failed to set socket options");
            }

            // Initialize the sockaddr_in structure to define the server's address.
            sockaddr_in addr;
            addr.sin_family = AF_INET;                 // Use IPv4.
            addr.sin_port = htons(server.port_);       // Set the port, converting to network byte order.
            if (server.host_ == "0.0.0.0") {
                addr.sin_addr.s_addr = INADDR_ANY;       // Bind to all available interfaces.
            } else {
                // Convert the IP address from text to binary form.
                if (inet_aton(server.host_.c_str(), &addr.sin_addr) == 0) {
                    close(sockFd);
					throw std::runtime_error("Invalid IP address");
                }
            }

            // Bind the socket to the specified IP address and port.
            if (bind(sockFd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
                close(sockFd);
				std::string errorMsg = "Failed to bind socket to " + server.host_ + ":" + std::to_string(server.port_) + " - " + strerror(errno);
    			throw std::runtime_error(errorMsg);
            }

            // Begin listening for incoming connections on the socket.
            if (listen(sockFd, SOMAXCONN) < 0) {
                close(sockFd);
                throw std::runtime_error("Failed to listen on socket");
            }

            // Store the new socket in our mapping so we don't create duplicate sockets.
            addressToSocket[key] = sockFd;
            // Add the socket to the set of listening file descriptors.
            listenFds_.insert(sockFd);
        }

        // Map this server configuration to the corresponding listening socket.
        // This allows multiple server configurations to share the same socket.
        globalConfig.listenFdToServers_[sockFd].push_back(&server);
    }
}

extern volatile bool globalServerRunning;

bool ServerManager::isServerRunning() {
    return globalServerRunning;
}

// print listenFds, listenFdToServers_
void ServerManager::print() {
    std::cout << "listenFds: ";
    for (std::set<int>::iterator it = listenFds_.begin(); it != listenFds_.end(); ++it) {
        std::cout << *it << ", ";
    }
    std::cout << std::endl;

    std::cout << "listenFdToServers_: " << std::endl;
    for (std::map<int, std::vector<ServerConfig*> >::const_iterator it = GlobalConfig::getInstance().listenFdToServers_.begin();
         it != GlobalConfig::getInstance().listenFdToServers_.end(); ++it) {
        std::cout << "listenFd: " << it->first << std::endl;
        for (size_t i = 0; i < it->second.size(); ++i) {
            std::cout << "server: " << it->second[i]->host_ << ":" << it->second[i]->port_ << std::endl;
        }
    }
}