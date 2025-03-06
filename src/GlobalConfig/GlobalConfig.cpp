#include "GlobalConfig.hpp"

// 정적 멤버 변수 초기화
GlobalConfig* GlobalConfig::instance_ = NULL;
bool GlobalConfig::isInitialized_ = false;

// 싱글톤 인스턴스를 반환
const GlobalConfig& GlobalConfig::getInstance() {
	if (!isInitialized_) {
		throw std::logic_error("GlobalConfig is not initialized. Please initialize it by calling initGlobalConfig().");
	}
	return *instance_;
}

// 설정 파일을 이용해 GlobalConfig를 초기화 (단 한 번만 호출 가능)
void GlobalConfig::initGlobalConfig(const char* path) {
	if (isInitialized_) {
		throw std::logic_error("GlobalConfig has already been initialized. You cannot initialize it more than once.");
	}
	instance_ = new GlobalConfig();
	ConfigParser::parse(*instance_, path);  // 설정 파일을 파싱하고 초기화 수행
	isInitialized_ = true;
}

// 싱글톤 인스턴스를 파괴
void GlobalConfig::destroyInstance() {
	delete instance_;
	instance_ = NULL;
	isInitialized_ = false;
}

// GlobalConfig 클래스 내의 findRequestConfig 함수: 
// listenFd, 도메인 이름, 그리고 target URL에 해당하는 RequestConfig를 찾는다.
const RequestConfig* GlobalConfig::findRequestConfig(const int listenFd, const std::string& domainName, \
const std::string& targetUri) const {
	// listenFd가 등록된 서버 목록에 없다면 NULL 반환
	if (listenFdToServers_.find(listenFd) == listenFdToServers_.end())
		return NULL;
	// listenFd에 해당하는 서버들의 벡터를 가져옴
	const std::vector<ServerConfig*>& servers = listenFdToServers_.at(listenFd);
	// 주어진 도메인 이름과 일치하는 서버 구성을 찾음
	const ServerConfig& server = findServerConfig(servers, domainName);
	// 해당 서버에서 target URL에 해당하는 location 설정을 찾음
	return findLocationConfig(server, targetUri);
}


// GlobalConfig 클래스 내의 findServerConfig 함수: 
// 주어진 서버 목록에서 도메인 이름과 일치하는 서버 설정을 찾는다.
const ServerConfig& GlobalConfig::findServerConfig(const std::vector<ServerConfig*>& servers, \
const std::string& domainName) const {
	if (!domainName.empty()) {
		// 서버 목록을 순회하며 각 서버의 serverNames_ 벡터 내에서 도메인 이름을 찾음
		for (size_t i = 0; i < servers.size(); ++i) {
			for (size_t j = 0; j < servers[i]->serverNames_.size(); j++) {
				// 도메인 이름이 일치하면 해당 서버 설정을 반환
				if (servers[i]->serverNames_[j] == domainName)
					return *servers[i];
			}
		}
	}
	// 일치하는 도메인을 찾지 못하면 첫 번째 서버 설정을 반환
	return *servers[0];
}


// GlobalConfig 클래스 내의 findLocationConfig 함수: 
// 주어진 서버 설정 내에서 target URL과 가장 잘 맞는 location 설정을 찾는다.
const RequestConfig* GlobalConfig::findLocationConfig(const ServerConfig& server, \
const std::string& targetUri) const {
	// targetUri이 비어있으면 서버의 기본 RequestConfig를 반환
	if (targetUri.empty()) {
		return &server.reqConfig_;
	}
	const LocationConfig* longestPrefixMatch = NULL; // 가장 긴 접두어 매칭을 저장할 포인터 초기화
	// 서버의 location 설정 목록을 순회
	for (size_t i = 0; i < server.locations_.size(); ++i) {
		// targetUri이 현재 location의 locationPath_로 시작하는지 확인
		if (targetUri.find(server.locations_[i].reqConfig_.locationPath_) == 0) {
			// 현재 매칭이 이전보다 길거나 아직 매칭이 없으면 업데이트
			if (longestPrefixMatch == NULL || \
				server.locations_[i].reqConfig_.locationPath_.size() > longestPrefixMatch->reqConfig_.locationPath_.size())
				longestPrefixMatch = &server.locations_[i];
		}
	}
	// 적절한 location 설정을 찾지 못하면 기본 RequestConfig를 반환
	if (longestPrefixMatch == NULL) {
		return &server.reqConfig_;
	} else {
		// 가장 긴 접두어 매칭을 가진 location의 RequestConfig를 반환
		return &longestPrefixMatch->reqConfig_;
	}
}

void GlobalConfig::print() const {
    for (size_t i = 0; i < servers_.size(); i++) {
        std::cout << "\n====== Server " << i + 1 << " ======\n";
        std::cout << "Host:         " << servers_[i].host_ << "\n";
        std::cout << "Port:         " << servers_[i].port_ << "\n";
        std::cout << "Server Names: ";
        for (size_t j = 0; j < servers_[i].serverNames_.size(); j++) {
            std::cout << servers_[i].serverNames_[j];
            if (j < servers_[i].serverNames_.size() - 1)
                std::cout << ", ";
        }
        std::cout << "\n\n";

        std::cout << "--- Default Server Configuration ---\n";
        servers_[i].reqConfig_.print(1);  // Pass indentation level
        
        if (!servers_[i].locations_.empty()) {
            std::cout << "\n--- Location Blocks ---\n";
            for (size_t j = 0; j < servers_[i].locations_.size(); j++) {
                std::cout << "Location [" << servers_[i].locations_[j].reqConfig_.locationPath_ << "]:\n";
                servers_[i].locations_[j].reqConfig_.print(1);  // Pass indentation level
                std::cout << "\n";
            }
        }
        std::cout << "==============================\n";
    }
}

void RequestConfig::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    
    // Methods
    std::cout << indent << "Methods:            ";
    if (methods_.empty()) {
        std::cout << "All methods allowed\n";
    } else {
        for (size_t i = 0; i < methods_.size(); i++) {
            std::cout << methods_[i];
            if (i < methods_.size() - 1)
                std::cout << ", ";
        }
        std::cout << "\n";
    }

    // Error Pages
    std::cout << indent << "Error Pages:        ";
    if (errorPages_.empty()) {
        std::cout << "None defined\n";
    } else {
        std::cout << "\n";
        for (std::map<int, std::string>::const_iterator it = errorPages_.begin();
             it != errorPages_.end(); ++it) {
            std::cout << indent << "  " << it->first << " -> " << it->second << "\n";
        }
    }

    // Other configuration options
    std::cout << indent << "Return URL:         " << (returnUrl_.empty() ? "None" : returnUrl_) << "\n";
    std::cout << indent << "Return Status:      " << (returnStatus_ == 0 ? "None" : utils::int_tos(returnStatus_)) << "\n";
    std::cout << indent << "Root:               " << (root_.empty() ? "None" : root_) << "\n";
    std::cout << indent << "Index File:         " << (indexFile_.empty() ? "None" : indexFile_) << "\n";
    std::cout << indent << "Upload Path:        " << (uploadPath_.empty() ? "None" : uploadPath_) << "\n";
    std::cout << indent << "CGI Extension:      " << (cgiExtension_.empty() ? "None" : cgiExtension_) << "\n";

    // Size and auto-index settings
    std::cout << indent << "Client Max Body Size: ";
    if (clientMaxBodySize_.isSet()) {
        std::cout << clientMaxBodySize_.value();
        if (clientMaxBodySize_.value() >= 1024 * 1024) {
            std::cout << " (" << (clientMaxBodySize_.value() / (1024 * 1024)) << " MB)";
        } else if (clientMaxBodySize_.value() >= 1024) {
            std::cout << " (" << (clientMaxBodySize_.value() / 1024) << " KB)";
        }
    } else {
        std::cout << "Not set";
    }
    std::cout << "\n";

    std::cout << indent << "Auto Index:         ";
    if (autoIndex_.isSet())
        std::cout << (autoIndex_.value() ? "on" : "off");
    else
        std::cout << "Default (off)";
    std::cout << "\n";
}