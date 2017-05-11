from socket import *
import sys

#get the message from the user
def getMessage(prompt):
  message = raw_input(prompt)
  while len(message) > 500:
    print 'That message is too long, please shorten it'
    message = raw_input(prompt)
  return message

#set up the connection
def connection(name, port):
  serverPort = port
  serverSocket = socket(AF_INET, SOCK_STREAM)
  serverSocket.bind((name, serverPort))
  serverSocket.listen(1)
  print 'The server is listening on port 9009'
  return serverSocket

#get the handle for the user on this end
def getHandle():
  userHandle = raw_input('Please enter a username: ')
  return userHandle + ' >'

#fix the extra newline at the end of the input
def formatOutput(recieved):
  recieved[recieved.len()] = " "

#start of the script
server = connection(str(sys.argv[1]), int(sys.argv[2]))
handle = getHandle()

while 1:
  print('Waiting on connection... ')
  connectionSocket, addr = server.accept()
  print('Somone connected!')
  message = getMessage(handle)

  while(message != '\quit'):
    connectionSocket.send(message)
    recieve = connectionSocket.recv(501)
    print (recieve)
    message = getMessage(handle)


