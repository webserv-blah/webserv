#include "ConfigParser.hpp"

namespace {
    void parseIPv6Address(std::string& hostPortToken, std::string::iterator& tokenIterator, std::string& host) {
        // Ensure the token starts with '['
        if (tokenIterator == hostPortToken.end() || *tokenIterator != '[') {
            throw std::runtime_error("Invalid IPv6 address: missing opening '['");
        }
        ++tokenIterator; // Skip the '['

        std::string parsedAddress;
        // IPv6 addresses must have exactly 8 hextets
        for (int i = 0; i < 8; ++i) {
            std::string hextet;
            int count = 0;
            // Read up to 4 hex digits for the hextet
            while (tokenIterator != hostPortToken.end() && count < 4 && std::isxdigit(*tokenIterator)) {
                hextet.push_back(*tokenIterator);
                ++tokenIterator;
                ++count;
            }
            // Each hextet must have at least one hex digit
            if (hextet.empty()) {
                throw std::runtime_error("Invalid IPv6 address: expected hex digits in hextet");
            }
            // If there is an extra hex digit beyond 4, that's an error.
            if (tokenIterator != hostPortToken.end() && std::isxdigit(*tokenIterator)) {
                throw std::runtime_error("Invalid IPv6 address: hextet exceeds 4 hex digits");
            }

            // Omit any leading zeros from the hextet.
            size_t nonZeroPos = 0;
            while (nonZeroPos < hextet.size() && hextet[nonZeroPos] == '0') {
                ++nonZeroPos;
            }
            std::string stripped = (nonZeroPos == hextet.size()) ? "0" : hextet.substr(nonZeroPos);

            // Convert hextet to lower-case.
            for (size_t j = 0; j < stripped.size(); ++j) {
                stripped[j] = std::tolower(stripped[j]);
            }

            // Append the processed hextet to the result (add colon as delimiter if needed)
            if (!parsedAddress.empty()) {
                parsedAddress.push_back(':');
            }
            parsedAddress.append(stripped);

            // For all but the last hextet, expect a colon delimiter.
            if (i < 7) {
                if (tokenIterator == hostPortToken.end() || *tokenIterator != ':') {
                    throw std::runtime_error("Invalid IPv6 address: expected ':' delimiter between hextets");
                }
                ++tokenIterator; // Skip the ':'
            }
        }

        // After 8 hextets, we must have a closing bracket ']'
        if (tokenIterator == hostPortToken.end() || *tokenIterator != ']') {
            throw std::runtime_error("Invalid IPv6 address: missing closing ']'");
        }
        ++tokenIterator; // Skip the ']'

        // Store the formatted IPv6 address with omitted leading zeros in lower-case into host.
        host = parsedAddress;
    }

    void parseIPv4Address(std::string& hostPortToken, std::string::iterator& tokenIterator, std::string& host) {
        std::string parsedAddress;

        for (int i = 0; i < 4; ++i) {
            std::string octet;
            int count = 0;
            while (tokenIterator != hostPortToken.end() && count < 4 && std::isdigit(*tokenIterator)) {
                octet.push_back(*tokenIterator);
                ++tokenIterator;
                ++count;
            }
            // Each octet must have at least one digit
            if (octet.empty()) {
                throw std::runtime_error("Invalid IPv4 address: expected digits in octet");
            }
            // If there is an extra digit beyond 3, that's an error.
            if (tokenIterator != hostPortToken.end() && std::isdigit(*tokenIterator)) {
                throw std::runtime_error("Invalid IPv4 address: octet exceeds 3 digits");
            }

            // Convert the octet to an integer and validate its range
            int octetVal = utils::stoi(octet);
            if (octetVal < 0 || octetVal > 255) {
                throw std::runtime_error("Invalid IPv4 address: octet out of range");
            }

            // Omit any leading zeros from the octet.
            size_t nonZeroPos = 0;
            while (nonZeroPos < octet.size() && octet[nonZeroPos] == '0') {
                ++nonZeroPos;
            }
            std::string stripped = (nonZeroPos == octet.size()) ? "0" : octet.substr(nonZeroPos);

            // Append the processed octet to the result (add dot as delimiter if needed)
            if (!parsedAddress.empty()) {
                parsedAddress.push_back('.');
            }
            parsedAddress.append(stripped);

            // For all but the last octet, expect a dot delimiter.
            if (i < 3) {
                if (tokenIterator == hostPortToken.end() || *tokenIterator != '.') {
                    throw std::runtime_error("Invalid IPv4 address: expected '.' delimiter between octets");
                }
                ++tokenIterator; // Skip the '.'
            }
        }

        // Store the formatted IPv4 address into host.
        host = parsedAddress;
    }

    void parsePort(std::string& hostPortToken, std::string::iterator& tokenIterator, unsigned int& port) {
        if (tokenIterator == hostPortToken.end()) {
            throw std::runtime_error("Expected port number in listen directive");
        }

        // The remaining characters must be all digits.
        if (utils::all_of(tokenIterator, hostPortToken.end(), ::isdigit)) {
            port = utils::stoi(std::string(tokenIterator, hostPortToken.end()));
            if (port == 0 || 65535 < port) {
                throw std::runtime_error("Invalid port number in listen directive");
            }
        } else {
            throw std::runtime_error("Invalid port number in listen directive");
        }
        tokenIterator = hostPortToken.end();
    }
}

void ConfigParser::parseHostPort(std::ifstream& configFile, std::string& host, unsigned int& port) {
    std::string token = getNextToken(configFile);
    if (token.empty()) {
        throw std::runtime_error("Unexpected end of file in listen directive");
    } else if (token == ";") {
        throw std::runtime_error("Expected an argument for listen directive");
    }

    // If there is only a port number, use the default host
    std::string::iterator tokenIterator = token.begin();
    if (!utils::all_of(token.begin(), token.end(), ::isdigit)) {
        if (*tokenIterator == '[') {
            parseIPv6Address(token, tokenIterator, host);
        }
        else {
            parseIPv4Address(token, tokenIterator, host);
        }
        // tokenIterator is now pointing to token.end() or non-number character
        if (tokenIterator != token.end()) {
            if (*tokenIterator == ':') {
                ++tokenIterator;
                parsePort(token, tokenIterator, port);
            } else {
                throw std::runtime_error("Invalid character in host:port");
            }
        }
    } else {
        parsePort(token, tokenIterator, port);
    }
}

namespace {
// Helper function to check if a server name is valid
// Allowed characters: alphanumerics, '-', '.', and '*' (only at the beginning or end)
    bool isValidServerName(const std::string& name) {
        if (name.empty()) {
            return false;
        }

        // Check for invalid characters
        for (std::size_t i = 0; i < name.size(); ++i) {
            char c = name[i];
            if (std::isalnum(c) || c == '-' || c == '.' || c == '*') {
                continue;
            }
            return false;
        }

        // Hyphen should not be at the beginning or end
        if (name.front() == '-' || name.back() == '-') {
            return false;
        }

        // If wildcard '*' is present, ensure it is only at the beginning or end
        std::size_t firstPos = name.find('*');
        std::size_t lastPos = name.rfind('*');

        if (firstPos != std::string::npos) {
            if (firstPos != 0 && firstPos != name.size() - 1) {
                return false;
            }
        }
        // Ensure all '*' are either at the beginning or end
        if (firstPos != lastPos) {
            return false;
        }

        return true;
    }
}

void ConfigParser::parseServerNames(std::ifstream& configFile, std::vector<std::string>& serverNames) {
    std::string nextToken;
    while (true) {
        nextToken = getNextToken(configFile);
        if (nextToken.empty()) {
            throw std::runtime_error("Unexpected end of file in server_name directive");
        } else if (nextToken == ";") {
            break;
        } else {
            // Validate the token
            if (!isValidServerName(nextToken)) {
                throw std::runtime_error("Invalid character in server_name");
            }
            serverNames.push_back(nextToken);
        }
    }
    // If no server names are provided, default to an empty string
    if (serverNames.empty()) {
        serverNames.push_back("");
    }
}