#include "ConfigParser.hpp"

namespace {
    // IPv6 주소를 파싱하는 함수
    // hostPortToken: 파싱할 문자열
    // tokenIterator: 문자열 내의 현재 위치 iterator
    // host: 파싱된 호스트(IPv6 주소)를 저장할 변수
    void parseIPv6Address(std::string& hostPortToken, std::string::iterator& tokenIterator, std::string& host) {
        // 토큰이 '['로 시작하는지 확인
        if (tokenIterator == hostPortToken.end() || *tokenIterator != '[') {
            throw std::runtime_error("Invalid IPv6 address: missing opening '['");
        }
        ++tokenIterator; // '[' 건너뛰기

        std::string parsedAddress;
        // IPv6 주소는 정확히 8개의 hextet(16진수 묶음)으로 구성되어야 함
        for (int i = 0; i < 8; ++i) {
            std::string hextet;
            int count = 0;
            // 각 hextet은 최대 4자리의 16진수로 구성됨
            while (tokenIterator != hostPortToken.end() && count < 4 && std::isxdigit(*tokenIterator)) {
                hextet.push_back(*tokenIterator);
                ++tokenIterator;
                ++count;
            }
            // 각 hextet은 최소 한 자리 이상의 16진수가 있어야 함
            if (hextet.empty()) {
                throw std::runtime_error("Invalid IPv6 address: expected hex digits in hextet");
            }
            // 4자리 초과의 16진수는 오류
            if (tokenIterator != hostPortToken.end() && std::isxdigit(*tokenIterator)) {
                throw std::runtime_error("Invalid IPv6 address: hextet exceeds 4 hex digits");
            }

            // hextet에서 선행하는 0을 제거
            size_t nonZeroPos = 0;
            while (nonZeroPos < hextet.size() && hextet[nonZeroPos] == '0') {
                ++nonZeroPos;
            }
            std::string stripped = (nonZeroPos == hextet.size()) ? "0" : hextet.substr(nonZeroPos);

            // hextet을 소문자로 변환
            for (size_t j = 0; j < stripped.size(); ++j) {
                stripped[j] = std::tolower(stripped[j]);
            }

            // 처리된 hextet을 결과 문자열에 추가 (필요 시 ':' 구분자 추가)
            if (!parsedAddress.empty()) {
                parsedAddress.push_back(':');
            }
            parsedAddress.append(stripped);

            // 마지막 hextet을 제외하고는 ':' 구분자가 있어야 함
            if (i < 7) {
                if (tokenIterator == hostPortToken.end() || *tokenIterator != ':') {
                    throw std::runtime_error("Invalid IPv6 address: expected ':' delimiter between hextets");
                }
                ++tokenIterator; // ':' 건너뛰기
            }
        }

        // 8개의 hextet 이후에는 닫는 대괄호 ']'가 있어야 함
        if (tokenIterator == hostPortToken.end() || *tokenIterator != ']') {
            throw std::runtime_error("Invalid IPv6 address: missing closing ']'");
        }
        ++tokenIterator; // ']' 건너뛰기

        // 처리된 IPv6 주소를 host 변수에 저장
        host = parsedAddress;
    }

    // IPv4 주소를 파싱하는 함수
    // hostPortToken: 파싱할 문자열
    // tokenIterator: 문자열 내의 현재 위치 iterator
    // host: 파싱된 호스트(IPv4 주소)를 저장할 변수
    void parseIPv4Address(std::string& hostPortToken, std::string::iterator& tokenIterator, std::string& host) {
        std::string parsedAddress;

        // IPv4 주소는 4개의 옥텟으로 구성됨
        for (int i = 0; i < 4; ++i) {
            std::string octet;
            int count = 0;
            // 각 옥텟은 최대 3자리의 숫자로 구성됨
            while (tokenIterator != hostPortToken.end() && count < 4 && std::isdigit(*tokenIterator)) {
                octet.push_back(*tokenIterator);
                ++tokenIterator;
                ++count;
            }
            // 각 옥텟은 최소 한 자리 이상의 숫자가 있어야 함
            if (octet.empty()) {
                throw std::runtime_error("Invalid IPv4 address: expected digits in octet");
            }
            // 3자리 초과의 숫자는 오류
            if (tokenIterator != hostPortToken.end() && std::isdigit(*tokenIterator)) {
                throw std::runtime_error("Invalid IPv4 address: octet exceeds 3 digits");
            }

            // 옥텟을 정수로 변환하고 범위를 검증
            int octetVal = utils::stoi(octet);
            if (octetVal < 0 || octetVal > 255) {
                throw std::runtime_error("Invalid IPv4 address: octet out of range");
            }

            // 옥텟에서 선행하는 0을 제거
            size_t nonZeroPos = 0;
            while (nonZeroPos < octet.size() && octet[nonZeroPos] == '0') {
                ++nonZeroPos;
            }
            std::string stripped = (nonZeroPos == octet.size()) ? "0" : octet.substr(nonZeroPos);

            // 처리된 옥텟을 결과 문자열에 추가 (필요 시 '.' 구분자 추가)
            if (!parsedAddress.empty()) {
                parsedAddress.push_back('.');
            }
            parsedAddress.append(stripped);

            // 마지막 옥텟을 제외하고는 '.' 구분자가 있어야 함
            if (i < 3) {
                if (tokenIterator == hostPortToken.end() || *tokenIterator != '.') {
                    throw std::runtime_error("Invalid IPv4 address: expected '.' delimiter between octets");
                }
                ++tokenIterator; // '.' 건너뛰기
            }
        }

        // 처리된 IPv4 주소를 host 변수에 저장
        host = parsedAddress;
    }

    // 포트 번호를 파싱하는 함수
    // hostPortToken: 파싱할 문자열
    // tokenIterator: 문자열 내의 현재 위치 iterator
    // port: 파싱된 포트 번호를 저장할 변수
    void parsePort(std::string& hostPortToken, std::string::iterator& tokenIterator, unsigned int& port) {
        if (tokenIterator == hostPortToken.end()) {
            throw std::runtime_error("Expected port number in listen directive");
        }

        // 남은 문자가 모두 숫자인지 확인
        if (utils::all_of(tokenIterator, hostPortToken.end(), ::isdigit)) {
            // 문자열을 정수로 변환
            port = utils::stoi(std::string(tokenIterator, hostPortToken.end()));
            if (port == 0 || 65535 < port) {
                // 포트 번호는 0에서 65535 사이여야 함
                throw std::runtime_error("Invalid port number in listen directive");
            }
        } else {
            // 숫자가 아닌 문자가 있으면 오류
            throw std::runtime_error("Invalid port number in listen directive");
        }
        // tokenIterator를 끝으로 이동
        tokenIterator = hostPortToken.end();
    }
}

// listen 지시문에서 호스트와 포트 정보를 파싱하는 함수
void ConfigParser::parseHostPort(std::ifstream& configFile, std::string& host, unsigned int& port) {
    std::string token = getNextToken(configFile);
    if (token.empty()) {
        throw std::runtime_error("Unexpected end of file in listen directive");
    } else if (token == ";") {
        throw std::runtime_error("Expected an argument for listen directive");
    }

    // 토큰이 오로지 포트 번호만 있을 경우 기본 호스트 사용
    std::string::iterator tokenIterator = token.begin();
    if (!utils::all_of(token.begin(), token.end(), ::isdigit)) {
        if (*tokenIterator == '[') {
            // IPv6 주소인 경우
            parseIPv6Address(token, tokenIterator, host);
        }
        else {
            // IPv4 주소인 경우
            parseIPv4Address(token, tokenIterator, host);
        }
        // tokenIterator는 이제 token.end()이거나 숫자가 아닌 문자를 가리킴
        if (tokenIterator != token.end()) {
            if (*tokenIterator == ':') {
                ++tokenIterator;
                parsePort(token, tokenIterator, port);
            } else {
                throw std::runtime_error("Invalid character in host:port");
            }
        }
    } else {
        // 토큰이 숫자로만 구성되어 있으면 포트 번호로 간주
        parsePort(token, tokenIterator, port);
    }
}

namespace {
    // 서버 이름이 유효한지 검사하는 헬퍼 함수
    // 허용되는 문자: 영숫자, '-', '.', 그리고 '*' (맨 앞이나 맨 뒤에만 올 수 있음)
    bool isValidServerName(const std::string& name) {
        if (name.empty()) {
            return false;
        }

        // 유효하지 않은 문자가 포함되어 있는지 확인
        for (std::size_t i = 0; i < name.size(); ++i) {
            char c = name[i];
            if (std::isalnum(c) || c == '-' || c == '.' || c == '*') {
                continue;
            }
            return false;
        }

        // '-'는 시작이나 끝에 위치하면 안 됨
        if (name.front() == '-' || name.back() == '-') {
            return false;
        }

        // '*'가 있을 경우, 맨 앞이나 맨 뒤에만 위치해야 함
        std::size_t firstPos = name.find('*');
        std::size_t lastPos = name.rfind('*');

        if (firstPos != std::string::npos) {
            if (firstPos != 0 && firstPos != name.size() - 1) {
                return false;
            }
        }
        // '*'가 둘 이상이면 오류
        if (firstPos != lastPos) {
            return false;
        }

        return true;
    }
}

// server_name 지시문에서 서버 이름들을 파싱하는 함수
void ConfigParser::parseServerNames(std::ifstream& configFile, std::vector<std::string>& serverNames) {
    std::string nextToken;
    while (true) {
        nextToken = peekNextToken(configFile);
        if (nextToken.empty()) {
            throw std::runtime_error("Unexpected end of file in server_name directive");
        } else if (nextToken == ";") {
            break;
        } else {
            // 토큰의 유효성을 검사
            if (!isValidServerName(nextToken)) {
                throw std::runtime_error("Invalid character in server_name");
            }
            serverNames.push_back(nextToken);
            nextToken = getNextToken(configFile);
        }
    }
    // 서버 이름이 제공되지 않을 경우, 빈 문자열을 기본값으로 사용
    if (serverNames.empty()) {
        serverNames.push_back("");
    }
}
