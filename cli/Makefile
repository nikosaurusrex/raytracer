CXX = clang++

CXXFLAGS = -O3 -std=c++17

ifeq ($(OS), Windows_NT)
	LIB = raytracer.lib
	EXEC = ../raytracer_cli.exe
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S), Darwin) #APPLE
		EXEC = ../raytracer_cli
		LIB = raytracer.a
	endif
endif

all:
	$(CXX) $(CXXFLAGS) -I ../core/src src/main.cpp -o $(EXEC) ../core/$(LIB)

clean:
	rm -f $(EXEC)
