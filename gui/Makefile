CXX = clang++

ifeq ($(OS), Windows_NT)
	GLFW_INCLUDE = -Iextern/include
	GLFW_LIBS = -Lextern/libs -lglfw3_mt

	CXXFLAGS = -O3 -std=c++17 -MMD $(GLFW_INCLUDE)
	LDFLAGS = $(GLFW_LIBS) -lgdi32 -lopengl32 -lshell32  -limm32
	EXEC = ../raytracer_gui.exe
	LDFLAGS += ../core/raytracer.lib
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S), Darwin) #APPLE
		GLFW_INCLUDE = `pkg-config --cflags glfw3`
		GLFW_LIBS = `pkg-config --static --libs glfw3`

		CXXFLAGS = -O3 -std=c++17 -MMD $(GLFW_INCLUDE)
		LDFLAGS = $(GLFW_LIBS) -framework OpenGL -framework Cocoa -framework IOKit
		EXEC = ../raytracer_gui
		LDFLAGS += ../core/raytracer.a
	endif
endif

CXXFLAGS += -I../core/src

BUILD_DIR = build
SRC_DIR = src

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
DEP_FILES = $(OBJ_FILES:.o=.d)

all: $(EXEC)

.PHONY: clean
clean:
	rm -f $(EXEC) ../imgui.ini $(OBJ_FILES) $(DEP_FILES)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXEC): $(OBJ_FILES)
	$(CXX) $(LDFLAGS) -o $(EXEC) $^

-include $(DEP_FILES)
