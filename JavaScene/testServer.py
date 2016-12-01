import socket
import sys

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ('localhost', 3000)
print >>sys.stderr, 'connecting to %s port %s' % server_address
sock.connect(server_address)

try:
    
    # Send data
    message = 'I am going to go to the store tomorrow.  I will buy grapes and apples.\n'
    print >>sys.stderr, 'sending: %s' % message
    sock.sendall(message)

    # receive the first part of the data (blocking)
    parsedDoc = sock.recv(16)

    # the rest of the sentence is now in the buffer, so read everything.
    sock.settimeout(0.0)
    while True:
        newData = sock.recv(16)
        if len(newData) == 0:
            break
        parsedDoc = parsedDoc + newData
    
    print >>sys.stderr, 'received: %s' % parsedDoc

finally:
    print >>sys.stderr, 'closing socket'
    sock.close()
