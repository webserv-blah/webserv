#pragma once

#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <vector>

namespace fileUtilities {
	enum ValidationResult {
		VALID_PATH,         // 유효한 경로 (디렉터리) 이며 권한 OK
		PATH_NOT_FOUND,     // 디렉터리로 볼 수 있는 경로가 없음
		PATH_NO_PERMISSION, // 디렉터리가 존재하지만 접근 권한 없음
		
		VALID_FILE,         // 유효한 정규파일이며 권한 OK
		FILE_NOT_FOUND,     // 정규파일로 볼 수 있는 경로가 없음
		FILE_NO_READ_PERMISSION, // 파일이 존재하지만 읽기 권한 없음
		FILE_NO_EXEC_PERMISSION  // 파일이 존재하지만 쓰기 권한 없음
	};

	//  존재 / 권한 상태를 7가지 결과 중 하나로 반환
	ValidationResult	validatePath(const std::string& path);
	std::string			readFile(const std::string &filePath);
};
