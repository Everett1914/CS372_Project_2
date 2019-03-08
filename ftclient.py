# client program
import socket
import sys

#Helper
def openDataConnection(dPrt):
    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    serverSocket.bind(('', dPrt))
    serverSocket.listen(10)
    connection, addr = serverSocket.accept()
    return connection, serverSocket

#Helper
def receiveDirData(dPrt):
    print 'Receiving Directory Structure from ' + sys.argv[1] + ':  ' + sys.argv[2]
    dataConnection, serverSocket = openDataConnection(dPrt)
    message = dataConnection.recv(1024)
    print message
    return 0

def makeRequest(clientSocket):
    if sys.argv[3] == '-l':
        message = sys.argv[1] + ' ' +sys.argv[3] + ' ' + sys.argv[4]
        clientSocket.send(message.encode())
        dataPort = int(sys.argv[4])
        receiveDirData(dataPort)
        clientSocket.shutdown(1)
        clientSocket.close()
    elif sys.argv[3] == '-p':
        message = sys.argv[1] + ' ' +sys.argv[3] + ' ' + sys.argv[5]
        dataPort = int(sys.argv[5])
        receiveFileData()
        clientSocket.send(message.encode())
        clientSocket.shutdown(1)
        clientSocket.close()
    return 0

def establishConnection(host, controlPort):
    clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    clientSocket.connect((host, controlPort))
    return clientSocket

if __name__ == "__main__":

    if len(sys.argv) < 5 or len(sys.argv) > 6:
        print "Arguement count is incorrect.  Arguement count is incorrect.  Must enter: ./ftclient <host> <controlport#> <cmd> <dataport#>"
        exit(1)

    host = sys.argv[1]
    controlPort = int(sys.argv[2])
    clientSocket = establishConnection(host, controlPort)
    makeRequest(clientSocket)
