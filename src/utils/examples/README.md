# 에러 처리 표준화 가이드

## 소개
이 가이드는 웹서브 프로젝트에서 사용할 표준화된 에러 메시지 형식과 처리 방식을 설명합니다.

## 에러 메시지 형식
모든 에러 메시지는 다음 형식을 따릅니다:
```
[Error Level] : [Error Cause] - [Context/Condition] (Source: [Source Info])
```

- **[Error Level]**: 오류의 심각도
  - **WARNING**: 경고 (계속 진행 가능)
  - **ERROR**: 오류 (복구 필요)
  - **FATAL**: 치명적 오류 (프로그램 종료)

- **[Error Cause]**: 문제의 핵심 원인
- **[Context/Condition]**: 에러 발생 맥락이나 조건
- **[Source Info]**: 에러의 근원지 (함수명, errno, 시스템 콜 등)

## 사용 방법

### 1. 필요한 헤더 포함
```cpp
#include "../include/errorUtils.hpp"
```

### 2. 일반 에러 로깅
```cpp
// WARNING 레벨 에러
webserv::logError(WARNING, "Invalid value", "Input is negative", "validateInput function");

// ERROR 레벨 에러
webserv::logError(ERROR, "File opening failed", "test.txt", 
                 "errno " + std::to_string(errno) + ", " + std::strerror(errno));
```

### 3. 시스템 호출 에러 로깅 (errno 자동 처리)
```cpp
// WARNING 레벨 에러
webserv::logSystemError(WARNING, "recv failed", 
			                 "Client fd: " + std::to_string(curSession.getClientFd()), 
			                 "EventHandler::recvRequest");
// ERROR 레벨 에러
webserv::logSystemError(ERROR, "fopen", "test.txt");
```

### 4. 예외 발생 (FATAL 에러)
```cpp
webserv::throwError("Missing required configuration", "server_name directive", "ConfigParser::parse");
```

### 5. 시스템 호출 예외 발생 (errno 자동 처리)
```cpp
webserv::throwSystemError("malloc", "Requested size: 1000000");
```

## 에러 레벨 선택 가이드

- **WARNING**: 프로그램 실행에 영향이 없는 경미한 문제
  - 예) 경계값 검사 실패, 비권장 설정 사용, 잠재적 문제점

- **ERROR**: 특정 기능이나 요청 처리에 실패했지만 서버는 계속 실행 가능
  - 예) 파일 열기 실패, 요청 처리 실패, 잘못된 입력값

- **FATAL**: 서버 실행을 계속할 수 없는 심각한 문제
  - 예) 설정 파일 파싱 실패, 포트 바인딩 실패, 중요 리소스 할당 실패