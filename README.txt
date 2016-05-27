# Description
This is a simple file transfer system.
A C++ server starts and hosts a control connection.
  ./ftserver [SERVER_PORT>]

And waits for a client request.

The client is written in Python and requests data from the server over the control connection.
Along with the request the client sends a new port to send the file or data back to. The actual
data gets transmitted over the new port as specified by the client.


# Usage
1. Compile the server program
  make server

2. Start the server
  ./ftserver [CONTROL PORT]

3. Start the client and connect to the servers control port
  python client.py [SERVER HOSTNAME] [CONTROL PORT] [COMMAND TYPE] [TRANSMIT PORT]

4. View results.
  GET(-g): Will save the requested file in the current directory of the client
  LIST(-l): Will print the contents of the remote server directory to the client's console


## Valid Arguments

### Server
./ftserver [PORT]

### Client
#### GET
python client.py [SERVER HOSTNAME] [CONTROL PORT] -g [FILENAME] [TRANSMIT PORT]

#### LIST
python client.py [SERVER HOSTNAME] [CONTROL PORT] -l [TRANSMIT PORT]
