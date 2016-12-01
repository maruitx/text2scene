import socket
import sys
import os

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ('localhost', 3000)
print >>sys.stderr, 'connecting to %s port %s' % server_address
sock.connect(server_address)

try:
    basePath = os.path.dirname(os.path.realpath(__file__))
    
    # Send data
    with open(os.path.join(basePath, 'input.txt'), 'r') as inputFile:
        message = inputFile.read()
    print >>sys.stderr, 'sending: %s' % message
    sock.sendall(message)

    # receive the first part of the data (blocking)
    parsedDoc = sock.recv(16)

    # the rest of the sentence is now in the buffer, so read everything.
    sock.settimeout(1.0)
    while True:
        newData = sock.recv(16)
        if len(newData) == 0:
            break
        parsedDoc = parsedDoc + newData
    
    print >>sys.stderr, 'received: %s' % parsedDoc

    with open(os.path.join(basePath, 'output.txt'), "w") as outputFile:
        outputFile.write(parsedDoc)
        
finally:
    print >>sys.stderr, 'closing socket'
    sock.close()
