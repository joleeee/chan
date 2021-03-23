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
#include<stdarg.h>
#include<dirent.h>
#include<time.h>

#define WRITE(client, string) write(client, string, strlen(string))

#define CONNMAX 1000
// is this like packet size?
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

		// could this potentially get stuck? what if a small % of connections never close
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

void sendfile(int n, char *file){
	char path[99999], data_to_send[BYTES];
	int fd, bytes_read;
	strcpy(path, ROOT);
	strcpy(&path[strlen(ROOT)], file);
	if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
	{
		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
			write (clients[n], data_to_send, bytes_read);
	}

}

void sendfiles(int n, int argc, ...){
	va_list valist, valist2;
	va_start(valist, argc);
	va_copy(valist2, valist);

	// do all the files exist?
	char path[99999], data_to_send[BYTES];
	int fd, bytes_read;
	for(int i = 0; i < argc; i++){
		strcpy(path, ROOT);
		strcpy(&path[strlen(ROOT)], va_arg(valist, char*));
		if ( (fd=open(path, O_RDONLY))==-1 ){

			WRITE(clients[n], "HTTP/1.0 404 Not Found\n\n"); //FILE NOT FOUND
			WRITE(clients[n], "<!DOCTYPE html><html><body>404</body></html>");
			return;
		}
	}

	WRITE(clients[n], "HTTP/1.0 200 OK\n\n");
	// print all the files
	for(int i = 0; i < argc; i++){
		strcpy(path, ROOT);
		strcpy(&path[strlen(ROOT)], va_arg(valist2, char*));
		if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
		{
			while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
				write (clients[n], data_to_send, bytes_read);
		}
	}

	va_end(valist);
	va_end(valist2);
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
		reqline[0] = strtok (mesg, " \t\n");	// "GET"
		reqline[1] = strtok (NULL, " \t");	// "/data.thread"
		reqline[2] = strtok (NULL, " \t\n");	// "HTTP/1.1"
		printf("[%s%s %s%s]\n", "\033[92m", reqline[0], reqline[1], "\033[0m");
		if ( strncmp(reqline[0], "GET\0", 4)==0 )
		{
			if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
			{
				WRITE(clients[n], "HTTP/1.0 400 Bad Request\n");
			}
			else
			{
				if (strlen(reqline[1]) >= strlen("/a.thread") && strncmp(&reqline[1][strlen(reqline[1])-strlen(".thread")], ".thread", strlen(".thread")) == 0) {
					printf("info: thread\n");
					sendfiles(n, 3,
							"/preindex.html",
							reqline[1],
							"/postindex.html");
				}
				/*else if(strlen(reqline[1]) == strlen("/index.html") && strncmp(reqline[1], "/index.html", strlen("/index.html")) == 0){*/
				else if(strncmp(reqline[1], "/\0", 2) == 0){
					printf("info: index\n");

					WRITE(clients[n], "HTTP/1.0 200 OK\n\n"); //FILE NOT FOUND

					sendfile(n, "/precatalog.html");

					DIR *dir;
					// ->d_type != DT_REG
					if(dir = opendir(ROOT)){
						struct dirent *files;
						while((files=readdir(dir)) != NULL){
							char *name = files->d_name;
							if(strncmp(name, ".", 1) != 0 &&
								strlen(name) >= strlen("a.thread") &&
								strncmp(&name[strlen(name)-strlen(".thread")], ".thread", strlen(".thread")) == 0)
									{
								WRITE(clients[n], "<a href=\"");
								WRITE(clients[n], files->d_name);
								WRITE(clients[n], "\">");
								WRITE(clients[n], files->d_name);
								WRITE(clients[n], "</a><br>");
							}
						}
					}
					sendfile(n, "/postcatalog.html");
				}
				else{
					printf("info: file\n");
					sendfiles(n, 1, reqline[1]);
				}
			}
		}
		if ( strncmp(reqline[0], "POST\0", 5)==0 ) {
			printf("info: you got mail\n");

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
				printf("info: Bad request!\n");
				WRITE(clients[n], "HTTP/1.0 400 Bad Request\n\n");
				WRITE(clients[n], "XSS is not permitted! >:)\n");
			}
			else
			{
				char *line, *last;
				line = strtok(mesg2, "\r\n");
				while(line != NULL){
					/*printf("loop%s\n", line);*/
					last = line;
					line = strtok(NULL, "\r\n");
				}

				char rthread[99999], rname[99999], rmessage[99999], rimg[99999]; // these are kinda long, should limit %s
				/*if (sscanf(last, "\nname%[^&]&message%[^&]&img%s", rname, rmessage, rimg) != 3){*/
				int argc;
				if ((argc = sscanf(last, "\nthread%[^&]&name%[^&]&message%[^&]&img%s", rthread, rname, rmessage, rimg)) == 4){
					/*printf("warn: missing post args (4)\n");*/
				}
				else if ((argc = sscanf(last, "\nname%[^&]&message%[^&]&img%s", rname, rmessage, rimg)) != 3){
					printf("warn: missing post args (3)\n");
					WRITE(clients[n], "HTTP/1.0 400 Bad Request\n\n");
					WRITE(clients[n], ">:)\n");
					shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
					close(clients[n]);
					clients[n]=-1;
					return;
				}
				printf("argc %d\n", argc);
				if(argc == 3){
					strncpy(rthread, reqline[1], 100); //99999 would be proper..
				}
				char thread[99999], name[99999], message[99999], img[99999];
				if(strlen(rthread) < 4){
					printf("warn: too short");
					WRITE(clients[n], "HTTP/1.0 400 Bad Request\n\n");
					WRITE(clients[n], "Too short thread name, at least 4 required");
					shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
					close(clients[n]);
					clients[n]=-1;
					return;
				}

				urldecode(thread, rthread+1);
				urldecode(name, rname+1); // +1 to skip '='
				urldecode(message, rmessage+1);
				urldecode(img, rimg+1);

				if(strlen(name) == 0)
					strncpy(name, "Ola\0", 4);

				if(strlen(thread) < strlen("a.thread") || strlen(thread) >= strlen("a.thread") && strncmp(&thread[strlen(thread)-strlen(".thread")], ".thread", strlen(".thread")) != 0){
					printf("appending .thread\n");
					strncpy(&thread[strlen(thread)], ".thread\0", 8);
				}

				printf("info: thread[%s] name:[%s], message:[%s] img:[%s]\n", thread, name, message, img);
				if(strlen(message) == 0){
					printf("warn: msg empty");
					WRITE(clients[n], "HTTP/1.0 400 Bad Request\n\n");
					WRITE(clients[n], "You have to say something!\n");
				}
				else
				{
					time_t rawtime;
					struct tm *timeinfo;
					time(&rawtime);
					timeinfo=gmtime(&rawtime);
					char timed[10000];
					sprintf(timed, "%04d-%02d-%02d %02d:%02d:%02d UTC",
							timeinfo->tm_year+1900,
							timeinfo->tm_mon+1,
							timeinfo->tm_mday,
							timeinfo->tm_hour,
							timeinfo->tm_min,
							timeinfo->tm_sec);

					strcpy(path, ROOT);
					strcpy(&path[strlen(ROOT)], "/\0");
					strcpy(&path[strlen(ROOT)+1], thread);

					// RACE CONDITION!!!! (me thinks)
					printf("appending to [%s]\n", path);
					FILE *fptr = fopen(path, "a");
					fprintf(fptr, "<hr><div class=\"post\"><span class=\"titlebar\"><span class=\"author\">%s</span> <time class=\"posttime\">%s</time></span><br>", name, timed);
					if(strlen(img) > 0){
						fprintf(fptr, "<a href=\"%s\"><img src=\"%s\" /></a><br>", img, img);
					}
					fprintf(fptr, "<pre class=\"postcontent\">%s</pre></div>\n\n", message);

					WRITE(clients[n], "HTTP/1.0 303 See Other\nLocation: ");
					WRITE(clients[n], &reqline[1][1]);
					WRITE(clients[n], "\n\n");

					printf("info: Redirecting to %s\n", reqline[1]);
				}
			}
		}
	}

	//Closing SOCKET
	shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(clients[n]);
	clients[n]=-1;
}

