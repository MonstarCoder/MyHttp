args = parse.o parse_test.o

a.out: $(args) 
	g++ -g -o a.out $(args) 
parse.o: ../parse.h ../parse.cc
	g++ -g -c ../parse.cc
parse_test.o: parse_test.cc
	g++ -g -c parse_test.cc

.PHONY: clean
clean:
	-rm args