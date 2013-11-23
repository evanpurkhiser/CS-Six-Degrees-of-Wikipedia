bin/sdwiki: src/sdwiki.cpp
	mkdir -p bin
	g++ -Wall --std=c++11 -fopenmp src/sdwiki.cpp -o $@

clean:
	rm -f bin/sdwiki
