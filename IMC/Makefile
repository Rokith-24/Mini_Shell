
# ============================================================
# Infrastructure Management Console (IMC)
# Linux Makefile
#
# Targets:
# all, run, test, gtest, gcov, coverage, cppcheck, misra,
# valgrind, helgrind, check-tools, clean, help
# ============================================================

# ------------------------------------------------------------
# COMPILERS AND DEVELOPMENT TOOLS
# ------------------------------------------------------------

CC       := gcc
CXX      := g++
GCOV     ?= gcov
GCOVR    ?= gcovr
CPPCHECK ?= cppcheck
VALGRIND ?= valgrind

SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

# ------------------------------------------------------------
# COMPILER FLAGS
# ------------------------------------------------------------

CPPFLAGS := -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -Iinclude

CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Wshadow \
          -Wstrict-prototypes -g -O0

CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic \
            -Wshadow -g -O0

LDFLAGS ?=
LDLIBS  ?= -pthread

GTEST_LIBS ?= -lgtest -lgtest_main -pthread

# MISRA addon location (can be overridden)
MISRA_ADDON ?= /usr/share/cppcheck/addons/misra.py

# ------------------------------------------------------------
# PROJECT DIRECTORIES
# ------------------------------------------------------------

SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build
DATA_DIR  := data
CONFIG_DIR := config
DOCS_DIR  := docs
INCLUDE_DIR := include

# ------------------------------------------------------------
# APPLICATION SOURCE FILES
# Automatically discovers all src/*.c files
# ------------------------------------------------------------

APP_SOURCES := $(wildcard $(SRC_DIR)/*.c)

APP_OBJECTS := $(patsubst $(SRC_DIR)/%.c, \
                $(BUILD_DIR)/%.o,$(APP_SOURCES))

# Exclude main.o from Google Test executable
CORE_OBJECTS := $(filter-out $(BUILD_DIR)/main.o,$(APP_OBJECTS))

APP := $(BUILD_DIR)/imc

# ------------------------------------------------------------
# GOOGLE TEST SOURCE FILES
# Automatically discovers all test/test_*.cpp files
# ------------------------------------------------------------

TEST_SOURCES := $(wildcard $(TEST_DIR)/test_*.cpp)

TEST_OBJECTS := $(patsubst $(TEST_DIR)/%.cpp, \
                 $(BUILD_DIR)/%.o,$(TEST_SOURCES))

GTEST_BIN := $(BUILD_DIR)/imc_tests

# ------------------------------------------------------------
# COVERAGE CONFIGURATION
# ------------------------------------------------------------

COVERAGE_DIR := $(BUILD_DIR)/coverage

COVERAGE_CFLAGS := $(CFLAGS) --coverage \
                   -fprofile-arcs -ftest-coverage

COVERAGE_CXXFLAGS := $(CXXFLAGS) --coverage \
                     -fprofile-arcs -ftest-coverage

# ------------------------------------------------------------
# PHONY TARGETS
# ------------------------------------------------------------

.PHONY: all run test gtest gcov coverage cppcheck misra \
        valgrind helgrind check-tools clean help

# ============================================================
# 1. BUILD APPLICATION
# ============================================================

all: $(APP)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(APP): $(APP_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# ============================================================
# 2. RUN APPLICATION
# ============================================================

run: $(APP)
	./$(APP)

# ============================================================
# 3. GOOGLE TEST COMPILATION
# ============================================================

$(BUILD_DIR)/test_%.o: $(TEST_DIR)/test_%.cpp \
                       $(TEST_DIR)/test_support.h \
                       $(INCLUDE_DIR)/imc.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(TEST_DIR) \
	       -MMD -MP -c $< -o $@

$(GTEST_BIN): $(CORE_OBJECTS) $(TEST_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(GTEST_LIBS) $(LDLIBS)

# ============================================================
# 4. RUN GOOGLE TEST SUITES
# ============================================================

# Build the application too because the integration test
# launches ./build/imc.

gtest: $(APP) $(GTEST_BIN)
	./$(GTEST_BIN)

# Both targets run the existing Google Test suites.
test: gtest

# ============================================================
# 5. GCOV / GCOVR CODE COVERAGE
# ============================================================

coverage:
	$(MAKE) clean

	$(MAKE) \
	    CFLAGS="$(COVERAGE_CFLAGS)" \
	    CXXFLAGS="$(COVERAGE_CXXFLAGS)" \
	    LDFLAGS="$(LDFLAGS) --coverage" \
	    test

	@command -v $(GCOVR) >/dev/null 2>&1 || { \
	    echo "ERROR: gcovr is required for coverage reports."; \
	    echo "Install gcovr and run make coverage again."; \
	    exit 127; \
	}

	mkdir -p $(COVERAGE_DIR)

	$(GCOVR) --root . \
	    --filter 'src/.*' \
	    --exclude 'src/main.c' \
	    --html-details $(COVERAGE_DIR)/index.html \
	    --xml $(COVERAGE_DIR)/coverage.xml \
	    --print-summary \
	    --fail-under-line 90 \
	    --fail-under-function 90

gcov: coverage

# ============================================================
# 6. CPPCHECK STATIC ANALYSIS
# ============================================================

cppcheck:
	@command -v $(CPPCHECK) >/dev/null 2>&1 || { \
	    echo "ERROR: Cppcheck is not installed."; \
	    exit 127; \
	}

	@echo "========== CPPCHECK: C SOURCE =========="

	$(CPPCHECK) \
	    --enable=warning,style,performance,portability \
	    --inconclusive \
	    --std=c11 \
	    --language=c \
	    --suppress=missingIncludeSystem \
	    -I $(INCLUDE_DIR) \
	    $(SRC_DIR)

	@echo "========== CPPCHECK: GOOGLE TEST SOURCE =========="

	$(CPPCHECK) \
	    --enable=warning,style,performance,portability \
	    --inconclusive \
	    --std=c++17 \
	    --language=c++ \
	    --suppress=missingIncludeSystem \
	    -I $(INCLUDE_DIR) -I $(TEST_DIR) \
	    $(TEST_DIR)

# ============================================================
# 7. MISRA C STATIC ANALYSIS
# ============================================================

misra:
	@command -v $(CPPCHECK) >/dev/null 2>&1 || { \
	    echo "ERROR: Cppcheck is not installed."; \
	    exit 127; \
	}

	@test -f "$(MISRA_ADDON)" || { \
	    echo "ERROR: MISRA addon not found: $(MISRA_ADDON)"; \
	    echo "Set MISRA_ADDON to the correct misra.py path."; \
	    exit 127; \
	}

	@echo "========== MISRA C ANALYSIS =========="

	$(CPPCHECK) \
	    --addon=$(MISRA_ADDON) \
	    --enable=warning,style,performance,portability \
	    --language=c \
	    --std=c11 \
	    --suppress=missingIncludeSystem \
	    -I $(INCLUDE_DIR) \
	    $(SRC_DIR) 2>&1 | tee misra_report.txt

# ============================================================
# 8. VALGRIND MEMORY ANALYSIS
# ============================================================

valgrind: $(APP)
	@command -v $(VALGRIND) >/dev/null 2>&1 || { \
	    echo "ERROR: Valgrind is not installed."; \
	    exit 127; \
	}

	$(VALGRIND) \
	    --leak-check=full \
	    --show-leak-kinds=all \
	    --track-origins=yes \
	    --verbose \
	    ./$(APP)

# ============================================================
# 9. HELGRIND THREAD ANALYSIS
# ============================================================

helgrind: $(APP)
	@command -v $(VALGRIND) >/dev/null 2>&1 || { \
	    echo "ERROR: Valgrind is not installed."; \
	    exit 127; \
	}

	$(VALGRIND) \
	    --tool=helgrind \
	    --history-level=full \
	    ./$(APP)

# ============================================================
# 10. CHECK DEVELOPMENT TOOLS
# ============================================================

check-tools:
	@echo "========== DEVELOPMENT TOOL CHECK =========="

	@for tool in $(CC) $(CXX) $(GCOV) \
	             $(CPPCHECK) $(VALGRIND) $(GCOVR); do \
	    if command -v $$tool >/dev/null 2>&1; then \
	        printf "FOUND:   %s\n" "$$tool"; \
	    else \
	        printf "MISSING: %s\n" "$$tool"; \
	    fi; \
	done

	@echo "========== GOOGLE TEST CHECK =========="

	@if printf '#include <gtest/gtest.h>\nint main(){return 0;}\n' | \
	    $(CXX) -x c++ -std=c++17 - -lgtest -lgtest_main \
	    -pthread -o /tmp/imc_gtest_check >/dev/null 2>&1; then \
	    echo "FOUND: Google Test headers and libraries"; \
	    rm -f /tmp/imc_gtest_check; \
	else \
	    echo "MISSING: Google Test headers or libraries"; \
	fi

	@echo "========== MISRA ADDON CHECK =========="

	@if test -f "$(MISRA_ADDON)"; then \
	    echo "FOUND: $(MISRA_ADDON)"; \
	else \
	    echo "MISSING: $(MISRA_ADDON)"; \
	fi

# ============================================================
# 11. CLEAN GENERATED FILES
# ============================================================

clean:
	rm -rf $(BUILD_DIR)/*
	rm -f *.gcda *.gcno *.gcov
	rm -f misra_report.txt

# ============================================================
# 12. HELP
# ============================================================

help:
	@echo "=================================================="
	@echo " Infrastructure Management Console - Make Targets"
	@echo "=================================================="
	@echo " make              Build the IMC application"
	@echo " make all          Build the IMC application"
	@echo " make run          Build and run the application"
	@echo " make test         Build and execute Google Tests"
	@echo " make gtest        Build and execute Google Tests"
	@echo " make gcov         Generate code coverage reports"
	@echo " make coverage     Generate GCOVR coverage reports"
	@echo " make cppcheck     Run C and C++ static analysis"
	@echo " make misra        Run MISRA C static analysis"
	@echo " make valgrind     Run memory error analysis"
	@echo " make helgrind     Run thread error analysis"
	@echo " make check-tools  Check development dependencies"
	@echo " make clean        Remove generated build files"
	@echo " make help         Display available targets"
	@echo "=================================================="

# ------------------------------------------------------------
# AUTOMATIC HEADER DEPENDENCIES
# ------------------------------------------------------------

-include $(APP_OBJECTS:.o=.d)
-include $(TEST_OBJECTS:.o=.d)
