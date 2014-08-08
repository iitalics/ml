CXX=g++
CXXFLAGS=-std=c++11 -Wall -O3
LXXFLAGS=-std=c++11 -O3 -s



OUTPUT=ml.exe

SOURCES=$(wildcard src/*.cpp)
OBJECTS=$(SOURCES:src/%.cpp=obj/%.o)

all: $(OUTPUT)


obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTPUT): $(OBJECTS)
	g++ $(LXXFLAGS) -o $@ $(OBJECTS)

clean:
	del /F/Q $(OBJECTS:obj/%="obj\\%") $(OUTPUT)

rebuild: clean all
