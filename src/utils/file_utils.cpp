#include "file_utils.hpp"
#include "../include/errorUtils.hpp"

namespace FileUtilities {
	EnumValidationResult validatePath(const std::string& path)
	{
		// 1) stat으로 파일 정보를 가져온다
		struct stat st;

		if (stat(path.c_str(), &st) != 0) {
			// 경로가 존재하지 않는 경우
			// "마지막 문자가 '/'이면 디렉터리로 간주, 아니면 파일"로 구분
			if (!path.empty() && path[path.size() - 1] == '/') {
				webserv::logError(webserv::WARNING, "Path not found", 
				                 path, 
				                 "FileUtilities::validatePath, errno: " + std::to_string(errno));
				return PATH_NOT_FOUND;
			} else {
				webserv::logError(webserv::WARNING, "File not found", 
				                 path, 
				                 "FileUtilities::validatePath, errno: " + std::to_string(errno));
				return FILE_NOT_FOUND;
			}
		}

		// 2) 경로가 디렉터리인지 확인
		if (S_ISDIR(st.st_mode)) {
			// 디렉터리 접근 권한(검색 권한, X_OK) 체크
			if (access(path.c_str(), X_OK) != 0) {
				return PATH_NO_PERMISSION;
			}
			return VALID_PATH;
		}
		// 3) 경로가 정규파일인지 확인
		else if (S_ISREG(st.st_mode)) {
			// 먼저 읽기 권한(R_OK) 체크
			if (access(path.c_str(), R_OK) != 0) {
				return FILE_NO_READ_PERMISSION;
			}
			// 다음으로 실행 권한(X_OK) 체크
			if (access(path.c_str(), X_OK) != 0) {
				return FILE_NO_EXEC_PERMISSION;
			}
			return VALID_FILE;
		}
		// 4) 그 밖의 유형(심볼릭 링크, 소켓, 파이프 등)은 "정규파일이 아니다"로 처리
		else {
			return FILE_NOT_FOUND;
		}
	}

	// 파일을 읽어 문자열로 반환하는 함수
	std::string readFile(const std::string &filePath) {
		// 파일을 바이너리 모드로 열고, 파일 끝에 위치하여 크기를 측정
		std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

		// 파일을 열지 못한 경우 에러 로깅 후 빈 문자열 반환
		if (!file.is_open()) {
			webserv::logError(webserv::ERROR, "File opening failed", 
			                 filePath, 
			                 "FileUtilities::readFile, errno: " + std::to_string(errno));
			return "";
		}

		// 파일 크기를 얻어옴
		std::ifstream::pos_type fileSize = file.tellg();
		// 파일 포인터를 파일의 시작 위치로 이동
		file.seekg(0, std::ios::beg);

		// 파일 크기가 음수인 경우(비정상적인 상태) 에러 로깅 후 빈 문자열 반환
		if (fileSize < 0) {
			webserv::logError(webserv::ERROR, "Invalid file size", 
			                 filePath + ", size: " + std::to_string(fileSize), 
			                 "FileUtilities::readFile");
			return "";
		}

		// 파일 크기만큼의 버퍼를 생성
		std::vector<char> buffer(static_cast<size_t>(fileSize));

		// 파일을 읽고, 실패 시 에러 로깅 후 빈 문자열 반환
		if (!file.read(&buffer[0], fileSize)) {
			webserv::logError(webserv::ERROR, "File read failed", 
			                 filePath + ", requested size: " + std::to_string(fileSize), 
			                 "FileUtilities::readFile");
			return "";
		}

		// 버퍼를 문자열로 변환하여 반환
		return std::string(buffer.begin(), buffer.end());
	}
}
