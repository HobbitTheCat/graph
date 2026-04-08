CXX := g++
NVCC := nvcc

CXXFLAGS := -Wall -Wextra -std=c++20
INCLUDES := -Iinclude -I/usr/local/cuda/include -I.

NVCCFLAGS := -std=c++20 -O3 --compiler-options '-fPIC'

LIBS := -lGLEW -lGL -lglfw -lcudart -L/usr/local/cuda/lib64

SRC_DIR := src
BUILD_DIR := build

CPP_SRC := $(shell find $(SRC_DIR) -type f -name '*.cpp' ! -name 'app_*')
CU_SRC := $(shell find $(SRC_DIR) -type f -name '*.cu')
APPS_SRC := $(shell find $(SRC_DIR) -type f -name 'app_*.cpp')

SHADERS := $(shell find . -type f -name '*.glsl')
SHADERS_IN_BUILD := $(addprefix $(BUILD_DIR)/, $(notdir $(SHADERS)))

OBJ := 	$(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SRC)) \
        $(patsubst $(SRC_DIR)/%.cu,$(BUILD_DIR)/%.cu.o,$(CU_SRC))

APPS_TARGET := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%,$(APPS_SRC))

all: $(APPS_TARGET) $(SHADERS_IN_BUILD)

$(BUILD_DIR)/%.glsl:
	@mkdir -p $(dir $@)
	cp $(shell find . -type f -name '$*.glsl' | head -n 1) $@
	
$(BUILD_DIR)/app_%: $(BUILD_DIR)/app_%.o $(OBJ) $(SHADERS_IN_BUILD)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(filter-out $(SHADERS_IN_BUILD), $^) -o $@ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cu.o: $(SRC_DIR)/%.cu
	@mkdir -p $(dir $@)
	$(NVCC) $(NVCCFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean