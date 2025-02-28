#ifndef COMMON_ENUMS_HPP
#define COMMON_ENUMS_HPP

typedef enum EnumSessionStatus {
	REQUEST_ERROR,
	READ_CONTINUE,
	READ_COMPLETE,
	WRITE_CONTINUE,
	WRITE_COMPLETE,
	CONNECTION_CLOSED
} EnumSesStatus;

enum EnumEvent {
	READ_EVENT,
	WRITE_EVENT,
	UNKNOWN_EVENT
};

enum EnumStatusCode {
	// RequestParser의 결과 정상상태
	NONE_STATUS_CODE = 0,
	// 2xx, Successful
	OK = 200,
	// 3xx, Redirection
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	// 4xx, Client Error
	BAD_REQUEST = 400,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	REQUEST_TIMEOUT = 408,
	CONTENT_TOO_LARGE = 413,
	URI_TOO_LONG = 414,
	// 5xx, Server Error
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	SERVICE_UNAVAILABLE = 503
};

enum EnumErrorLevel {
	WARNING, // 경고 (계속 진행 가능)
	ERROR,   // 오류 (복구 필요)
	FATAL    // 치명적 오류 (프로그램 종료)
};

#endif