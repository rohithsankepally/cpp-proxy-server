all: setup_proxy

setup_proxy: helper.o proxy.o
	g++ helper.o proxy.o -o proxy -pthread

helper.o: helper.cpp
	g++ -c helper.cpp

proxy.o: proxy.cpp
	g++ -c proxy.cpp

clean:
	rm *o proxy