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
	// 프로그램 종료 시 싱글톤 인스턴스 파괴를 위해 atexit에 등록
	std::atexit(&GlobalConfig::destroyInstance);
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
const std::string& targetUrl) const {
	// listenFd가 등록된 서버 목록에 없다면 NULL 반환
	if (listenFdToServers_.find(listenFd) == listenFdToServers_.end())
		return NULL;
	// listenFd에 해당하는 서버들의 벡터를 가져옴
	const std::vector<ServerConfig*>& servers = listenFdToServers_.at(listenFd);
	// 주어진 도메인 이름과 일치하는 서버 구성을 찾음
	const ServerConfig& server = findServerConfig(servers, domainName);
	// 해당 서버에서 target URL에 해당하는 location 설정을 찾음
	return findLocationConfig(server, targetUrl);
}


// GlobalConfig 클래스 내의 findServerConfig 함수: 
// 주어진 서버 목록에서 도메인 이름과 일치하는 서버 설정을 찾는다.
const ServerConfig& GlobalConfig::findServerConfig(const std::vector<ServerConfig*>& servers, \
const std::string& domainName) const {
	// 서버 목록을 순회하며 각 서버의 serverNames_ 벡터 내에서 도메인 이름을 찾음
	for (size_t i = 0; i < servers.size(); ++i) {
		for (size_t j = 0; j < servers[i]->serverNames_.size(); j++) {
			// 도메인 이름이 일치하면 해당 서버 설정을 반환
			if (servers[i]->serverNames_[j] == domainName)
				return *servers[i];
		}
	}
	// 일치하는 도메인을 찾지 못하면 첫 번째 서버 설정을 반환
	return *servers[0];
}


// GlobalConfig 클래스 내의 findLocationConfig 함수: 
// 주어진 서버 설정 내에서 target URL과 가장 잘 맞는 location 설정을 찾는다.
const RequestConfig* GlobalConfig::findLocationConfig(const ServerConfig& server, \
const std::string& targetUrl) const {
	const LocationConfig* longestPrefixMatch = NULL; // 가장 긴 접두어 매칭을 저장할 포인터 초기화
	// 서버의 location 설정 목록을 순회
	for (size_t i = 0; i < server.locations_.size(); ++i) {
		// targetUrl이 현재 location의 path_로 시작하는지 확인
		if (targetUrl.find(server.locations_[i].path_) == 0) {
			// 현재 매칭이 이전보다 길거나 아직 매칭이 없으면 업데이트
			if (longestPrefixMatch == NULL || \
				server.locations_[i].path_.size() > longestPrefixMatch->path_.size())
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
		for (std::map<int, std::string>::const_iterator it = servers_[i].reqConfig_.errorPages_.begin();
			 it != servers_[i].reqConfig_.errorPages_.end(); ++it) {
			std::cout << "      " << it->first << " -> " << it->second << "\n";
		}
		std::cout << "    Return URL: " << servers_[i].reqConfig_.returnUrl_ << "\n";
		std::cout << "    Return Status: " << servers_[i].reqConfig_.returnStatus_ << "\n";
		std::cout << "    Root: " << servers_[i].reqConfig_.root_ << "\n";
		std::cout << "    Index File: " << servers_[i].reqConfig_.indexFile_ << "\n";
		std::cout << "    Upload Path: " << servers_[i].reqConfig_.uploadPath_ << "\n";
		std::cout << "    CGI Extension: " << servers_[i].reqConfig_.cgiExtension_ << "\n";

		std::cout << "    Client Max Body Size: ";
		if (servers_[i].reqConfig_.clientMaxBodySize_.isSet())
			std::cout << servers_[i].reqConfig_.clientMaxBodySize_.value();
		else
			std::cout << "N/A";
		std::cout << "\n";

		std::cout << "    Auto Index: ";
		if (servers_[i].reqConfig_.autoIndex_.isSet())
			std::cout << (servers_[i].reqConfig_.autoIndex_.value() ? "on" : "off");
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
				for (size_t k = 0; k < servers_[i].locations_[j].reqConfig_.methods_.size(); k++) {
					std::cout << servers_[i].locations_[j].reqConfig_.methods_[k];
					if (k < servers_[i].locations_[j].reqConfig_.methods_.size() - 1)
						std::cout << ", ";
				}
				std::cout << "\n";

				std::cout << "        Error Pages:\n";
				for (std::map<int, std::string>::const_iterator it = servers_[i].locations_[j].reqConfig_.errorPages_.begin();
					 it != servers_[i].locations_[j].reqConfig_.errorPages_.end(); ++it) {
					std::cout << "          " << it->first << " -> " << it->second << "\n";
				}
				std::cout << "        Return URL: " << servers_[i].locations_[j].reqConfig_.returnUrl_ << "\n";
				std::cout << "        Return Status: " << servers_[i].locations_[j].reqConfig_.returnStatus_ << "\n";
				std::cout << "        Root: " << servers_[i].locations_[j].reqConfig_.root_ << "\n";
				std::cout << "        Index File: " << servers_[i].locations_[j].reqConfig_.indexFile_ << "\n";
				std::cout << "        Upload Path: " << servers_[i].locations_[j].reqConfig_.uploadPath_ << "\n";
				std::cout << "        CGI Extension: " << servers_[i].locations_[j].reqConfig_.cgiExtension_ << "\n";

				std::cout << "        Client Max Body Size: ";
				if (servers_[i].locations_[j].reqConfig_.clientMaxBodySize_.isSet())
					std::cout << servers_[i].locations_[j].reqConfig_.clientMaxBodySize_.value();
				else
					std::cout << "N/A";
				std::cout << "\n";

				std::cout << "        Auto Index: ";
				if (servers_[i].locations_[j].reqConfig_.autoIndex_.isSet())
					std::cout << (servers_[i].locations_[j].reqConfig_.autoIndex_.value() ? "on" : "off");
				else
					std::cout << "on";
				std::cout << "\n";
			}
		}
		std::cout << "\n";
	}
}