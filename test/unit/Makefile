CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Directories
SRC_DIR = ../../src
TEST_DIR = .

# Include paths
INCLUDES = -I. -I$(SRC_DIR) -I$(SRC_DIR)/include \
           -I$(SRC_DIR)/ClientManager -I$(SRC_DIR)/ClientSession \
           -I$(SRC_DIR)/ConfigParser -I$(SRC_DIR)/Demultiplexer \
           -I$(SRC_DIR)/EventHandler -I$(SRC_DIR)/GlobalConfig \
           -I$(SRC_DIR)/RequestHandler -I$(SRC_DIR)/RequestMessage \
           -I$(SRC_DIR)/RequestParser -I$(SRC_DIR)/ResponseBuilder \
           -I$(SRC_DIR)/ServerManager -I$(SRC_DIR)/TimeoutHandler \
           -I$(SRC_DIR)/utils

# Test framework and test cases
TEST_RUNNER = $(TEST_DIR)/test_runner.cpp
TEST_CASES = $(filter-out $(TEST_RUNNER), $(wildcard $(TEST_DIR)/test_*.cpp))
TEST_SRCS = $(TEST_RUNNER) $(TEST_CASES)

# Project source files by module
# Only include modules that are needed for the current tests

# Utils module - Required for test_utils.cpp
PROJECT_SRCS = $(SRC_DIR)/utils/utils.cpp \
               $(SRC_DIR)/utils/file_utils.cpp \
               $(SRC_DIR)/utils/enumToStr.cpp

# Object files
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
PROJECT_OBJS = $(PROJECT_SRCS:.cpp=.o)
OBJS = $(TEST_OBJS) $(PROJECT_OBJS)

# Executable
TARGET = webserv_tests

# Color definitions
GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

# Targets
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

run_kqueue: all
	@echo "$(GREEN)Running kqueue closed fd test with verbose output...$(RESET)"
	./$(TARGET) KqueueClosedFD

.PHONY: all clean fclean re run run_kqueue