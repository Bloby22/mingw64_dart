CMAKE ?= cmake
BUILD_DIR ?= build
CMAKE_FLAGS ?=

ifeq ($(OS),Windows_NT)
  CMAKE_GENERATOR ?= "MinGW Makefiles"
  RM = if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
else
  CMAKE_GENERATOR ?= "Unix Makefiles"
  RM = rm -rf $(BUILD_DIR)
endif

.PHONY: all build configure clean rebuild test help

all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -G $(CMAKE_GENERATOR) $(CMAKE_FLAGS)

build: configure
	$(CMAKE) --build $(BUILD_DIR) --config Release

rebuild: clean build

test: configure
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	$(RM)

help:
	@echo "Targets:"
	@echo "  make build     Configure and build the project"
	@echo "  make test      Run tests with CTest"
	@echo "  make clean     Remove the build directory"
	@echo "  make rebuild   Clean and rebuild"
	@echo "  make help      Show this help"
