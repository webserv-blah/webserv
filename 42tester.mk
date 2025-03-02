IMAGE_NAME		= webserv
CONTAINER_NAME	= webserv
PORT_HOST		= 8080
PORT_CONTAINER	= 8080

tgo:
	docker exec -it $(CONTAINER_NAME) /bin/bash

t: ubuntu_tester
	docker build --platform=linux/amd64 -t $(IMAGE_NAME) .
#   이미 동일 이름의 컨테이너가 있으면 중지 후 제거
	@docker stop $(CONTAINER_NAME) 2&>/dev/null || true
	@docker rm $(CONTAINER_NAME) 2&>/dev/null || true
#   새로 컨테이너 실행
	docker run --platform=linux/amd64 -p $(PORT_HOST):$(PORT_CONTAINER) --name $(CONTAINER_NAME) -d $(IMAGE_NAME)

tlog:
	docker logs $(CONTAINER_NAME)

tre:
	@$(MAKE) tclean
	@$(MAKE) t

tclean:
	docker stop $(CONTAINER_NAME) 2&>/dev/null || true
	docker rm $(CONTAINER_NAME) 2&>/dev/null || true
	docker image rm -f $(IMAGE_NAME) 2&>/dev/null || true
