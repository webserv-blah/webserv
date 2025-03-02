IMAGE_NAME = webserv
CONTAINER_NAME = webserv

test:
	docker build -t $(IMAGE_NAME) .
#   이미 동일 이름의 컨테이너가 있으면 중지 후 제거
	@docker stop $(CONTAINER_NAME) 2&>/dev/null || true
	@docker rm $(CONTAINER_NAME) 2&>/dev/null || true
#   새로 컨테이너 실행
	docker run --name $(CONTAINER_NAME) -d $(IMAGE_NAME)

test_log:
	docker logs $(CONTAINER_NAME)

test_re:
	@$(MAKE) test_clean
	@$(MAKE) test

test_clean:
	docker stop $(CONTAINER_NAME) 2&>/dev/null || true
	docker rm $(CONTAINER_NAME) 2&>/dev/null || true
	docker image rm -f $(IMAGE_NAME) 2&>/dev/null || true
