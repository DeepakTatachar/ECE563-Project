#!/bin/bash

BUILD_DIR := ./bin
INCLUDES := -I ./inc
DBG_TARGET := dbg.o
REL_TARGET := rel.o
SRC_DIR := ./src
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)

CXXFLAGS := -fopenmp -ggdb -Wall
CXXFLAGS_REL := -fopenmp -O3 -Wall
CREATE_FOLDER:= @if [ ! -d "$(BUILD_DIR)" ]; then echo "The $(BUILD_DIR) folder does not exist. Creating $(BUILD_DIR)"; mkdir $(BUILD_DIR) ; fi

.ONESHELL:
debug:
	$(CREATE_FOLDER)
	@echo "Building Debug ..."
	@g++ $(CXXFLAGS) $(INCLUDES) $(SOURCES) -o $(BUILD_DIR)/$(DBG_TARGET)

.ONESHELL:
release:
	$(CREATE_FOLDER)
	@echo "Building Release ..."
	@g++ $(CXXFLAGS_REL) $(INCLUDES) $(SOURCES) -o $(BUILD_DIR)/$(REL_TARGET)



clean:
	@rm -r $(BUILD_DIR)
