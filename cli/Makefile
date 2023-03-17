CXX = clang++

CXXFLAGS = -O3 -std=c++17

ifeq ($(UNAME_S), Darwin) #APPLE
	EXEC = ../raytracer_cli
endif

ifeq ($(OS), Windows_NT)
	EXEC = ../raytracer_cli.exe
endif

all:
	$(CXX) $(CXXFLAGS) -I ../core/src src/main.cpp -o $(EXEC) ../core/raytracer.lib

clean:
	rm -f $(EXEC)