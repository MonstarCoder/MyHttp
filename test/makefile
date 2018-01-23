args1 = parse.o parse_test.o
# args2 = threads.o thread_test.o

parse: $(args1) 
	g++ -g -o parse $(args1) 
parse.o: ../parse.h ../parse.cc
	g++ -g -c ../parse.cc
parse_test.o: parse_test.cc
	g++ -g -c parse_test.cc

# thread: $(args2)
# 	g++ -g -o thread $(args2) -pthred
# threads.o: ../threads.h threads.cc
# 	g++ -g -c ../threads.cc
# thread_test.o: thread_test.cc
# 	g++ -g -c thread_test.cc


.PHONY: clean
clean:
	-rm $(args1) $(args2)