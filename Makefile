
##
# webserv
#
# @file
# @version 0.1

# = ... lazy assignment
# variable is not calculated at the time of assignment
# evaluated every time the variable is used
# := (Simple or Immediate Assignment)
# variable is calculated and fixed at the time of assignment
# variable's value will not change even if the variables it depends on changes later

CXX := c++
CXXFLAGS := -std=c++98 -Wall -Werror -Wextra
DEBUG :=

SRC_DIR := src
OBJ_DIR := obj

BUILD_DIR := build
RUN_DIR := run
INCLUDES := -Iincludes -Iincludes/Interfaces
SRC_FILES := 	$(SRC_DIR)/Buffer.cpp \
				$(SRC_DIR)/ConfigParser.cpp \
				$(SRC_DIR)/ConfigParser2.cpp \
				$(SRC_DIR)/Connection.cpp \
				$(SRC_DIR)/ConnectionHandler.cpp \
				$(SRC_DIR)/DeleteHandler.cpp \
				$(SRC_DIR)/EpollIONotifier.cpp \
				$(SRC_DIR)/GetHandler.cpp \
				$(SRC_DIR)/UploadHandler.cpp \
				$(SRC_DIR)/DeleteHandler.cpp \
				$(SRC_DIR)/CgiHandler.cpp \
				$(SRC_DIR)/CgiHandler2.cpp \
				$(SRC_DIR)/CgiHandler3.cpp \
				$(SRC_DIR)/HttpParser.cpp \
				$(SRC_DIR)/Listener.cpp \
				$(SRC_DIR)/Logger.cpp \
				$(SRC_DIR)/ResponseWriter.cpp \
				$(SRC_DIR)/Router.cpp \
				$(SRC_DIR)/Router_newRouter.cpp \
				$(SRC_DIR)/Server.cpp \
				$(SRC_DIR)/ServerBuilder.cpp \
				$(SRC_DIR)/TokenStream.cpp \
				$(SRC_DIR)/UploadHandler.cpp \
				$(SRC_DIR)/handlerUtils.cpp \
				$(SRC_DIR)/httpParsing.cpp \
				$(SRC_DIR)/BodyParser.cpp \
				$(SRC_DIR)/logging.cpp \
				$(SRC_DIR)/utils.cpp

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

NAME := webserv

all: $(NAME)

$(NAME): $(OBJ_FILES) $(RUN_DIR)/main.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG) $^ -o $(NAME) $(INCLUDES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG) $(INCLUDES) -c $< -o $@

debug:
	$(MAKE) $(NAME) DEBUG=-DDEBUG

unittest: prepare_unittests
	cmake -S . -B build && \
	cmake --build build  && \
	./build/run_unittests

prepare_unittests:
	@mkdir tests/unittests/test_files/UploadHandler/uploads/dirCannotOpen || \
	chmod 000 tests/unittests/test_files/UploadHandler/uploads/dirCannotOpen

	@touch tests/unittests/test_files/UploadHandler/uploads/existDir/fileCannotOpen.txt && \
	chmod 000 tests/unittests/test_files/UploadHandler/uploads/existDir/fileCannotOpen.txt

clean:
	rm -rf .cache
	rm -rf build
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean
	make $(NAME)

compile_commands:
	cmake -S . -B build -DBUILD_TEST=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && \
	mv build/compile_commands.json ./compile_commands.json

compile_commands_98:
	cmake -S . -B build -DBUILD_MAIN=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && \
	mv build/compile_commands.json ./compile_commands.json

# end
