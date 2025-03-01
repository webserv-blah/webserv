# DEBUG_LOG 사용 가이드

## 개요
`DEBUG_LOG`는 webserv 프로젝트에서 디버깅 정보를 로깅하기 위한 매크로입니다. 이 매크로는 디버그 빌드에서만 활성화되며, 릴리스 빌드에서는 자동으로 비활성화됩니다.

## 사용법

```cpp
DEBUG_LOG("[컴포넌트명] 메시지");
```

### 예시
```cpp
DEBUG_LOG("[ClientSession] New client connected: " << clientId);
DEBUG_LOG("[RequestParser] Parsing HTTP request: " << requestLine);
DEBUG_LOG("[ConfigParser] Loading configuration from: " << configPath);
```

## 특징
- `DEBUG` 매크로가 정의된 경우에만 로그가 출력됩니다 (`make DEBUG=1`로 빌드 시)
- 로그는 `std::clog`를 통해 표준 에러(stderr)로 출력됩니다
- 세미콜론(;)을 끝에 추가하지 않습니다 - 매크로 정의에 이미 포함되어 있습니다
- 스트림 연산자(<<)를 사용하여 다양한 타입의 데이터를 출력할 수 있습니다

## 가이드라인
- 로그 메시지 앞에 항상 `[컴포넌트명]` 형식의 접두사를 붙여 로그 출처를 명확히 합니다
- 간결하면서도 충분한 정보를 포함하는 메시지를 작성합니다
- 디버깅용 추가 계산이나 임시 변수는 `#ifdef DEBUG` / `#endif` 블록으로 감싸주세요:

```cpp
#ifdef DEBUG
    std::string debugInfo = calculateDebugInfo();
    DEBUG_LOG("[ServerManager] 디버그 정보: " << debugInfo);
#endif
```

- 단순 `DEBUG_LOG` 호출은 매크로 자체가 이미 조건부로 정의되어 있으므로 추가 `#ifdef DEBUG` 블록이 필요하지 않습니다

## 빌드 방법
디버그 로깅을 활성화하려면 다음 명령으로 빌드하세요:
```
make DEBUG=1
```

일반(릴리스) 빌드에서는 모든 `DEBUG_LOG` 호출이 컴파일러에 의해 제거됩니다:
```
make
```