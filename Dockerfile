FROM ubuntu:latest

RUN apt update && \
    apt install -y \
    g++ \
    make \
    libkqueue-dev \
    php-cgi

WORKDIR /app

COPY . .

CMD [ "sh", "-c", "make && ./webserv ./config/42tester.conf && make fclean" ]