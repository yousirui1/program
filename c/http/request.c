#include <stddef.h>
#include "boa.h"

int total_connections;

void free_request(request **list_head_addr, request *req);

request *new_request(void)
{
	request *req;
	
	if(request_free)
	{
		req = request_free;				//firest on free list
		dequeue(&request_free, request_free);   //dequeue the head
	}
	else
	{
		req = (request *)malloc(sizeof (request));
		if(!req)
		{
			return NULL;
		}

	}	

	return req;
}

void get_request(int server_s)
{
	int fd;
	struct SOCKADDR remote_addr;    
	struct SOCKADDR salocal;
	int remote_addrlen = sizeof(struct SOCKADDR);  //socket len
	request *conn; 
	size_t len;
	static int system_bufsize = 0;    //default size of SNDBUF given by system
	
	//remote_addr.S_FAMILY = 0xdead;
	fd = accept(server_s, (struct sockaddr *)&remote_addr,
						&remote_addrlen);
	if(fd == -1)
	{
		if(errno != EAGAIN && errno != EWOULDBLOCK)
			printf("accept err \r\n");
		else
			pending_requests = 0;
		return;	
	}
		
	if(fd >= FD_SETSIZE)
	{
	
		printf("fd > FD_SETSIZE \r\n");
		return;	
		close(fd);
		return;
	}	
	
	len = sizeof(salocal);
	
	if(getsockname(fd, (struct sockaddr *)&salocal, &len) != 0)
	{

		printf("getsockname err \r\n");
		close(fd);
		return;
	}

	conn = new_request();   //创建一个新的request
	if(!conn)
	{
		printf("new_request err\r\n");
		close(fd);
		return;
	}	

	conn->fd = fd;
	conn->status = READ_HEADER;
	conn->header_line = conn->client_stream;  
	conn->time_last = current_time;
	conn->kacount = ka_max;
	
	/* nonblocking socket */
	if(set_nonblock_fd(conn->fd) == -1)
		printf("nonblocking socket\r\n");
		
	/* set close on exec to true */
	if(fcntl(conn->fd, F_SETFD, 1) == -1)
		printf("close on exec \r\n");

	
	conn->remote_port = net_port(&remote_addr);
	
	total_connections++;

	enqueue(&request_ready, conn); //写队列
}


/************************************
 *  
 *
 * 处理一个就绪队列请求
 ***************************************/
void process_request(int server_s)
{
    int retval = 0;

	request *current, *trailer;

    if(pending_requests)
        get_request(server_s);       

    current = request_ready;

    while(current)
    {   
        time(&current_time);
        if(current->buffer_end &&        //there is data in the buffer                  
            current->status != DEAD && current->status != DONE)
        {   
            retval = req_flush(current);  //flush headers
    
            /* retval can be -2 = error -1 = blocked. or bytes left */
            if(retval == -2) 
            {   
                current->status = DEAD; 
                retval = 0;
            }   
            else if(retval >= 0)
            {   
                /* 只是解析headers */
                retval = 1;  
            }   
        }   
        else
        {   
            switch(current->status)
            {   
                case READ_HEADER:
                case ONE_CR:
                case ONE_LF:
                case TWO_CR:
					retval = read_header(current);
					break;
				case BODY_READ:
					retval = read_body(current);
					break;
				case WRITE:
					//retval = process_get(current);
					break;
				case PIPE_READ:
					//retval = read_from_pipe(current);
					break;
				case PIPE_WRITE:
					//retval = write_from_pipe(current);
					break;
				case DONE:
					//retval = req_flush(current);	
					/* retval -2 error  -1 blocked or bytes left */
					if(retval == -2)
					{
						current->status = DEAD;
						retval = 0;		
					}
					else if(retval > 0)
						retval = 1;
					break;		
				case DEAD:
					retval = 0;	
					current->buffer_end = 0;
					//SQUASH_KA(current);
					break;
				default:
					retval = 0;
					fprintf(stderr, "Unknow status (%s). "
							"closeing\n", current->status);
					current->status =DEAD;
					break;
            }   
        }   

        if(pending_requests)
            get_request(server_s);
       
        switch(retval)
        {   
            case -1:                    //request blocked
                trailer = current;     
				current = current->next;
				block_request(trailer);
				break;
            case 0:                     //request complete
				current->time_last = current_time;
				trailer = current;
				current = current->next;	
				free_request(&request_ready, trailer);	
				break;
            case 1:                     //more to do
				current->time_last = current_time;
				current = current->next;
				break;
            default:
				//log_error_time();	
            	fprintf(stderr, "Unknown retval in process.c - "
                    "Status: %d, retval: %d\n", current->status, retval);
				current = current->next;	
				break;

        }

    }
}




/********************************************
 *
 *
 *
 *
 * 解析http第一行
 *******************************************/
int process_logline(request *req)
{
	char *stop, *stop2;
	static char *SIMPLE_HTTP_VERSION = "HTTP/1.0";
	
	req->logline = req->client_stream;
	if(!memcmp(req->logline, "GET ", 4))
		req->method = M_GET;
	else if(!memcmp(req->logline, "HEAD ", 5))
		/* head is just get w/no body */
		req->method = M_HEAD;
	else if(!memcmp(req->logline, "POST ", 5))
		req->method = M_POST;
				
	else
	{
		//log_error_time();
		

		return 0;
	}

	req->http_version = SIMPLE_HTTP_VERSION;
	req->simple = 1;
	
	/* Guaranteed to find ' ' since we matched a method above */
	stop = req->logline + 3;
	if(*stop != ' ')
		++stop;
			
	/* scan to start of non-whitespcae */
	while(*(++stop) == ' ');				//取空格和空格之间的内容

	stop2 = stop;

	/* scan to end of non-whitespace */
	while(*stop2 != '\0' && *stop2 != ' ')
		++stop2;
	
	if(stop2 - stop > MAX_HEADER_LENGTH)
	{
		fprintf(stderr, "URI too long %d \"%s\"\n", MAX_HEADER_LENGTH,
					req->logline);
		return 0;
	}
	memcpy(req->request_uri, stop, stop2-stop);
	req->request_uri[stop2 - stop] = '\0';
	
	printf("uri %s \r\n", req->request_uri);
	if(*stop2 == ' ')
	{



	}

	return 1;

BAD_VERSION:
		
	return 0;	
}


int process_option_line(request *req)
{
	char c, *value, *line = req->header_line;
	
	value = strchr(line, ':');
	if(value == NULL)
	{
		log_error_doc(req);
		fprintf(stderr, "header \"%s\" dose not contain ':'\n", line);
		return 0;
	}
	*value ++= '\0';			//overwite the
	to_upper(line);				//header types are case-insensitive			
	
	while((c = *value) && (c == ' ' || c == '\t'))
		value++;
	
	if(c == '\0')
	{
		return 1;
	}	
	
	switch(line[0])
	{
		case 'A':  //ACCEPT
		{
			if(!memcmp(line, "ACCEPT", 7))
			{

			}
			break;
		}
		break;
	
		case 'C':
		{
			
		}	
		break;
		
		case 'H':
		if(!memcmp(line, "HOST", 5) && !req->header_host)
		{
			req->header_host = value;	
			return 1;
		}
		break;
		
		case 'I':
		if(!memcmp(line, "IF_MODIFIED_SINCE", 18) && !req->if_modified_since)
		{
			req->if_modified_since = value;	
			return 1;
		}
		break;
		case 'R':
			if(!memcmp(line, "REFERER", 8))
			{
				req->header_referer = value;	
				if(!add_cgi_env(req, "REFERER", value, 1))
					return 0;	
			}
			else if(!memcmp(line, "RANGE", 6))
			{
				if(req->ranges && req->ranges->stop == INT_MAX)
					return 1;
				else if(!range_parse(req, value))
				{
					send_r_invalid_rang(req);
					return 0;
				}
			}
			break;		
		
		case 'U':
			if(!memcmp(line, "USER_AGENT", 11))
			{
				req->header_user_agent = value;
				if(!add_cgi_env(req, "USER_AGENT", value, 1))
				{
					return 0;
				}
				return 1;
			}
			break;
		default:
			break;	
	}

	return add_cgi_env(req, line, value, 1);

}

void free_request(request **list_head_addr, request *req)
{
	int i;
	
	/* free_request should never get called by anything but process_request */
	if(req->buffer_end && req->status != DEAD)
	{
		req->status = DONE;
		return;
	}

	dequeue(list_head_addr, req);

	if(req->logline)
		;//log_access(req);

	if(req->data_fd)
		close(req->data_fd);
		
	if(req->post_data_fd)
		close(req->post_data_fd);
	
	if(req->response_status >= 400)
		;//status.errors++;

#if 0
	for(i = COMMON_CGI_COUNT; i < req->cgi_env_index; ++i)
	{


	}
#endif

	if(req->pathname)
		free(req->pathname);
	if(req->path_info)
		free(req->path_info);
	if(req->path_translated)
		free(req->path_translated);	
	if(req->script_name)
		free(req->script_name);
	
	//超时记录fd 放入阻塞队列
	if((req->keepalive == KA_ACTIVE) &&
		(req->response_status < 500) && req->kacount >0)
	{



	}

	if(req->method == M_POST)
	{
		char buf[32768];
		read(req->fd, buf, 32768);
	}
	
	close(req->fd);
	total_connections--;
	
	enqueue(&request_free, req);
	
	return;
}
