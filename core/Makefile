CXX = clang++
ARCHIVER = llvm-ar

CXXFLAGS = -O3 -std=c++17 -MMD

BUILD_DIR = build
SRC_DIR = src

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
DEP_FILES = $(OBJ_FILES:.o=.d)

ifeq ($(UNAME_S), Darwin) #APPLE
	LIB = raytracer.a
endif

ifeq ($(OS), Windows_NT)
	LIB = raytracer.lib
endif

all: $(LIB)

.PHONY: clean
clean:
	rm -f $(LIB) $(OBJ_FILES) $(DEP_FILES)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(LIB): $(OBJ_FILES)
	$(ARCHIVER) rc $(LIB) $^

-include $(DEP_FILES)