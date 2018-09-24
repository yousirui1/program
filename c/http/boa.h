#ifndef _BOA_H
#define _BOA_H

#include <errno.h>
#include <stdlib.h>             /* malloc, free, etc. */
#include <stdio.h>              /* stdin, stdout, stderr */
#include <string.h>             /* strdup */
#include <ctype.h>
#include <time.h>               /* localtime, time */
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>             /* OPEN_MAX */
#include <setjmp.h>

#include <netdb.h>
#include <netinet/in.h>

#include <sys/mman.h>
#include <sys/select.h>
#include <sys/types.h>          /* socket, bind, accept */
#include <sys/socket.h>         /* socket, bind, accept, setsockopt, */
#include <sys/stat.h>           /* open */

#define SO_MAXCONN 250

#define set_nonblock_fd(fd) fcntl(fd, F_SETFL, NOBLOCK)

#define BOA_FD_SET(fd, where) { FD_SET(fd, where); \
    if (fd > max_fd) max_fd = fd; \
    }


#define SERVER_AF AF_INET
#define SOCKADDR sockaddr_in

#define NOBLOCK O_NONBLOCK

#define SOCKETBUF_SIZE 			8192
#define CLIENT_STREAM_SIZE		SOCKETBUF_SIZE
#define BUFFER_SIZE				CLIENT_STREAM_SIZE

#define CGI_ENV_MAX				50

#define MAX_HEADER_LENGTH		1024

#define REQUEST_TIMEOUT 		50



/***************** METHODS ***********************/

#define M_GET       1
#define M_HEAD      2
#define M_PUT       3
#define M_POST      4
#define M_DELETE    5
#define M_LINK      6
#define M_UNLINK    7


/***************** REQUEST STATUS (req->status) ******/
#define READ_HEADER             0
#define ONE_CR                  1
#define ONE_LF                  2
#define TWO_CR                  3
#define BODY_READ               4
#define BODY_WRITE              5
#define WRITE                   6
#define PIPE_READ               7
#define PIPE_WRITE              8
#define DONE            		9
#define DEAD                   10


#define KA_ACTIVE 		2

struct request 				//requests
{
	int fd;
	int status;				//
	time_t time_last;		//time of last
	char *pathname;			//path of request file
	int simple;				//simple request ?
	int keepalive;			//keepalive status
	int kacount;			//keepalive count

	int data_fd;			//fd of data
	unsigned long filesize; //file size
	unsigned long filepos;  //position in file
	char *data_mem;			
	int method;				//M_GET, M_POST

	char *logline;			//line to log file
	
	char *header_line;		//beginning of un or incompletely processed header line
	char *header_end; 		//last known end of header, or end of processed data
	int parse_pos;			//how much have we parsed
	int client_stream_pos;   //how much have we read..
	
	int buffer_start;		//where the buffer starts
	int buffer_end;			//where the buffer ends
	
	char *http_version;		//HTTP/?.?
	int response_status;	//R_NOT_FOUND 
	
	char *if_modified_since; //
	time_t last_modified;
	
	char local_ip_addr[NI_MAXHOST];	
	
	/* CGI vars */
	int remote_port;
	
	char remote_ip_addr[NI_MAXHOST]; //
	
	int is_cgi;	
	int cgi_status;
	int cgi_env_index;			// index into array

	/* Agent and referer for logfiles */
	char *header_user_agent;
	char *header_referer;
	
	int post_data_fd;			//fd for post data tmpfile


	/*cgi env var*/
	char *path_info;
	char *path_translated;
	char *script_name;
	char *query_string;
	char *content_type;
	char *content_length;	



	struct request *next;		//next
	struct request *prev;		//previous

	/* everything below this line is kept regardless */
	char buffer[BUFFER_SIZE + 1];  // generic I/O buffer
	char request_uri[MAX_HEADER_LENGTH + 1];  //uri
	char client_stream[CLIENT_STREAM_SIZE];   //data from client 
	char *cgi_env[CGI_ENV_MAX + 4] 	 //CGI environment
		

};


typedef struct request request;


extern int max_fd;
extern request *request_block; 
extern request *request_ready;
extern request *request_free;

extern time_t current_time;
extern int pending_request;
extern int ka_max;
extern int pending_requests;
extern int total_connections;
extern long int max_connections;
#endif
