#include "file_utils.hpp"
#include "../include/errorUtils.hpp"

namespace FileUtilities {
	EnumValidationResult validatePath(const std::string& path)
	{
		// 1) stat으로 파일 정보를 가져온다
		struct stat st;

		if (stat(path.c_str(), &st) != 0) {
			DEBUG_LOG("[FileUtilities::validatePath]stat failed: " + path);

			// 경로의 실행 권한이 없는 경우
			if (errno == EACCES) {
				webserv::logError(WARNING, "Permission denied", 
								 path, 
								 "FileUtilities::validatePath, errno: " + utils::size_t_tos(errno));
				return PATH_NO_PERMISSION;
			}

			// 경로가 존재하지 않는 경우
			// "마지막 문자가 '/'이면 디렉터리로 간주, 아니면 파일"로 구분
			if (!path.empty() && path[path.size() - 1] == '/') {
				webserv::logError(WARNING, "Path not found", 
				                 path, 
				                 "FileUtilities::validatePath, errno: " + utils::size_t_tos(errno));
				return PATH_NOT_FOUND;
			} else {
				webserv::logError(WARNING, "File not found", 
				                 path, 
				                 "FileUtilities::validatePath, errno: " + utils::size_t_tos(errno));
				return FILE_NOT_FOUND;
			}
		}

		// 2) 경로가 디렉터리인지 확인
		if (S_ISDIR(st.st_mode)) {
			// 접근 권한의 확인은 stat() 호출의 반환값으로 위에서 확인
			return VALID_PATH;
		}
		// 3) 경로가 정규파일인지 확인
		else if (S_ISREG(st.st_mode)) {
			// 먼저 읽기 권한(R_OK) 체크
			if (access(path.c_str(), R_OK) != 0) {
				return FILE_NO_READ_PERMISSION;
			}
			// 다음으로 실행 권한(X_OK) 체크
			// if (access(path.c_str(), X_OK) != 0) {
			// 	return FILE_NO_EXEC_PERMISSION;
			// }
			return VALID_FILE;
		}
		// 4) 그 밖의 유형(심볼릭 링크, 소켓, 파이프 등)은 "정규파일이 아니다"로 처리
		else {
			return FILE_NOT_FOUND;
		}
	}

	// 파일 경로의 확장자가 주어진 확장자와 일치하는지 확인하는 함수
	bool hasExtension(const std::string& path, const std::string& extension) {
		if (path.size() < extension.size()) {
			return false;
		}
		return path.compare(path.size() - extension.size(), extension.size(), extension) == 0;
	}

	// 디렉토리가 존재하는지 확인하는 함수
	bool isDirectory(const char* path) {
		struct stat st;
		if (stat(path, &st) != 0) {
			return false;
		}
		return S_ISDIR(st.st_mode);
	}

	// 파일의 실행 권한을 확인하는 함수
	bool hasExecutePermission(const std::string& filePath) {
		if (access(filePath.c_str(), X_OK) == 0) {
			return true;
		} else {
			webserv::logError(WARNING, "No execute permission", 
			                 filePath, 
			                 "FileUtilities::hasExecutePermission, errno: " + utils::size_t_tos(errno));
			return false;
		}
	}

	// 파일을 읽어 문자열로 반환하는 함수
	std::string readFile(const std::string &filePath) {
		// 파일을 바이너리 모드로 열고, 파일 끝에 위치하여 크기를 측정
		std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

		// 파일을 열지 못한 경우 에러 로깅 후 빈 문자열 반환
		if (!file.is_open()) {
			webserv::logError(ERROR, "File opening failed", 
			                 filePath, 
			                 "FileUtilities::readFile, errno: " + utils::size_t_tos(errno));
			return "";
		}

		// 파일 크기를 얻어옴
		std::ifstream::pos_type fileSize = file.tellg();
		// 파일 포인터를 파일의 시작 위치로 이동
		file.seekg(0, std::ios::beg);

		// 파일 크기가 음수인 경우(비정상적인 상태) 에러 로깅 후 빈 문자열 반환
		if (fileSize < 0) {
			webserv::logError(ERROR, "Invalid file size", 
			                 filePath + ", size: " + utils::size_t_tos(fileSize), 
			                 "FileUtilities::readFile");
			return "";
		}

		// 파일 크기만큼의 버퍼를 생성
		std::vector<char> buffer(static_cast<size_t>(fileSize));

		// 파일을 읽고, 실패 시 에러 로깅 후 빈 문자열 반환
		if (!file.read(&buffer[0], fileSize)) {
			webserv::logError(ERROR, "File read failed", 
			                 filePath + ", requested size: " + utils::size_t_tos(fileSize), 
			                 "FileUtilities::readFile");
			return "";
		}

		// 버퍼를 문자열로 변환하여 반환
		return std::string(buffer.begin(), buffer.end());
	}

	// 경로가 절대경로인지 판별 (Linux/Unix 전용)
	bool isAbsolutePath(const std::string &path) {
		return (!path.empty() && path[0] == '/');
	}

	// 경로 정규화 함수: 불필요한 "."와 ".."를 제거
	std::string normalizePath(const std::string &path) {
		std::vector<std::string> stack;
		std::istringstream iss(path);
		std::string token;
		
		// '/'를 구분자로 토큰 분리
		while (std::getline(iss, token, '/')) {
			if (token.empty() || token == ".") {
				continue;
			} else if (token == "..") {
				if (!stack.empty()) {
					stack.pop_back();
				}
				// 절대경로의 경우, stack이 비면 ".."를 무시합니다.
			} else {
				stack.push_back(token);
			}
		}
		
		std::string result;
		if (isAbsolutePath(path)) {
			result = "/";
		}
		
		for (size_t i = 0; i < stack.size(); ++i) {
			result += stack[i];
			if (i != stack.size() - 1)
				result += "/";
		}
		
		return result.empty() ? "/" : result;
	}

	// 상대경로를 절대경로로 변환 (Linux/Unix 전용)
	std::string relativeToAbsolute(const std::string &path) {
		if (isAbsolutePath(path))
			return normalizePath(path);

		char buffer[PATH_MAX];
		if (getcwd(buffer, PATH_MAX) == NULL) {
			throw std::runtime_error("getcwd() 호출 중 오류 발생");
		}
		std::string currentDir(buffer);
		if (currentDir.empty() || currentDir[currentDir.size()-1] != '/')
			currentDir += '/';
		
		return normalizePath(currentDir + path);
	}

	std::string joinPaths(const std::string& root, const std::string& path) {
		if (root.empty()) {
			return path;	
		}
	
		bool rootHasTrailingSlash = (!root.empty() && root[root.length()-1] == '/');
		bool pathHasLeadingSlash = (!path.empty() && path[0] == '/');
		
		if (rootHasTrailingSlash && pathHasLeadingSlash) {
			// If root ends with '/' and path starts with '/', remove one
			return root + path.substr(1);
		} else if (!rootHasTrailingSlash && !pathHasLeadingSlash) {
			// If neither has a slash, add one
			return root + "/" + path;
		} else {
			// One of them has the needed slash
			return root + path;
		}
	}

}