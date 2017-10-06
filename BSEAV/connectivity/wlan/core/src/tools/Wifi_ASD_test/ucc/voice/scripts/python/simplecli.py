import socket
mySocket = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
mySocket.bind ( ('', 2727))
while True:
    data, client = mySocket.recvfrom(100)
    print 'We have received a datagram from', client
    print data
    mySocket.sendto( 'Green-eyed datagram.', client)
