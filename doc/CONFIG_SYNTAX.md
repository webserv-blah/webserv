# Webserv 설정 파일 예제

아래 예제는 Webserv에서 사용 가능한 모든 지시어를 포함하는 종합적인 설정 파일입니다.

```
# 기본 서버 설정
server {
    # 서버가 수신할 IP 주소와 포트
    listen 127.0.0.1:8080;
    
    # 서버 이름 설정
    server_name example.com www.example.com;
    
    # 클라이언트 요청 본문 최대 크기 (10MB)
    client_max_body_size 10m;
    
    # 기본 오류 페이지 설정
    error_page 404             /error/404.html;
    error_page 500 502 503 504 /error/50x.html;
    
    # 루트 디렉토리 설정
    root /var/www/html;
    
    # 기본 위치 블록
    location / {
        # 기본 인덱스 파일
        index index.html;
        
        # 디렉토리 목록 표시 비활성화
        autoindex off;
        
        # 허용할 HTTP 메소드
        methods GET POST;
    }
    
    # 정적 파일 서빙을 위한 위치 블록
    location /static/ {
        root /var/www;
        autoindex on;
        methods GET;
    }
    
    # 파일 업로드를 위한 위치 블록
    location /upload {
        methods POST;
        upload_path /tmp/uploads;
        client_max_body_size 20m;  # 이 위치에서만 20MB 허용
    }
    
    # CGI 스크립트를 위한 위치 블록
    location /cgi-bin/ {
        root /var/www;
        cgi_extension .php;
        methods GET POST;
    }
    
    # 리다이렉션 예제
    location /old-page {
        return 301 /new-page;
    }
}

# 다른 포트로 수신하는 서버
server {
    listen 127.0.0.1:8081;
    server_name secondary.example.com;
    root /var/www/secondary;
    
    location / {
        index index.html;
        methods GET;
    }
}
```

## 지시어 요약

| 지시어 | 컨텍스트 | 기본값 | 설명 |
|--------|----------|--------|------|
| `server {}` | - | - | 가상 서버 정의 |
| `listen` | server | *:80 | 수신할 주소와 포트 설정 |
| `server_name` | server | "" | 가상 서버 이름 설정 |
| `location {}` | server | - | URI별 설정 블록 정의 |
| `error_page` | server, location | - | 오류 페이지 지정 |
| `client_max_body_size` | server, location | 1m | 최대 요청 본문 크기 |
| `return` | server, location | - | 리다이렉션 응답 생성 |
| `root` | server, location | html | 요청의 루트 디렉토리 지정 |
| `autoindex` | server, location | off | 디렉토리 목록 표시 설정 |
| `index` | server, location | index.html | 기본 인덱스 파일 설정 |
| `upload_path` | server, location | - | 업로드 파일 저장 위치 |
| `cgi_extension` | server, location | - | CGI 처리할 파일 확장자 |
| `methods` | location | GET POST DELETE | 허용할 HTTP 메소드 지정 |