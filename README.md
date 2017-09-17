# UDP_Client_Server
Client Server communication using Socket Programming in C and UDP Sockets


This code connects client and server in a loop.
Client displays following commands to user, and performs respective operations depending on user input. User to give inputs as follows - 

get<newline>filename<newline>
put<newline>filename<newline>
ls<newline>anytext<newline>
exit<newline>anytext<newline>
  
Inputs given in this manners are recorded and sent to the respective if else if condition statement where server and client communication is established.The if..else if..else loop has been defined for incorporating all four types of user input.

Depending on what input the user gives, command (for e.g get) <newline> filename(for e.g foo2), that particular 'if' condition is selected, and inside the if loop for that option, the communication between client and server takes place.

There are two programs in this directory. These programs accomplish communication between server and client using UDP protocol.

The clientv2.c displays the dashboard to the user for above mentioned command options and takes action based on user input. serverv2.c first establishes connection with client, by creating a socket, and binding that socket. Once the soocket is bound connection with client in created and the server can receive commands back and forth with client.Once user gives input, corresponding if statement is run and depending on what command user has selected, the client side, sends the commmand type to server side in order to carry out server side operations related to that command. There are do while loops in both client and server, which exits when user exits from program.(syntax - exit<newline>anytext<newline>)

get - filename, transfers file from server to client directory (e.g foo1,foo2,foo3 which are located in the directory where both source codes are located.
put - filename, transfers file from client to server.

ls command (syntac - ls<newline>anytext<newline> displays the list of files in server. This command has been implemented using system() call, where in client passes the command to server, and server executes the received command on its system console and output is received as standard output.
exit command runs on the similar lines using system() call to exit command in linux. Packet size for data transfer has been taken as 500 bytes and can be changed by changing value BUFLEN in source code and sendto() and recvfrom() call arguments.

How to compile the program - 
to compile the client program run 

make clientv2 ---run this command on the command line, in the directory where source code files are saved. 

to compile the server program run 
make serverv1 ---run this command on the command line, in the directory where source code files are saved

to run the client executable
./clientv2 <ipaddr of server> <portno>
to run the server executable
./serverv2 <same portno as server>

After executing the program, integrity of files can be assured by checking the md5sum of original and the transferred files and it should be the same.

For implementing reliability, this program covers following logic - 
1. for first data packet transferred, the packet which is transferred is also stored in a temp buffer by the sender side. Sender waits for an acknowledgement from the received with a timeout value of 1 sec. If an ACK is received from receiver, within the timeout value, the for loop is taken to next iteration and next packet is trasnferred. If ACK does not occur within timeout value, the iteration value (i) is set to i-1 and loop is continued, thus data would be sent again.
Difference between duplicate and original acknowledgements has been coded into program, by ACK0 - original acknowledgement and ACK1, duplicate acknowledgement.

2. Both sender and receiver, before transmitting the data packets,store previously sent data packet in a temp buffer and each time,check if the new packet that has been received is a new original packet or duplicate of previous packet. As sender waits for receiver ack and packets do not arrive out of order being one tranfer-one Ack at a time, reliability is ensure by this technique. Also, if any ACks are lost, sender would not receive ACK, so it would resend previous packet after timeout. Receiver checks if this is original packet or duplicate packet,if duplicate it sends duplicate ACK, i.e ACK1, to continue the process forward.

I tried to check the reliability by transferring a 1MB file, during which I observed that for ACKs which were not received within timeout period
data packets were resent and duplicate ACKs helped to understand which type of data packet has been received. This, combined with md5sum integrity makes a case for up for reliability of data.  
