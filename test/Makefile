
all: t_Hydrogen
	./t_Hydrogen --log_level=all --report_level=detailed

clean:
	rm -rf t_Hydrogen *.o

t_Hydrogen: t_000_master_test.o t_TransportPosition.o t_SeqEvent.o
	g++ -ggdb -o $@ -DBOOST_TEST_DYN_LINK $^ -L.. -lhydrogen -lboost_unit_test_framework -lQtCore

t_000_master_test.o: t_000_master_test.cpp
	g++ -ggdb -c -o $@ -DBOOST_TEST_DYN_LINK -I../libs/hydrogen/include -I. $<

t_TransportPosition.o: t_TransportPosition.cpp
	g++ -ggdb -c -o $@ -DBOOST_TEST_DYN_LINK -I../libs/hydrogen/include -I. $<

t_SeqEvent.o: t_SeqEvent.cpp
	g++ -ggdb -c -o $@ -DBOOST_TEST_DYN_LINK -I../libs/hydrogen/include -I. -I/usr/include/qt4/Qt -I/usr/include/qt4 $<
