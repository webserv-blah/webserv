CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
INCLUDES = -I../../src -I. -I../../src/include \
           -I../../src/ClientManager -I../../src/ClientSession \
           -I../../src/ConfigParser -I../../src/Demultiplexer \
           -I../../src/EventHandler -I../../src/GlobalConfig \
           -I../../src/RequestHandler -I../../src/RequestMessage \
           -I../../src/RequestParser -I../../src/ResponseBuilder \
           -I../../src/ServerManager -I../../src/TimeoutHandler \
           -I../../src/utils

# Main source files for project components to test
SRC_DIR = ../../src
TEST_DIR = .

# Project source files needed for tests
# Uncomment the modules you need to test

# Utils module
PROJECT_SRCS = $(SRC_DIR)/utils/utils.cpp
PROJECT_SRCS += $(SRC_DIR)/utils/file_utils.cpp

# Add ResponseBuilder for testing EventHandler
PROJECT_SRCS += $(SRC_DIR)/ResponseBuilder/ResponseBuilder.cpp
PROJECT_SRCS += $(SRC_DIR)/ResponseBuilder/ErrorPageResolver.cpp

# We're not using the actual EventHandler, just the ResponseBuilder
# PROJECT_SRCS += $(SRC_DIR)/EventHandler/EventHandler.cpp
# PROJECT_SRCS += $(SRC_DIR)/StaticHandler/StaticHandler.cpp
# PROJECT_SRCS += $(SRC_DIR)/CgiHandler/CgiHandler.cpp

# RequestParser module for testing
PROJECT_SRCS += $(SRC_DIR)/RequestParser/RequestParser.cpp
PROJECT_SRCS += $(SRC_DIR)/RequestParser/RequestParserStartLine.cpp
PROJECT_SRCS += $(SRC_DIR)/RequestParser/RequestParserFieldLine.cpp
PROJECT_SRCS += $(SRC_DIR)/RequestParser/RequestParserBody.cpp
PROJECT_SRCS += $(SRC_DIR)/RequestMessage/RequestMessage.cpp
PROJECT_SRCS += $(SRC_DIR)/ClientSession/ClientSession.cpp
PROJECT_SRCS += $(SRC_DIR)/GlobalConfig/GlobalConfig.cpp

# ClientManager module
#PROJECT_SRCS += $(SRC_DIR)/ClientManager/ClientManager.cpp

# ConfigParser module
PROJECT_SRCS += $(SRC_DIR)/ConfigParser/ConfigParser.cpp \
                $(SRC_DIR)/ConfigParser/get_next_token.cpp \
                $(SRC_DIR)/ConfigParser/parse_location_block.cpp \
                $(SRC_DIR)/ConfigParser/parse_server_block.cpp \
                $(SRC_DIR)/ConfigParser/parse_simple_directives.cpp

# Add other modules as needed

# Test source files
TEST_SRCS = $(TEST_DIR)/test_runner.cpp \
            $(wildcard $(TEST_DIR)/test_*.cpp)

# Object files
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
PROJECT_OBJS = $(PROJECT_SRCS:.cpp=.o)

# All objects
OBJS = $(TEST_OBJS) $(PROJECT_OBJS)

# Executable
TARGET = webserv_tests

# Color definitions
GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "$(GREEN)Linking $(TARGET)...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^
	@echo "$(GREEN)Build complete! Run ./$(TARGET) to execute tests.$(RESET)"

%.o: %.cpp
	@echo "$(GREEN)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "$(RED)Removing object files...$(RESET)"
	@rm -f $(TEST_OBJS) $(PROJECT_OBJS)

fclean: clean
	@echo "$(RED)Removing executable...$(RESET)"
	@rm -f $(TARGET)

re: fclean all

run: all
	@echo "$(GREEN)Running tests...$(RESET)"
	@./$(TARGET)

.PHONY: all clean fclean re run