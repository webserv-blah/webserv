FROM ubuntu:latest

RUN apt update && \
    apt install -y \
    g++ \
    make \
    libkqueue-dev \
    php-cgi

WORKDIR /app

COPY . .

RUN make && make clean

ENV TESTER="./ubuntu_tester http://localhost:8080"
ENV SERVER="./webserv config/42tester.conf"

CMD [ "tail", "-f", "/dev/null" ]