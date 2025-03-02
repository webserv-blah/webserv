FROM ubuntu:latest

RUN apt update && \
    apt install -y \
    g++ \
    make \
    libkqueue-dev \
    php-cgi

WORKDIR /app

COPY . .

ENV TESTER="./ubuntu_tester http://localhost:8080"
ENV SERVER="./webserv config/42tester.conf"

ENTRYPOINT [ "sh", "-c", "make && make clean" ]
CMD [ "tail", "-f", "/dev/null" ]