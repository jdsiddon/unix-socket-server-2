Author: Justin Siddon
Date: 5/26/16
CS372

* Description *
This is a simple file transfer system.
A C++ server starts and hosts a control connection and waits for a client request.
The Python client requests data from the server over the control connection.
Along with the request the client sends a new port to send the requested data back to(a file or list). The
returned data gets transmitted over the port as specified by the client(transmission port).

* Usage *
1. Compile the server program
  g++ server.cpp -o ftserver

2. Start the server
  ./ftserver [CONTROL PORT]

3. Start the client and connect to the servers control port
  python client.py [SERVER HOSTNAME] [CONTROL PORT] [COMMAND TYPE] [TRANSMIT PORT]

4. View results.
  GET(-g): Will save the requested file in the current directory of the client
  LIST(-l): Will print the contents of the remote server directory to the client's console

5. Clean server (remove)
  rm ftserver

* Valid Arguments *
Server
./ftserver [PORT]

Client
GET: python client.py [SERVER HOSTNAME] [CONTROL PORT] -g [FILENAME] [TRANSMIT PORT]

LIST: python client.py [SERVER HOSTNAME] [CONTROL PORT] -l [TRANSMIT PORT]

* Special Notes *
- Make sure to use screen in the same terminal session to communicate between the client and servers to prevent any
security problems.
- The client will not allow you to get a file that already exists in its current directory (no overwriting). To
test the file GET move the client code into a different sub directory where the file you are requesting doesn't exist.
e.g.,
\ Root
- server.cpp
- ftserver
- commands.c
- README.txt
- socket.c
  \ DIRECTORY
    - client.py

When requesting a file only use the filename, the server will get the file named in its current directory and return
it to the client.

* Sources *
- https://pymotw.com/3/socketserver/index.html
- https://pymotw.com/2/socket/tcp.html
- http://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data
- http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
- http://stackoverflow.com/questions/666601/what-is-the-correct-way-of-reading-from-a-tcp-socket-in-c-c
- http://www.linuxhowtos.org/C_C++/socket.htm

* Testing *
- Ubuntu 14.04.3 LTS and OSU FLIP3 server
