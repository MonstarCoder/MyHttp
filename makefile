args = main.o utility.o parse.o

a.out: $(args) 
	g++ -std=c++11 -g -o a.out $(args)
main.o: http.h http.cc
	g++ -std=c++11 -g -c http.cc
utility.o: utility.h utility.cc
	g++ -std=c++11 -g -c utility.cc
parse.o: parse.h parse.cc
	g++ -std=c++11 -g -c parse.cc

.PHONY: clean
clean:
	-rm $(args)