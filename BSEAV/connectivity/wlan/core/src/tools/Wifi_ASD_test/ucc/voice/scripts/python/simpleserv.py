from socket import *
host = "localhost"
port = 2727
buf = 1024
addr = (host, port)

UDPSock = socket(AF_INET, SOCK_DGRAM)
msg = "whatever message goes here"
UDPSock.sendto(msg,addr)
