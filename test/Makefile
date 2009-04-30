
all: t_TransportPosition
	./t_TransportPosition --log_level=all --report_level=detailed

clean:
	rm -rf t_TransportPosition *.o

t_TransportPosition: t_TransportPosition.o
	g++ -ggdb -o $@ -DBOOST_TEST_DYN_LINK $< -L.. -lhydrogen -lboost_unit_test_framework

t_TransportPosition.o: t_TransportPosition.cpp
	g++ -ggdb -c -o $@ -DBOOST_TEST_DYN_LINK -I../libs/hydrogen/include -I. $<
