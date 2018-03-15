CC = g++
CXX = g++

CXXFLAGS = -Wall -Werror -ansi -pedantic -std=c++11 -I .
SRC = main.cc water_jug_solver.cc

.PHONY: main

main: CXXFLAGS += -O2

main: $(SRC) 
	$(CC) $(CXXFLAGS) -o $@ $^

clean:
	$(RM) main
