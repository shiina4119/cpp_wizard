all: client

client: client.o packet.o
	g++ -o client client.o packet.o

packet.o: packet.cpp
	g++ -c packet.cpp

client.o: client.cpp
	g++ -c client.cpp

