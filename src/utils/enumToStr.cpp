#include "enumToStr.hpp"

namespace enumToStr {
	std::string EnumStatusCodeToStr(EnumStatusCode HttpStatusCode) {
		switch (HttpStatusCode) {
			case OK: return "OK";                     // 200: OK
			case MOVED_PERMANENTLY: return "Moved Permanently";				// 301: 영구적으로 이동됨
			case FOUND: return "Found";                  					// 302: 찾음 (임시 이동)
			case BAD_REQUEST: return "Bad Request";            				// 400: 잘못된 요청
			case FORBIDDEN: return "Forbidden";              				// 403: 접근 거부
			case NOT_FOUND: return "Not Found";        						// 404: 찾을 수 없음
			case METHOD_NOT_ALLOWED: return "Method Not Allowed";   		// 405: 허용되지 않은 메소드
			case REQUEST_TIMEOUT: return "Request Timeout";        			// 408: 요청 시간 초과
			case CONTENT_TOO_LARGE: return "Content Too Large";     		// 413: 요청 페이로드가 너무 큼
			case URI_TOO_LONG: return "Uri Too Large";     					// 413: 요청 페이로드가 너무 큼
			case INTERNAL_SERVER_ERROR: return "Internal Server Error"; 	// 500: 내부 서버 오류
			case NOT_IMPLEMENTED: return "Not Implemented";					// 501: 구현되지 않음
			case SERVICE_UNAVAILABLE: return "Service Unavailable";			// 503: 서비스 이용 불가
			case GATEWAY_TIMEOUT: return "Gateway Timeout";					// 504: 게이트웨이 시간 초과
			default:  return "Error";										// 그 외: 일반 오류
		}
	}

	std::string EnumSesStatusToStr(EnumSesStatus status) {
		switch (status) {
			case REQUEST_ERROR: return "REQUEST_ERROR";				// 요청 오류
			case READ_CONTINUE: return "READ_CONTINUE";				// 읽기 계속
			case READ_COMPLETE: return "READ_COMPLETE";				// 읽기 완료
			case WAIT_FOR_CGI: return "WAIT_FOR_CGI";				// CGI 작업 대기 중
			case WRITE_CONTINUE: return "WRITE_CONTINUE";			// 쓰기 계속
			case WRITE_COMPLETE: return "WRITE_COMPLETE";			// 쓰기 완료
			case CONNECTION_CLOSED: return "CONNECTION_CLOSED";		// 연결 종료
			default: return "Error";								// 그 외: 일반 오류
		}
	}
}
