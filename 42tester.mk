IMAGE_NAME = webserv
CONTAINER_NAME = webserv

test:
	docker build -t $(IMAGE_NAME) .
	docker run --name $(CONTAINER_NAME) -d $(IMAGE_NAME)

test_log:
	docker logs $(CONTAINER_NAME)

test_re:
	@$(MAKE) test_clean
	@$(MAKE) test

test_clean:
	docker stop $(CONTAINER_NAME)
	docker rm $(CONTAINER_NAME)
	docker image rm -f $(IMAGE_NAME)
