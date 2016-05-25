GCC = g++
STD = -std=c++11
DEBUG = -g
OPTIMIZE = -O0

all: 1-1 N-N quality

1-1: simulate_1-1.cpp
	$(GCC) $(STD) $(DEBUG) $(OPTIMIZE)  -o  simulate_1-1        simulate_1-1.cpp

N-N: simulate_N-N.cpp
	$(GCC) $(STD) $(DEBUG) $(OPTIMIZE)  -o  simulate_N-N        simulate_N-N.cpp

quality: quality-statistics.cpp
	$(GCC) $(STD) $(DEBUG) $(OPTIMIZE)  -o  quality-statistics  quality-statistics.cpp

clean:
	rm -f  simulate_1-1        \
	       simulate_N-N        \
	       quality-statistics

