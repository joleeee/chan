/*
AUTHOR: Abhijeet Rastogi (http://www.google.com/profiles/abhijeet.1989)

This is a very simple HTTP server. Default port is 10000 and ROOT for the server is your current working directory..

You can provide command line arguments like:- $./a.aout -p [port] -r [path]

for ex.
$./a.out -p 50000 -r /home/
to start a server at port 50000 with root directory as "/home"

$./a.out -r /home/shadyabhi
starts the server at port 10000 with ROOT as /home/shadyabhi

*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>
#include<ctype.h>

#define CONNMAX 1000
#define BYTES 1024

void urldecode(char *dst, const char *src)
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else if (*src == '+') {
                        *dst++ = ' ';
                        src++;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}

char *ROOT;
int listenfd, clients[CONNMAX];
void error(char *);
void startServer(char *);
void respond(int);

int main(int argc, char* argv[])
{
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;

	//Default Values PATH = ~/ and PORT=10000
	char PORT[6];
	ROOT = getenv("PWD");
	strcpy(PORT,"10000");

	int slot=0;

	//Parsing the command line arguments
	while ((c = getopt (argc, argv, "p:r:")) != -1)
		switch (c)
		{
			case 'r':
				ROOT = malloc(strlen(optarg));
				strcpy(ROOT,optarg);
				break;
			case 'p':
				strcpy(PORT,optarg);
				break;
			case '?':
				fprintf(stderr,"Wrong arguments given!!!\n");
				exit(1);
			default:
				exit(1);
		}

	printf("Server started at port no. %s%s%s with root directory as %s%s%s\n","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
	// Setting all elements to -1: signifies there is no client connected
	int i;
	for (i=0; i<CONNMAX; i++)
		clients[i]=-1;
	startServer(PORT);

	// ACCEPT connections
	while (1)
	{
		addrlen = sizeof(clientaddr);
		clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

		if (clients[slot]<0)
			error ("accept() error");
		else
		{
			if ( fork()==0 )
			{
				respond(slot);
				exit(0);
			}
		}

		while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
	}

	return 0;
}

//start server
void startServer(char *port)
{
	struct addrinfo hints, *res, *p;

	// getaddrinfo for host
	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo( NULL, port, &hints, &res) != 0)
	{
		perror ("getaddrinfo() error");
		exit(1);
	}
	// socket and bind
	for (p = res; p!=NULL; p=p->ai_next)
	{
		listenfd = socket (p->ai_family, p->ai_socktype, 0);
		if (listenfd == -1) continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	if (p==NULL)
	{
		perror ("socket() or bind()");
		exit(1);
	}

	freeaddrinfo(res);

	// listen for incoming connections
	if ( listen (listenfd, 1000000) != 0 )
	{
		perror("listen() error");
		exit(1);
	}
}

//client connection
void respond(int n)
{
	char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
	int rcvd, fd, bytes_read;

	memset( (void*)mesg, (int)'\0', 99999 );

	rcvd=recv(clients[n], mesg, 99999, 0);

	char mesg2[99999];
	memcpy(mesg2, mesg, 99999);
	char mesg3[99999];
	memcpy(mesg3, mesg, 99999);

	if (rcvd<0)    // receive error
		fprintf(stderr,("recv() error\n"));
	else if (rcvd==0)    // receive socket closed
		fprintf(stderr,"Client disconnected upexpectedly.\n");
	else    // message received
	{
		/*printf("[\n%s]\n", mesg);*/
		reqline[0] = strtok (mesg, " \t\n");
		if ( strncmp(reqline[0], "GET\0", 4)==0 )
		{
			reqline[1] = strtok (NULL, " \t");
			reqline[2] = strtok (NULL, " \t\n");
			if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
			{
				write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
			}
			else
			{
				if ( strncmp(reqline[1], "/\0", 2)==0 )
					reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...

				char* reqs[3];
				int reql = 0;
				if (strncmp(reqline[1], "/index.html", 11) == 0){
					char* preindex = "/preindex.html";
					char* postindex = "/postindex.html";
					//reqs = {preindex, "/data", postindex};
					reqs[0] = preindex;
					reqs[1] = "/data";
					reqs[2] = postindex;
					reql = 3;
				}
				else{
					reqs[0] = reqline[1];
					reql = 1;
				}

				// do all the files exist?
				int fof = 0;
				for(int i = 0; i < reql; i++){
					strcpy(path, ROOT);
					strcpy(&path[strlen(ROOT)], reqs[i]);
					if ( (fd=open(path, O_RDONLY))==-1 ){
						fof=1;
						printf("ERR 404d on local file %s!\n", path);
						break;
					}
				}

				if(fof){
					write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
				}
				else{
					write(clients[n], "HTTP/1.0 200 OK\n\n", 17);
				}

				// print all the files
				for(int i = 0; i < reql; i++){
					strcpy(path, ROOT);
					strcpy(&path[strlen(ROOT)], reqs[i]);
					if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
					{
						while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
							write (clients[n], data_to_send, bytes_read);
					}
				}
				printf("done\n");
			}
		}
		if ( strncmp(reqline[0], "POST\0", 5)==0 ) {
			printf("you got mail\n");
			reqline[1] = strtok (NULL, " \t");
			reqline[2] = strtok (NULL, " \t\n");

			char proper[99999];
			urldecode(proper, mesg3);

			int illegal = 0;
			for(size_t i = 0; i < 99999; i++)
				if(proper[i] == '<' || proper[i] == '>'){
					illegal = 1;
					break;
				}

			if( illegal || ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 ))
			{
				printf("Bad request!\n");
				write(clients[n], "HTTP/1.0 400 Bad Request\n\n", 26);
				char* info = "XSS is not permitted! >:)\n";
				write(clients[n], info, strlen(info));
			}
			else
			{
				char *line, *last;
				line = strtok(mesg2, "\r\n");
				while(line != NULL){
					printf("loop%s\n", line);
					last = line;
					line = strtok(NULL, "\r\n");
				}

				char rname[99999], rmessage[99999]; // these are kinda long, should limit %s
				if (sscanf(last, "\nname=%[^&]&message=%s", rname, rmessage) == 2){
					printf("name:%s, message:%s\n", rname, rmessage);
				}

				char name[99999], message[99999];
				urldecode(name, rname);
				urldecode(message, rmessage);

				if(strcmp(name, "") == 0)
					strncpy(name, "Anon\0", 5);

				// RACE CONDITION!!!! (me thinks)
				FILE *fptr = fopen("data", "a");
				fputs("<hr><em>", fptr);
				fputs(name, fptr);
				fputs("</em> says:\n<pre>", fptr);
				fputs(message, fptr);
				fputs("</pre>\n\n", fptr);

				const char* o = "HTTP/1.0 303 See Other\nLocation: /\n\n";
				write(clients[n], o, strlen(o));
			}
		}
	}

	//Closing SOCKET
	shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(clients[n]);
	clients[n]=-1;
}

