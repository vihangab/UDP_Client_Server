#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#define BUFLEN 500

int main(int argc, char * argv[])
{
	if(argc < 2)
        {
			printf("USAGE:  <port>\n");
			exit(1);
		}
	  char cmd[5];
	do
	{
		struct sockaddr_in servaddr,cliaddr;
		bzero(&servaddr,sizeof(servaddr));                    //zero the struct
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr= INADDR_ANY; //supplies the IP address of the local machine//address family
		servaddr.sin_port = htons(atoi(argv[1]));
		int nbytes;                        //number of bytes we receive in our message

	/******************
		  This code populates the sockaddr_in struct with
		  the information about our socket
		 ******************/		
		int sock = socket(AF_INET,SOCK_DGRAM,0);
		if( sock < 0)
		{
			printf("unable to create socket");
		}
		printf("socket function value is %d\n",sock);
		/*******************
		  Once we've created a socket, we must bind that socket to the 
		  local address and port we've supplied in the sockaddr_in struct
		 *****************/
		int b=bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if(b<0)
		{
			printf("unable to bind socket on server....\n");
		}
		printf("bind function value is ....%d\n",b);
		int len=sizeof(cliaddr);
		int cliaddrlen=sizeof(struct sockaddr);

		int nrec=recvfrom(sock,cmd,sizeof(cmd),0,(struct sockaddr *)&cliaddr, &len);
		if (nrec<0)
		{
		  printf("error in recieving command\n");
		  exit(1);
		}
		printf("comm is  %s\n",cmd);
		if(strncmp(cmd,"get",strlen("get"))==0) 
		{
				char buf[500];
				char fbuf[5000];

				  int nbytes=recvfrom(sock,buf,sizeof(buf),0,(struct sockaddr *)&cliaddr,&len);//receive command
				  printf("command Bytes received %d\n",nbytes);
				  printf(" command received  %s\n",buf);
				 FILE *fp;
				  fp=fopen(buf,"r");
				  if(fp==NULL)
					{
					  printf("file not found\n");
					}
				  fseek(fp,0,SEEK_END);
				  size_t fsize=ftell(fp);
				  fseek(fp,0,SEEK_SET);
					int *sbuf;
					int fsize1=(int)fsize;
					sbuf=&fsize1;
					printf("total file size...%d\n",*sbuf);

					int sentsize=sendto(sock,sbuf,sizeof(sbuf),0,(struct sockaddr *)&cliaddr,len); 
					if (sentsize<0)
					{
						printf("error in sending the file size\n");
						exit(1);
					}
					int i;
					int npack;
					int nresidual;
					npack=fsize1/BUFLEN;
					printf("npack....%d\n",npack);
					nresidual=fsize1%BUFLEN;
					printf("nresidual....%d\n",nresidual);
					char recvack[4];
					for(i=1;i<=npack;i++)
					{
						if(fread(fbuf,500,1,fp)<0)
						{
						  printf("unable to copy file into buffer\n");
						  exit(1);
						}
						int sentfile=sendto(sock,fbuf,500,0,(struct sockaddr *)&cliaddr,len);     
						if (sentfile<0)
						{
						 printf("error in sending the file\n");
						 exit(1);
						}
						printf("i is %d\n",i);
						printf("file size being sent   %d\n",sentfile);
						while(1)
						{
							int recack=recvfrom(sock,recvack,sizeof(recvack),0,(struct sockaddr *)&cliaddr,&len);
							printf("ack bytes%s\n",recvack);
							if (recack<=0)
							{
							 printf("ACK not received\n");
								int s = sleep(1);//timeout
								printf("packet resent no %d",i);
								i=i-1;
								break;
							}
							if(strcmp(recvack,"ACK0")==0||strcmp(recvack,"ACK1")==0)
							{
								printf("received ACK\n");
								break;
							}
						}
					}
					if(fread(fbuf,nresidual,1,fp)<0)
					{
					  printf("unable to copy file into buffer\n");
					  exit(1);
					}
					while(1)
					{	
					int sentfile=sendto(sock,fbuf,nresidual,0,(struct sockaddr *)&cliaddr,len);     
					 if (sentfile<=0)
					 {
						 printf("error in sending the file\n");
						 exit(1);
					 }
					printf("file size being sent   %d\n",sentfile);
							int recack=recvfrom(sock,recvack,sizeof(recvack),0,(struct sockaddr *)&cliaddr,&len);
							if (recack<=0)
							{
							 printf("ACK not received\n");
								int s = sleep(1);//timeout
								printf("packet resent no %d",i);
								continue;
							}
							if(strcmp(recvack,"ACK0")==0||strcmp(recvack,"ACK1")==0)
							{
								printf("received ACK\n");
								break;
							}
						}
			if (NULL != fp)
			{
				fclose(fp);
			}
					close(sock);
				}
		else if(strncmp(cmd,"put",strlen("put"))==0) 
		{	
				char buf[200];
				int sbuf[1];
			
				int filesize=recvfrom(sock,sbuf,sizeof(sbuf),0,(struct sockaddr *)&cliaddr, &cliaddrlen);//receive filesize
				if (filesize<0)
				{
				  printf("error in recieving the file\n");
				  exit(1);
				}
				printf(" filesize recved %d\n",sbuf[0]);

			  int nbytes=recvfrom(sock,buf,sizeof(buf),0,(struct sockaddr *)&cliaddr,&cliaddrlen);
				if (nbytes<0)
				{
				  printf("error in recieving the file\n");
				  exit(1);
				}						
			  printf("command Bytes received %d\n",nbytes);
			  printf(" command received  %s\n",buf);
			  char fbuf[sbuf[0]];
			  FILE *fp;
			  char new_file[]="copied";
			  strcat(new_file,buf);
				char temp[600];
				strcpy(temp,"initial");
				char origack[]= "ACK0";
				char duplack[]= "ACK1";
			printf("newfile name%s",buf);
			  fp=fopen(new_file,"w+");
				if (fp == NULL)
				{
					printf("File not created\n");
					exit(1);
				}
				int i;
				int npack;
				int nresidual;
				npack=(sbuf[0]/BUFLEN);
				printf("npack....%d\n",npack);
				nresidual=sbuf[0]%BUFLEN;
				printf("nresidual....%d\n",nresidual);
				for(i=1;i<=npack;i++)
				{

				  int nrec=recvfrom(sock,fbuf,500,0,(struct sockaddr *)&cliaddr, &cliaddrlen); 
				if (nrec<0)
				{
				  printf("error in recieving the file\n");
				  exit(1);
				}
				printf("i is%d\n",i);
				printf("rec bytes are.. is%d\n",nrec);
				if(nrec == 500)
				{
					if(strcmp(temp,fbuf)!=0)
					{
						strcpy(temp,fbuf);
						printf("Sending..%s\n",origack);
						int sentack=sendto(sock,origack,sizeof(origack), 0,(struct sockaddr *)&cliaddr, sizeof(struct sockaddr));
						if (sentack<=0)
						{
						  printf("error in sending ack\n");
						  exit(1);
						}	
						if(fwrite(fbuf,500,1,fp)<0)
						{
						  printf("error writting file\n");
						  exit(1);
						}
					}	
					else if(strcmp(temp,fbuf)==0)
					{
						printf("Sending ACK..%s\n",duplack);
						int sentack=sendto(sock,duplack,sizeof(duplack), 0,(struct sockaddr *)&cliaddr, sizeof(struct sockaddr));
						if (sentack<=0)
						{
						  printf("error in sending ack\n");
						  exit(1);
						}	
					}
				  }
				}
				int nrec=recvfrom(sock,fbuf,nresidual,0,(struct sockaddr *)&cliaddr, &cliaddrlen); 
				if (nrec<0)
				{
				  printf("error in recieving the file\n");
				  exit(1);
				}
				if(nrec == nresidual)
				{
					if(strcmp(temp,fbuf)!=0)
					{
						strcpy(temp,fbuf);
						printf("Sending ACK0..\n");
						int sentack=sendto(sock,origack,sizeof(origack), 0,(struct sockaddr *)&cliaddr, sizeof(struct sockaddr));
						if (sentack<=0)
						{
						  printf("error in sending ack\n");
						  exit(1);
						}	
						if(fwrite(fbuf,nresidual,1,fp)<0)
						{
						  printf("error writting file\n");
						  exit(1);
						}
					}	
					else if(strcmp(temp,fbuf)==0)
					{
						printf("Sending ACK1..\n");
						int sentack=sendto(sock,duplack,sizeof(duplack), 0,(struct sockaddr *)&cliaddr, sizeof(struct sockaddr));
						if(sentack<=0)
						{
						  printf("error in sending ack\n");
						  exit(1);
						}	
					}
				  }
				printf("size of written buffer %d\n",nrec);
			if (NULL != fp)
			{
				fclose(fp);
			}
				close(sock);
			}
		else if((strncmp(cmd,"ls",strlen("ls"))==0))
		{	
			
			char cbuf[10];
			char res[1000];
			printf("command Bytes received...%d\n",nrec);
			sprintf(cbuf,"%s\n", cmd);
			
				system(cbuf);
				fgets(res,sizeof(res),stdout);
				int nbytes=sendto(sock,res,sizeof(res),0,(struct sockaddr *)&cliaddr,len);
				if(nbytes<0)
			{
				printf("unable to send output of ls....\n");
			}
			close(sock);
		}
		else if(strncmp(cmd,"exit",strlen("exit"))==0)
				{
			char cbuf[10];
			char res[1000];
			printf("command Bytes received...%d\n",nrec);
			sprintf(cbuf,"%s\n", cmd);
				system(cbuf);
			close(sock);
			break;
		}
		else
		{
			printf("please enter again a valid command with valid parameters\n");
		}	
	}while(1);
return 0;
}
