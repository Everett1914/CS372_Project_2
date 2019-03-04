# client program
import socket

HOST = 'localhost'    # The remote host
PORT = 50058          # The same port as used by the server
clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientSocket.connect((HOST, PORT))
while 1:
   message = raw_input('Client > ')
   if message == "/quit":
       clientSocket.shutdown(socket.SHUT_RDWR)
       clientSocket.close()
       break
   else:
       clientSocket.send(message.encode())
       message = clientSocket.recv(1024)
       message = message.decode()
       print 'Server > ',message
