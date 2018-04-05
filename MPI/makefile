#!/bin/bash

BUILD_DIR := ./bin
INCLUDES := -I ./inc
DBG_TARGET := dbg.o
REL_TARGET := rel.o
SRC_DIR := ./src
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)

CXXFLAGS := -fopenmp -ggdb -Wall -std=c++0x
CXXFLAGS_REL := -fopenmp -O3 -Wall -std=c++0x
CREATE_FOLDER:= @if [ ! -d "$(BUILD_DIR)" ]; then echo "The $(BUILD_DIR) folder does not exist. Creating $(BUILD_DIR)"; mkdir $(BUILD_DIR) ; fi

.ONESHELL:
debug:
	$(CREATE_FOLDER)
	@echo "Building Debug ..."
	@mpiicpc $(CXXFLAGS) $(INCLUDES) $(SOURCES) -o $(BUILD_DIR)/$(DBG_TARGET)
	@echo "Done!"

.ONESHELL:
release:
	$(CREATE_FOLDER)
	@echo "Building Release ..."
	@mpiicpc $(CXXFLAGS_REL) $(INCLUDES) $(SOURCES) -o $(BUILD_DIR)/$(REL_TARGET)
	@echo "Done!"

clean:
	@rm -r $(BUILD_DIR)
