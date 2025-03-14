# py CGI 예제 스크립트

이 디렉토리는 웹서버와 CGI 기능을 시연하는 py 스크립트 예제를 포함하고 있습니다.

## 설치 지침

1. 시스템에 py CGI가 설치되어 있는지 확인:
   - macOS: `brew install python3`
   - Ubuntu: `sudo apt install python3`

2. 스크립트에 실행 권한 부여:
   ```
   chmod +x html/cgi-bin/*.py
   ```

3. 필요한 경우 각 py 스크립트의 첫 줄(shebang)이 py-CGI 실행 파일을 가리키도록 업데이트:
   ```
   #!/Users/damin/francinette/venv/bin/python3    # 시스템 경로에 맞게 수정
   ```

4. 제공된 `config_example.conf` 설정 사용:
   ```
   ./webserv config_example.conf
   ```

5. 다음 주소로 예제 접근:
   - http://localhost:8080/ - 모든 예제 링크가 있는 메인 인덱스
   - http://localhost:8080/cgi-bin/hello.py - 간단한 인사말 스크립트
   - http://localhost:8080/cgi-bin/info.py - 서버/요청 정보
   - http://localhost:8080/cgi-bin/form.py - 폼 처리 예제

## 포함된 예제

### hello.py
인사말을 표시하는 간단한 스크립트. 기본 GET 매개변수 처리를 보여줍니다.

### info.py
환경 변수, 요청 헤더 및 기타 CGI 정보를 표시합니다. 디버깅에 유용합니다.

### form.py
py에서 POST 방식을 사용한 폼 처리를 보여줍니다.

## 문제 해결

- **500 Internal Server Error**: py-CGI가 설치되어 있고 shebang 라인의 경로가 올바른지 확인
- **실행 권한**: 모든 .py 파일에 실행 권한이 있는지 확인
- **CGI 설정**: 서버 설정에 `cgi_extension .py`가 올바르게 설정되어 있는지 확인
- **경로 문제**: 스크립트 경로가 서버에 구성된 경로와 일치하는지 확인