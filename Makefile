

CXXFLAGS += -Wall -pedantic -O3 -fopenmp --std=c++17

main : main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o main



.PHONY : clean


clean : 
	rm -rf ./main
	rm -rf ./*.dat
	rm -rf ./*.svg
