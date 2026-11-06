CC := gcc
CXX := g++
CPPFLAGS := -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -Iinclude
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -g -O0
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -g -O0
LDFLAGS :=
LDLIBS :=
GTEST_LIBS ?= -lgtest -lgtest_main -pthread

SRC := $(wildcard src/*.c)
APP_OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
TEST_SRC := $(wildcard test/test_*.cpp)
TEST_OBJ := $(patsubst test/%.cpp,build/test_%.o,$(TEST_SRC))
TEST_APP_OBJ := $(filter-out build/main.o,$(APP_OBJ))
TARGET := build/imc
TEST_TARGET := build/imc_tests

.PHONY: all run test coverage cppcheck valgrind helgrind clean help check-tools

all: $(TARGET)

$(TARGET): $(APP_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: src/%.c include/imc.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/test_%.o: test/test_%.cpp test/test_support.h include/imc.h
	@mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Itest -c $< -o $@

$(TEST_TARGET): $(TEST_APP_OBJ) $(TEST_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^ $(GTEST_LIBS) $(LDLIBS)

run: all
	./$(TARGET)

check-tools:
	@command -v $(CC) >/dev/null || { echo "GCC is required"; exit 1; }
	@command -v $(CXX) >/dev/null || { echo "G++ is required for Google Test"; exit 1; }
	@$(CXX) -x c++ -E - </dev/null >/dev/null

test: all $(TEST_TARGET)
	./$(TEST_TARGET)

cppcheck:
	@command -v cppcheck >/dev/null || { echo "Cppcheck is not installed. Install cppcheck and rerun make cppcheck."; exit 127; }
	cppcheck --enable=warning,style,performance,portability --inconclusive --std=c11 --suppress=missingIncludeSystem -I include src
	cppcheck --enable=warning,style,performance,portability --inconclusive --std=c++17 --language=c++ --suppress=missingIncludeSystem -I include -I test test

coverage:
	@command -v gcovr >/dev/null || { echo "gcovr is required for enforced line/function thresholds. Install gcovr and rerun make coverage."; exit 127; }
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) --coverage -fprofile-arcs -ftest-coverage" CXXFLAGS="$(CXXFLAGS) --coverage -fprofile-arcs -ftest-coverage" LDFLAGS="--coverage" all test
	@mkdir -p build/coverage
	gcovr --root . --filter 'src/.*' --exclude 'src/main.c' --html-details build/coverage/index.html --xml build/coverage/coverage.xml --print-summary --fail-under-line 90 --fail-under-function 90

valgrind: all
	@command -v valgrind >/dev/null || { echo "Valgrind is not installed."; exit 127; }
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

helgrind: all
	@command -v valgrind >/dev/null || { echo "Valgrind is not installed."; exit 127; }
	valgrind --tool=helgrind ./$(TARGET)

clean:
	rm -rf build/* *.gcda *.gcno *.gcov

help:
	@echo "Targets: all run test cppcheck coverage valgrind helgrind clean help"
	@echo "Google Test is required for 'make test'; gcovr is required for 'make coverage'."
