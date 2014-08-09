CXX=g++
CXXFLAGS=-std=c++11 -g -Wall -O2
LXXFLAGS=-std=c++11



OUTPUT=ml.exe

SOURCES=$(wildcard src/*.cpp)
OBJECTS=$(SOURCES:src/%.cpp=obj/%.o)

all: $(OUTPUT)


obj/%.o: src/%.cpp src/Global.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTPUT): $(OBJECTS)
	g++ $(LXXFLAGS) -o $@ $(OBJECTS)

clean:
	del /F/Q $(OBJECTS:obj/%="obj\\%") $(OUTPUT)

rebuild: clean all
