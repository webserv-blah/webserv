FROM ubuntu:latest

RUN apt update && \
    apt install -y \
    g++ \
    make \
    libkqueue-dev \
    python3

WORKDIR /app

COPY . .

# 서버 실행 파일을 환경 변수로 설정
ENV TESTER="./ubuntu_tester http://localhost:8080"
ENV SERVER="./webserv config/42tester.conf"

# 컨테이너 실행 시 서버를 실행하도록 변경
RUN make DEBUG=1

ENTRYPOINT [ "sh", "-c", "./webserv config/simple_config.conf" ]
