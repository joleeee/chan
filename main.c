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

#include"str.c"

#define WRITE(client, string) write(client, string, strlen(string))

#define CONNMAX 1000
// is this like packet size?
#define BYTES 1024

#define STRLEN 99999

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
	char path[STRLEN], data_to_send[BYTES];
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
	char path[STRLEN], data_to_send[BYTES];
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

#define HASH_THREAD 6954056392669
#define HASH_NAME 6385503302
#define HASH_MESSAGE 229474704965354
#define HASH_IMG 193495042

struct POSTR{
	char *thread, *message, *name, *img;
};

void FREE_POSTR(struct POSTR *postr){
	if(postr->thread)
		free(postr->thread);
	if(postr->message)
		free(postr->message);
	if(postr->name)
		free(postr->name);
	if(postr->img)
		free(postr->img);
}

void COPY_STRING(char** optr, char* val){
	char* ptr = *optr;
	if(ptr)
		free(ptr);
	ptr = strdup(val);
	*optr = ptr;
}

void getpostreqs(char *msg, struct POSTR *postreq){
	char *a = msg;
	char *b;
	/*char *b = strtok(NULL, "&");*/
	while(1){
		char *key, *value=NULL;
		b=a;
		while(*b != '=' && *b != '\0')
			b++;
		if(*b == '\0') // we hit end of file while looking for key, so missing value, just return!
			break;
		*b='\0';
		key=a;
		a=b+1;

		if(*a != '\0'){ // if we don't start at the end then find it
			b=a;
			while(*b != '&' && *b != '\0')
				b++;
			*b='\0';
			value=a;
			a=b+1;
		}

		char empty[1] = "";
		if(!value){ // if we are at the end then set it to emptystring
			value=empty;
		}

		switch(hash(key)){
			case HASH_THREAD:
				COPY_STRING(&postreq->thread, value);
				break;
			case HASH_MESSAGE:
				{
					char *esc1 = escape(value, ">", "&gt;");
					char *esc2 = escape(esc1, "<", "&lt;");
					char *esc3 = escape(esc2, "\"", "&quot;");
					COPY_STRING(&postreq->message, esc3);
					free(esc1);
					free(esc2);
					free(esc3);
				}
				break;
			case HASH_NAME:
				if(isalphanumerical(value))
					COPY_STRING(&postreq->name, value);
				break;
			case HASH_IMG:
				COPY_STRING(&postreq->img, value);
				break;
			default:
				printf("err %s = %ld not recognized\n", key, hash(key));
				break;
		}
end:
		;
	}
}

//client connection
void respond(int n)
{
	char mesg[STRLEN], *reqline[3], data_to_send[BYTES], path[STRLEN];
	int rcvd, fd, bytes_read;

	memset( (void*)mesg, (int)'\0', STRLEN );

	rcvd=recv(clients[n], mesg, STRLEN, 0);

	char mesg2[STRLEN];
	memcpy(mesg2, mesg, STRLEN);
	char mesg3[STRLEN];
	memcpy(mesg3, mesg, STRLEN);

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
				else if(strncmp(reqline[1], "/\0", 2) == 0){
					printf("info: index\n");

					WRITE(clients[n], "HTTP/1.0 200 OK\n\n");

					sendfile(n, "/precatalog.html");

					DIR *dir;
					int listc=0;
					char list[100][100];
					// ->d_type != DT_REG
					if(dir = opendir(ROOT)){
						struct dirent *files;
						while((files=readdir(dir)) != NULL){
							char *name = files->d_name;
							if(strncmp(name, ".", 1) != 0 &&
								strlen(name) >= strlen("a.thread") &&
								strncmp(&name[strlen(name)-strlen(".thread")], ".thread", strlen(".thread")) == 0){

								struct stat attrib;
								// TODO NEED TO PREPEND ROOT
								stat(files->d_name, &attrib);
								char date[30];
								strftime(date, 30, "%Y-%m-%d %H:%M UTC", gmtime(&(attrib.st_ctime)));
								if(listc < 100){
									strncpy(list[listc], date, 30);
									int idx = strlen(date);
									strncpy(&list[listc][idx], "+", 1);
									strncpy(&list[listc][idx+1], files->d_name, 68);
									listc++;
								}
							}
						}
					}
					char** slist = malloc(sizeof(char*) * listc);
					for(int i = 0; i < listc; i++)
						slist[i] = (char*)&list[i];

					qsort(slist, listc, sizeof(char *), revcmpstr);

					char buff[1000];
					for(int i = 0; i < listc; i++){
						char* date = strtok(slist[i], "+");
						char* name = strtok(NULL, "\0");
						snprintf(buff, 1000,
								"<a href=\"%s\"><time class=\"posttime\">%s</time> %s</a><br>", name, date, name);
						WRITE(clients[n], buff);
					}
					free(slist);

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

			if(strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0)
			{
				printf("info: Bad request!\n");
				WRITE(clients[n], "HTTP/1.0 400 Bad Request\n\n");
				goto done;
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

				char bigdump[STRLEN];
				urldecode(bigdump, last);
				struct POSTR postreq = {NULL};
				getpostreqs(bigdump, &postreq);
				/*printf("info: thread[%s] name:[%s], message:[%s] img:[%s]\n", postreq.thread, postreq.name, postreq.message, postreq.img);*/
				if(!postreq.message || !postreq.name){
					printf("warn: missing post args, message, name and img required\n");
					printf("message: %d | name: %d | img: %d\n", !!postreq.message, !!postreq.name, !!postreq.img);
					WRITE(clients[n], "HTTP/1.0 400 Bad Request\n\n");
					WRITE(clients[n], ">:)\n");
					goto done;
				}

				if(!postreq.thread){
					char reqline1raw[100], reqline1[100];
					strncpy(reqline1raw, &reqline[1][1], 100);
					urldecode(reqline1, reqline1raw);
					COPY_STRING(&postreq.thread, reqline1);
					printf("info: no thread name, set to %s\n", postreq.thread);
				}

				if(strlen(postreq.thread) < 4){
					printf("warn: too short");
					WRITE(clients[n], "HTTP/1.0 400 Bad Request\n\n");
					WRITE(clients[n], "Too short thread name, at least 4 characters required");
					goto done;
				}

				// now postreq.* are all set to at least ""

				if(strlen(postreq.name) < 1){
					COPY_STRING(&postreq.name, "Ola");
				}

				if(strlen(postreq.thread) < strlen("a.thread") || strlen(postreq.thread) >= strlen("a.thread") && strncmp(&postreq.thread[strlen(postreq.thread)-strlen(".thread")], ".thread", strlen(".thread")) != 0){
					printf("appending .thread\n");
					strncpy(&postreq.thread[strlen(postreq.thread)], ".thread\0", 8);
				}

				if(strlen(postreq.message) == 0){
					printf("warn: msg empty");
					WRITE(clients[n], "HTTP/1.0 400 Bad Request\n\n");
					WRITE(clients[n], "You have to say something!\n");
					goto done;
				}

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
				strcpy(&path[strlen(ROOT)+1], postreq.thread);

				// RACE CONDITION!!!! (me thinks)
				printf("appending to [%s]\n", path);
				FILE *fptr = fopen(path, "a");
				fprintf(fptr, "<hr><div class=\"post\"><span class=\"titlebar\"><span class=\"author\">%s</span> <time class=\"posttime\">%s</time></span><br>", postreq.name, timed);
				if(postreq.img && strlen(postreq.img) > 1){
					fprintf(fptr, "<a href=\"%s\"><img src=\"%s\" /></a><br>", postreq.img, postreq.img);
				}
				fprintf(fptr, "<div class=\"postcontent\">");
				char *token = strtok(postreq.message, "\n");
				while(token != NULL){
					// remove leading whitespace
					while(*token == ' ')
						token++;
					int green = strncmp(token, "&gt;", 4) == 0;
					int pink = strncmp(token, "&lt;", 4) == 0;
					fprintf(fptr, "%s%s%s<br>",
							(pink||green)?green?"<span class=\"greentext\">":"<span class=\"pinktext\">":"",
							token,
							(pink||green)?"</span>":"");
					token = strtok(NULL, "\n");
				}
				fprintf(fptr, "</div></div>\n\n");
				fclose(fptr);
				printf("closed file\n");

				WRITE(clients[n], "HTTP/1.0 303 See Other\nLocation: ");
				WRITE(clients[n], &reqline[1][1]);
				WRITE(clients[n], "\n\n");

				printf("info: Redirecting to %s\n", reqline[1]);





				//free postr and close sockets
done:
				FREE_POSTR(&postreq);
			}
		}
	}
	//Closing SOCKET
	shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(clients[n]);
	clients[n]=-1;
}
