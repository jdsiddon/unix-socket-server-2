 #############################################
 # File: client.python
 # Author: Justin Siddon
 # Description: This file provides the client interface to a file server.
 #  Once it send a message to a server (in the correct format) it starts
 #  its own server based on the passed port for the server to connect to.
 # Sources
 #  https://pymotw.com/3/socketserver/index.html
 #  https://pymotw.com/2/socket/tcp.html
 #############################################

import socket
import sys
from sys import argv

#############################################
# Function: receiveFile
# Description: Gets all the data from the passed socket.
#   Based off: http://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data
# Parameters: sock - socket connection
# Returns: string with all data from socket.
#############################################
def receiveFile(sock):
    data = ''                               # Blank string to hold message in.
    count = 0
    n = 24                                  # Set n to read in enough to get the actual message size.

    while len(data) < n:                    # While we havent gotten all the data.
        packet = sock.recv(n - len(data))

        if not packet:
            return None

        if(count == 0):
            n = int(packet.split(':')[0])     # Get actual total message size.

            if(packet.split(':')[1].split(']')[0] == '[ERROR'):         # Check for error message from server.
                data += "e:"                                            # Designate the message as an error.
                data += packet.split(':')[1].split(']')[1]              # Errors have '[ERROR]' in front of the message.
            else:
                data += packet.split(':')[1]                            # Non-error message, Get the first part of the message.
        else:
            data += packet

        count += 1
    return data

#############################################
# Function: parseParams
# Description: This function parses apart the parameters the user passed in at the
#  command line when starting the client.
# Parameters: params - command line parameters
# Returns: python dictionary with connection attributes defined.
#############################################
def parseParams(params):
    serv = {}

    if(len(params) < 5):
        print "usage: [hostname] [control port] [control command] [filename] [reply port]"   # Provide hostname and port
        return
    else:
        for idx, p in enumerate(params):
            if(idx == 1):
                serv['hostname'] = p
            elif(idx == 2):
                serv['contport'] = int(p)
            elif(idx == 3):
                if(p == "-l"):
                    serv['type'] = 0
                    if(len(params) != 5):
                        print "ERROR: Only 5 params can be passed to a list (-l) command [hostname] [control port] -l [reply port]"
                        exit()
                elif(p == "-g"):
                    serv['type'] = 1
                    if(len(params) != 6):
                        print "ERROR: Only 6 params can be passed to a list (-g) command [hostname] [control port] -l [filename] [reply port]"
                        exit()
                else:
                    print "ERROR: invalid command \"%s\", valid commands are -l or -g" % p
                    exit()
            elif(idx == 4):
                if(serv['type'] == 0):
                    try:
                        serv['transport'] = int(p)
                    except:
                        print "ERROR: invalid port \"%s\", must be valid number" % p
                        exit()
                else:
                    serv['filename'] = p
            elif(idx == 5):
                try:
                    serv['transport'] = int(p)
                except:
                    print "ERROR: invalid port \"%s\", must be valid number" % p
                    exit()

        return serv


def makeRequest(sock, server):
    # Determine request type
    if(server['type'] == 0):                        # LIST request
        message = server['hostname'] + ' ' + str(server['type']) + ' ' + str(server['transport'])
    elif(server['type'] == 1):                      # GET request
        message = server['hostname'] + ' ' + str(server['type']) + ' ' + str(server['transport']) + ' ' + server['filename']

    # Send request
    sock.sendall(message)
    return




#############################################
# Function: initiateContact
# Description: This function starts up a data server to receive the requested command result on.
# Parameters: none - consumes command line args
# Returns: nothing
#############################################
def initiateContact():
    # Main code.
    server = {}                         # Initialize blank dictionary
    server = parseParams(argv)          # Get params to talk to server.

    # Set up Streaming socket.
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (server['hostname'], server['contport'])

    # Connect to server on control port.
    sock.connect(server_address)

    makeRequest(sock, server)       # Send request to server.

    sock.close()                    # Close control socket.

    # Create server to receive data back from server.
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_host = (server['hostname'], server['transport'])
    sock.bind(client_host)

    sock.listen(1)          # Listen for incoming requests.

    while True:
        connection, client_address = sock.accept()              # Accept connections.
        message = receiveFile(connection)                           # Read data from connection.
        if message:
            break

    # Determine message type to print or save as file.
    if(server['type'] == 0):
        print "Receiving directory structure from %s:%d" % (server['hostname'], server['transport'])
        print message
        print "Transfer complete"

    elif(server['type'] == 1):
        if(message.split(':')[0] == "e"):                        # Make sure the file was present.
            print "%s:%d says %s" % (server['hostname'], server['transport'], message.split(':')[1])
        else:
            exists = 1
            print "Receiving \"%s\" from %s" % (server['filename'], server['hostname'])

            try:                                        # Check duplicate file.
                file = open(server['filename'], "r")
            except:                                     # File didn't exist so throw error.
                exists = 0

            if(exists):
                print "File with name \"%s\" already exists! Select new file name or delete existing file!" % server['filename']
            else:
                file = open(server['filename'], "w")
                file.write(message)
                file.close()
                print "Transfer complete"


#### Main program code
initiateContact()
