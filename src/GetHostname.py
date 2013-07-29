import socket

def GetHostname(hostname):
    addr = socket.gethostbyname(hostname)
    return addr
