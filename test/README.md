# Webserv 테스트 프레임워크

## 개요

이 테스트 프레임워크는 webserv 프로젝트의 단위 테스트를 위한 간단한 C++98 호환 프레임워크입니다.

## 디렉토리 구조

```
test/
├── unit/               # 단위 테스트
│   ├── TestFramework.hpp  # 테스트 프레임워크 정의
│   ├── test_runner.cpp    # 테스트 실행기
│   ├── test_*.cpp         # 각 모듈별 테스트 파일
│   └── Makefile           # 테스트용 Makefile
└── README.md           # 이 문서
```

## 사용 방법

### 테스트 실행

```bash
# 테스트 디렉토리로 이동
cd test/unit

# 테스트 빌드 및 실행
make run
```

### 새 테스트 추가

1. `test/unit/` 디렉토리에 `test_<모듈명>.cpp` 파일을 생성합니다.
2. 다음 형식으로 테스트 케이스를 작성합니다:

```cpp
#include "TestFramework.hpp"
#include "../../src/<경로>/<모듈>.hpp"

// 테스트 스위트 선언
TEST_SUITE(모듈명)

// 테스트 케이스 작성
TEST_CASE(테스트명, 모듈명) {
    // 테스트 내용 작성
    ASSERT_EQ(예상값, 실제값);
    ASSERT_TRUE(조건);
    // ...
}
```

3. Makefile에서 테스트할 모듈의 소스 파일을 활성화합니다:
   - Makefile의 `PROJECT_SRCS` 변수에서 테스트할 모듈의 소스 파일 주석을 해제합니다.
   - 예를 들어, ConfigParser 모듈을 테스트하려면 다음 라인의 주석을 해제합니다:
     ```make
     PROJECT_SRCS += $(SRC_DIR)/ConfigParser/ConfigParser.cpp \
                     $(SRC_DIR)/ConfigParser/get_next_token.cpp
     ```

## 예외 처리 테스트 패턴

함수가 예외를 던지는지 확인하려면 다음 패턴을 사용하세요:

```cpp
bool exceptionThrown = false;
try {
    // 예외를 던질 것으로 예상되는 함수 호출
    someFunction();
} catch (const std::exception&) {
    exceptionThrown = true;
}
ASSERT_TRUE(exceptionThrown); // 예외가 발생했는지 확인
```

## 제공되는 Assertion 매크로

- `ASSERT_TRUE(condition)`: 조건이 참이어야 함
- `ASSERT_FALSE(condition)`: 조건이 거짓이어야 함
- `ASSERT_EQ(expected, actual)`: 두 값이 같아야 함
- `ASSERT_NE(expected, actual)`: 두 값이 달라야 함

## 테스트 실행 예시

```
=== Running Test Suite: Utils ===
Running test [StringToInt]... PASSED
Running test [StringToSizeT]... PASSED
Running test [StringTrim]... PASSED
Running test [NumberToString]... PASSED
Running test [AllOf]... PASSED
Running test [ErrorMessage]... PASSED

Results: 6 passed, 0 failed

=== Test Summary ===
All tests PASSED!
```