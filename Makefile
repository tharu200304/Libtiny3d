# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -Iinclude

# Directories
SRC_DIR = src
BUILD_DIR = build
DEMO_DIR = demo
TEST_DIR = tests

# Source files
SRC_FILES = $(SRC_DIR)/canvas.c $(SRC_DIR)/math3d.c $(SRC_DIR)/renderer.c \
            $(SRC_DIR)/lighting.c $(SRC_DIR)/animation.c

# Demo source files
MAIN_SRC = $(DEMO_DIR)/main.c
CLOCKFACE_SRC = $(DEMO_DIR)/clockface.c
SOCCERBALL_SRC = $(DEMO_DIR)/Soccerball.c

# Test source files
TEST_MATH_SRC = $(TEST_DIR)/test_math.c $(SRC_DIR)/math3d.c
TEST_PIPELINE_SRC = $(TEST_DIR)/test_pipeline.c $(SRC_DIR)/canvas.c $(SRC_DIR)/math3d.c

# Derived object files (stored temporarily)
LIB_OBJS := $(SRC_FILES:%.c=%.o)

# Targets
.PHONY: all clean dirs png gif \
        run_all_task run_main run_clockface run_soccerball \
        run_test_math run_test_pipeline run_tests \
        libtiny3d.a

# Default: just build all
all: dirs libtiny3d.a main test_math test_pipeline clockface soccerball

# Create necessary directories
dirs:
	mkdir -p $(BUILD_DIR) frames

# Build static library
libtiny3d.a:
	$(CC) $(CFLAGS) -c $(SRC_FILES)
	ar rcs $(BUILD_DIR)/libtiny3d.a *.o
	rm -f *.o

# Build and link demo using the static library
main: $(BUILD_DIR)/libtiny3d.a
	$(CC) $(CFLAGS) $(MAIN_SRC) -L$(BUILD_DIR) -ltiny3d -lm -o main

clockface:
	$(CC) $(CFLAGS) $(CLOCKFACE_SRC) $(SRC_DIR)/canvas.c -lm -o clockface

soccerball: $(BUILD_DIR)/libtiny3d.a
	$(CC) $(CFLAGS) $(SOCCERBALL_SRC) -L$(BUILD_DIR) -ltiny3d -lm -o soccerball

# Build test_math
test_math:
	$(CC) $(CFLAGS) $(TEST_MATH_SRC) -lm -o test_math

# Build test_pipeline
test_pipeline:
	$(CC) $(CFLAGS) $(TEST_PIPELINE_SRC) -lm -o test_pipeline

# Run all primary executables and tests
run_all_task: main clockface soccerball test_math test_pipeline
	./clockface
	./soccerball
	./main
	./test_math
	./test_pipeline

# Individual run targets
run_main:
	./main

run_clockface:
	./clockface

run_soccerball:
	./soccerball

run_test_math:
	./test_math

run_test_pipeline:
	./test_pipeline

# Run both tests
run_tests: run_test_math run_test_pipeline

# Clean everything
clean:
	rm -rf build main clockface soccerball test_math test_pipeline *.pgm *.png *.o *.a frames/*.pgm frames/*.png soccerball.gif
