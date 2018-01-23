args = main.o utility.o parse.o threads.o

http: $(args) 
	g++ -std=c++11 -g -o http $(args) -lpthread
main.o: http.h http.cc
	g++ -std=c++11 -g -c -o main.o http.cc
utility.o: utility.h utility.cc
	g++ -std=c++11 -g -c utility.cc
parse.o: parse.h parse.cc
	g++ -std=c++11 -g -c parse.cc
threads.o: threads.h threads.cc
	g++ -std=c++11 -g -c threads.cc

.PHONY: clean
clean:
	-rm $(args)