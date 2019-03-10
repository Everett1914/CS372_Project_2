
#!/bin/python

# Developer:  Everett Williams
# Last Modified:  102330MAR19 (Day/Time/Month/Year)
# Program Name: ftclient.py
# Assignment:  CS372 Project 2
# Description:  Client implementation for a File Transfer Program using a TCP.
# This program represents the client side coding.
# client side architecture.
# References:
#   https://stackoverflow.com/questions/82831/how-do-i-check-whether-a-file-exists-without-exceptions
#   https://tools.ietf.org/html/rfc959
import socket
import sys
import os

# Name: openDataConnection(dprt)
# Desc: Creates socket and listens.  This is the dataport for the ftp.
# Args: port number for dataport
# Return: connection and socket
def openDataConnection(dPrt):
    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    serverSocket.bind(('', dPrt))
    serverSocket.listen(10)
    connection, addr = serverSocket.accept()
    return connection, serverSocket

# Name: receiveDirData(dPrt)
# Desc: Revieves the servers directory through the data connection.  Prints
#       file names to console.
# Helper for makerequest
# Args: port number for dataport
# Return: nothing
def receiveDirData(dPrt):
    print 'Receiving Directory Structure from ' + sys.argv[1] + ':  ' + sys.argv[4]
    dataConnection, serverSocket = openDataConnection(dPrt)
    message = dataConnection.recv(1024)
    print message
    print "___Transfer Complete___"
    return 0

# Name: receiveFileData(dPrt)
# Desc: Receives requested file through the data connection.  Saves the file
#       as the specified name or temp if there is a duplicate.
# Helper for makeRequest
# Args: port number for dataport
# Return: nothing
def receiveFileData(dPrt):
    print 'Receiving ' + sys.argv[4] + ' from ' + sys.argv[1] + ':  ' + sys.argv[5]
    dataConnection, serverSocket = openDataConnection(dPrt)
    buffer = dataConnection.recv(1024)
    filename = sys.argv[4]
    if os.path.isfile(sys.argv[4]):
        filename = "temp.txt"
        print sys.argv[4] + " is a duplicate file.  Contents for this file will be placed in a temporary file called temp.txt"
    f = open(filename, "w")
    f.write(buffer)
    while "<<stop>>" not in buffer:
        buffer = dataConnection.recv(1024)
        f.write(buffer)
    print "___Transfer Complete___"
    f.close()
    return 0

# Name: makeRequest(clientSocket)
# Desc: Controls the request logic with assitance from helper functions recieveFileData()
#       and receiveDirData()
# Args: port number for dataport
# Return: nothing
def makeRequest(clientSocket):
    if sys.argv[3] == '-l':
        message = socket.getfqdn() + ' ' +sys.argv[3] + ' ' + sys.argv[4]
        clientSocket.send(message.encode())
        dataPort = int(sys.argv[4])
        receiveDirData(dataPort)
    elif sys.argv[3] == '-g':
        message = socket.getfqdn() + ' ' +sys.argv[3] + ' ' + sys.argv[4] + ' ' + sys.argv[5]
        clientSocket.send(message.encode())
        response = clientSocket.recv(1024)
        if response == "FILE NOT FOUND":
            print response
        else:
            dataPort = int(sys.argv[5])
            receiveFileData(dataPort)
    clientSocket.shutdown(1)
    clientSocket.close()
    return 0

# Name: establishConnection(host, controlPort)
# Desc: Establishes connection with ftp server
# Args: port number for dataport
# Return: socket for using control connection
def establishConnection(host, controlPort):
    clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    clientSocket.connect((host, controlPort))
    return clientSocket

if __name__ == "__main__":

    if len(sys.argv) < 5 or len(sys.argv) > 6:
        print "Arguement count is incorrect.  Arguement count is incorrect.  Must enter: python <ftclient.py> <host> <controlport#> <cmd> <dataport#>"
        exit(1)
    var1 = sys.argv[3]
    if var1 not in ['-g', '-l']:
        print "Use correct commands: -l for directory or -g to retieve a file"
        exit(1)
    host = sys.argv[1]
    controlPort = int(sys.argv[2])
    clientSocket = establishConnection(host, controlPort)
    makeRequest(clientSocket)
