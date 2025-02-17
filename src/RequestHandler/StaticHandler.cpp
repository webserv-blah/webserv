#include "StaticHandler.hpp"

std::string StaticHandler::handleRequest(const ReqMessage& reqMsg, const ReqHandleConf& currConf) {
	std::string uri = reqMsg.getTargetURI();
	std::string docRoot = currConf.root_;
	std::string filePath = docRoot + uri;

	// 공통 유틸 함수 사용
	FileValidationResult valRes = validateFile(filePath, true, false);
	ResponseBuilder responseBuilder(currConf);

	if (valRes != FILE_OK) {
		switch (valRes) {
			case FILE_NOT_FOUND:
				return responseBuilder.buildError(404);
			case FILE_NOT_REGULAR:
			case NO_READ_PERMISSION:
				return responseBuilder.buildError(403);
			default:
				return responseBuilder.buildError(500);
		}
	}

	// 파일 로딩
	std::string fileContent = readFile(filePath);
	if (fileContent.empty()) {
		// 읽기 실패 or 0바이트
		return responseBuilder.buildError(404);
	}

	// MIME
	std::string contentType = determineContentType(filePath);

	// 정상 응답
	responseBuilder.setStatus(200, "OK");
	responseBuilder.addHeader("Content-Type", contentType);
	responseBuilder.setBody(fileContent);

	return responseBuilder.build();
}

std::string StaticHandler::readFile(const std::string &filePath) const {
	std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		return "";
	}
	std::ifstream::pos_type fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	if (fileSize < 0) {
		return "";
	}

	std::vector<char> buffer(static_cast<size_t>(fileSize));
	if (!file.read(&buffer[0], fileSize)) {
		return "";
	}
	return std::string(buffer.begin(), buffer.end());
}

std::string StaticHandler::determineContentType(const std::string &filePath) const {
	// 단순 확장자 매핑
	std::string::size_type pos = filePath.find_last_of('.');
	if (pos == std::string::npos) {
		return "application/octet-stream";
	}
	std::string ext = filePath.substr(pos + 1);
	if (ext == "html" || ext == "htm") {
		return "text/html";
	}
	if (ext == "css") {
		return "text/css";
	}
	if (ext == "js") {
		return "application/javascript";
	}
	if (ext == "png") {
		return "image/png";
	}
	if (ext == "jpg" || ext == "jpeg") {
		return "image/jpeg";
	}
	if (ext == "gif") {
		return "image/gif";
	}
	return "application/octet-stream";
}
