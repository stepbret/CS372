#!/usr/bin/python

import socket 
import sys 
import os.path 

#If command is -l no file name is sent.
#If command is -g, filename sent to server.
#If command is invalid, ftserver evaluates and sends bad input prompt to client to print to console.
#Receives Acknowledgment messages.
def makeRequest( sock, userselect, fname, hport, sHost, sPort, dq ):
        if userselect == '-l':
                q = sock.send(userselect) # Sending user selection.
                if not q:
                        print "ERROR: writing to socket."
                        sock.close()
                        sys.exit(1)
                q = sock.recv(1024) #receive response if valid command
                if not q:
                        print "ERROR: reading from socket."
                        sock.close()
                        sys.exit(1)
                dq[0] = 1
        
        elif userselect == '-g':
                q = sock.send(userselect) # User selection
                if not q:
                        print "ERROR: writing to socket."
                        sock.close()
                        sys.exit(1)
                q = sock.recv(1024) #receive response if valid command
                if not q:
                        print "ERROR: reading from socket."
                        sock.close()
                        sys.exit(1)
                q = sock.send(fname) # File name to be sent. File name validated by ftserver.
                if not q:
                        print "ERROR: writing to socket."
                        sock.close()
                        sys.exit(1)
                q = sock.recv(1024) # Read in ACK from server.
                if not q:
                        print "ERROR: reading from socket."
                        sock.close()
                        sys.exit(1)
                dq[0] = 1
        
        else:                   # User input is other than '-g' and '-l'.
                q = sock.send(userselect) # Sending user selection.
                if not q:
                        print "ERROR: writing to socket."
                        sock.close()
                        sys.exit(1)
                q = sock.recv(1024) # Reading in invalid error prompt from server. 
                if not q:
                        print "ERROR: reading from socket."
                        sock.close()
                        sys.exit(1)
                q.rstrip()
                if ( q == "INVALID COMMAND\x00" ): 
                        print "Server:%d says %s." % (sPort, q)
                        dq[0] = 0
        return

#MAIN() begins.
#checking commands
if len(sys.argv) < 5: 
        print "USAGE: python ftclient.py <Hostname> <Port #(Comm)> <Command> [Filename] <Port #(Data Transfer)."
        sys.exit(1)

#checking if there are less than 5 parameters, user it trying to get a file
if len(sys.argv) > 5:

#Set up the connection.
        serverHost = socket.gethostbyname(sys.argv[1]) #hostname
        serverHost.strip()

        portno = sys.argv[2]  #Servers port number
        portno.strip()
        serverPort = int(portno)

        command = sys.argv[3] #users command 
        command.strip()

        fileName = sys.argv[4] #store name of file
        fileName.strip()

        dPort = sys.argv[5] #Data port
        dataportno = int(dPort)

else: # Four parameters specified on command line. User requesting dir print out with '-l'. 

        # Client Init to connect to server.
        serverHost = socket.gethostbyname(sys.argv[1]) # Hostname
        serverHost.strip()

        portno = sys.argv[2]  # Port number for communication with server.
        portno.strip()
        serverPort = int(portno)

        command = sys.argv[3] # User selection. Validate input to be '-g' or '-l'. 
        command.strip()

        dPort = sys.argv[4] 
        dataportno = int(dPort) # Port for data transfer.
        fileName = "empty"

try:
        mySocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 

except socket.error:
        print "ERROR: Cannot open socket, EXITING."
        sys.exit(1)

mySocket.connect(('', serverPort)) 

newSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
newSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
newSock.bind(('', dataportno))
newSock.listen(1)

q = mySocket.send(dPort)
if not q:
        print "ERROR: Initiating contact with Server."
        sock.close()
        sys.exit(1)

q = mySocket.recv(1024)
if not q:
        print "ERROR: Reading from Server."
        sock.close()
        sys.exit(1)

userSelect = [""]
makeRequest( mySocket, command, fileName, dataportno, serverHost, serverPort, userSelect )
if (userSelect[0] == 0):
        mySocket.close()
        sys.exit(1)

#connect
conn, addr = newSock.accept()

#if the command is get
if command == '-g': 
        q = conn.recv(1024) #receive whether filename exists
        if not q:
                print "ERROR: Reading from socket."
                conn.close()
                sys.exit(1)
                
        conn.send(q) 
        q.rstrip()

        if (q == "FILE NOT FOUND\x00"): 
                print "File not found in directory."

        print 'Data transfer in progress:  "%s" on port:%d' % (fileName, serverPort)
        serverFile = open(fileName, 'w') 
        q = conn.recv(1024)
        #get the file in chunks of 1024 bytes
        while (q):
                serverFile.write(q)
                q = conn.recv(1024) 
                
        serverFile.close()
        print "File transfer complete. Closing Connection."
        
if command == '-l':
        print "Requested directory contents from server on port :", serverPort
        while (True):
                q = conn.recv(1024) 
                if len(q) == 0: 
                        break
                print "%s" % (q)

mySocket.close() # Close socket and connection to server.
