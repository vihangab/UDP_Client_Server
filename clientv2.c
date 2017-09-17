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
#include <time.h>
#define BUFLEN 500

int main(int argc,char * argv[])
{
	if(argc < 3)
        {
                printf("USAGE:  <server_ip> <server_port>\n");
                exit(1);
        }
	  char filename[10];
	  char cmd[5];
	
	do
	{
		printf("command menu...\n");
		printf("1. get<newline> filename<newline> \n");
		printf("2. put<newline> filename<newline> \n");
		printf("3. ls (command would list all files in server directory)\n");
		printf("4. exit\n");

	//get input from user
		scanf("%s",cmd);
		scanf("%s",filename);
		int sock;
		struct sockaddr_in servaddr,cliaddr;
		//bzero(&servaddr,sizeof(servaddr));
		bzero(&cliaddr,sizeof(cliaddr));
		int len=sizeof(cliaddr);
		int cliaddrlen=sizeof(struct sockaddr);
		cliaddr.sin_family = AF_INET;
		cliaddr.sin_addr.s_addr =inet_addr(argv[1]);
		cliaddr.sin_port = htons(atoi(argv[2]));
		// create socket in client side
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock<0)
		{
			printf(" socket not created in client\n");
			exit(1);
		}	
		
		if(strncmp(cmd,"get",strlen("get"))==0)
		{
				int nsent=sendto(sock,cmd,sizeof(cmd), 0,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr));
				if(nsent<0)
				{
					printf("error in sending command to server\n");
					exit(1);
				}
				char cbuf[200];
				int sbuf[1];

			// send  msg to server
				strcpy(cbuf,filename);
				nsent=sendto(sock,cbuf,sizeof(cbuf), 0,(struct sockaddr *)&cliaddr, sizeof(struct sockaddr));
				if (nsent<0)
				{
				  printf("error in sending the command\n");
				  exit(1);
				}
				int filesize=recvfrom(sock,sbuf,sizeof(sbuf),0,(struct sockaddr *)&cliaddr, &cliaddrlen);
				if (filesize<0)
				{
				  printf("error in recieving the file\n");
				  exit(1);
				}
				printf("filesize recvd  %d\n",sbuf[0]);
				char fbuf[sbuf[0]];
			  printf("size of received buffer %ld\n", sizeof(fbuf));
				int i;
				int npack, nresidual;
				npack=(sbuf[0]/BUFLEN);
				printf("npack..%d\n",npack);
				nresidual=(sbuf[0]%BUFLEN);
				printf("nresidual..%d\n",nresidual);
				char new_file[]="copied";
			  	strcat(new_file,cbuf); 
				FILE *fp;
				char temp[500];
				strcpy(temp,"initial");
				char origack[]= "ACK0";
				char duplack[]= "ACK1";
			  fp=fopen(new_file,"w+");
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
				
				printf("size of written buffer %d\n",nrec);
			if (NULL != fp)
			{
				fclose(fp);
			}
			  //close client side connection
			close(sock);
		}
		}
		else if(strncmp(cmd,"put",strlen("put"))==0) 
		{	
			int nsent=sendto(sock,cmd,sizeof(cmd), 0,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr));
				if(nsent<0)
				{
					printf("error in sending command to server\n");
					exit(1);
				}
				char cbuf[200];
				strcpy(cbuf,filename);
				FILE *fp;
				fp=fopen(cbuf,"r");
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
					printf("  file size %d\n",*sbuf);
					//send filesize to server
					int filesize=sendto(sock,sbuf,sizeof(sbuf),0,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr));
					if (filesize<0)
					{
					  printf("error in sending the filesize\n");
					  exit(1);
					}
					printf("filesize sent  %d\n",sbuf[0]);

			//send  filename to server

				nsent=sendto(sock,cbuf,sizeof(cbuf), 0,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr));
				if (nsent<0)
				{
				  printf("error in sending the command\n");
				  exit(1);
				}
				char fbuf[sbuf[0]];
				int i, npack, nresidual;
				npack=(sbuf[0]/BUFLEN);
				printf("npack..%d\n",npack);
				nresidual=(sbuf[0]%BUFLEN);
				printf("nresidual..%d\n",nresidual);
				char temp[600];
				strcpy(temp,"initial");
				char origack[]= "ACK0";
				char duplack[]= "ACK1";
				char recvack[4];
				for(i=1;i<=npack;i++)
				{	
					if(fread(fbuf,500,1,fp)<0)
					{
					  printf("error reading file\n");
					  exit(1);
					}
					printf("size of read buffer %ld\n", strlen(fbuf));
					nsent=sendto(sock,fbuf,500,0,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr)); 
					if (nsent<0)
					{
					  printf("error in sending the file\n");
					  exit(1);
					}
					printf("i is %d\n",i);
					printf("file size being sent   %d\n",nsent);
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
					int sentfile=sendto(sock,fbuf,nresidual,0,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr));     
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
			//close client side connection
		close(sock);
		}	
		else if(strncmp(cmd,"ls",strlen("ls"))==0)
		{
				int nsent=sendto(sock,cmd,sizeof(cmd), 0,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr));
				if(nsent<0)
				{
					printf("error in sending command to server\n");
					exit(1);
				}
			printf("bytes sent %d\n",nsent);
				char buf[1000];
				int result=recvfrom(sock,buf,sizeof(buf),0,(struct sockaddr *)&cliaddr, &cliaddrlen);
				if (result<0)
				{
				  printf("error in recieving the file\n");
				  exit(1);
				}
				fputs(buf,stdout);//prints the result received to standard output
				close(sock);
		}
		else if(strncmp(cmd,"exit",strlen("exit"))==0)
		{
			int nsent=sendto(sock,cmd,sizeof(cmd), 0,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr));
				if(nsent<0)
				{
					printf("error in sending command to server\n");
					exit(1);
				}
		break;
		}
		else
		{
			printf("please enter again a valid command with valid parameters\n");
		}
	}while(1);
return 0;
}
		
		
		
	

