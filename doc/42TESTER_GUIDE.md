# 42 Tester Guide

이 문서는 webserv 프로젝트의 42 테스터 사용 방법을 안내합니다.

## 개요

42 테스터는 Docker를 사용하여 Ubuntu 환경에서 webserv를 빌드하고, 테스트할 수 있는 도구입니다. 이 테스터는 42 과제 평가 환경과 유사한 환경에서 프로젝트가 동작하는지 확인하는 데 유용합니다.

## 사전 요구사항

- Docker가 설치되어 있어야 합니다.
- 포트 8080이 사용 가능해야 합니다.

## 42intra ubuntu_tester 사용법

### 설정 및 실행 방법

1. 42intra Webserv 페이지에서 ubuntu_tester 다운로드
2. 다운로드한 파일을 프로젝트 루트 디렉토리에 위치시킴
3. 실행 권한 부여:
   ```
   chmod 755 ubuntu_tester
   ```
4. 테스트 컨테이너 빌드:
   ```
   make t
   ```
5. 두 개의 터미널 창을 준비
6. 첫 번째 터미널에서:
   ```
   make tgo
   SERVER
   ```
7. 두 번째 터미널에서:
   ```
   make tgo
   TESTER
   ```

## 테스터 설정 파일

### Dockerfile
Docker 이미지 구성:
- Ubuntu 최신 버전 기반
- 필요 패키지 설치: g++, make, libkqueue-dev, php-cgi
- 환경 변수:
  - `TESTER="./ubuntu_tester http://localhost:8080"`
  - `SERVER="./webserv config/42tester.conf"`

### 테스트 설정 파일 (config/42tester.conf)
- 42 테스트 요구사항에 맞춘 설정 파일
- 다음 기능 테스트를 위한 설정 포함:
  - 포트 80 및 8080에서 서버 실행
  - YoupiBanane 디렉토리 설정
  - 오류 페이지 설정
  - 업로드 경로 설정
  - CGI 확장자 설정 (.php)
  - 리다이렉션 설정

## 사용법

### 테스터 명령어

테스터는 Make 명령을 통해 간편하게 사용할 수 있습니다:

1. **테스터 생성 및 실행**
   ```
   make -f 42tester.mk t
   ```
   - Docker 이미지를 빌드하고 컨테이너를 실행합니다.
   - 포트 8080을 호스트와 컨테이너 간에 매핑합니다.

2. **테스터 컨테이너 접속**
   ```
   make -f 42tester.mk tgo
   ```
   - 실행 중인 테스터 컨테이너에 bash 쉘로 접속합니다.

3. **테스터 로그 확인**
   ```
   make -f 42tester.mk tlog
   ```
   - 컨테이너의 로그를 확인합니다.

4. **테스터 재시작**
   ```
   make -f 42tester.mk tre
   ```
   - 기존 테스터를 제거하고 새로 생성합니다.

5. **테스터 제거**
   ```
   make -f 42tester.mk tclean
   ```
   - 컨테이너와 이미지를 모두 제거합니다.

### 테스트 실행

1. 테스터 컨테이너에 접속합니다:
   ```
   make -f 42tester.mk tgo
   ```

2. 컨테이너 내부에서 서버와 테스터를 실행합니다:
   ```
   # 서버 실행 (백그라운드)
   $SERVER &
   
   # 테스터 실행
   $TESTER
   ```

## 주의사항

- 테스터 실행 전 로컬에서 포트 8080이 사용 중이지 않은지 확인하세요.
- 테스트 설정 파일(42tester.conf)을 수정하면 테스트 결과가 달라질 수 있으니 주의하세요.

## 문제 해결

1. 이미 존재하는 컨테이너 오류
   - `make -f 42tester.mk tclean` 실행 후 다시 시도

2. 포트 충돌 오류
   - 로컬에서 실행 중인 서버나 다른 응용 프로그램 종료
   - 42tester.mk 파일에서 PORT_HOST 변수 수정