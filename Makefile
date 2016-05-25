server: server.cpp
	g++ -Wall -Wextra server.cpp -o ftserver 

client: client.c
	gcc -Wall -Wextra -o ftclient client.c 

clean:
	rm -r ftserver ftclient

