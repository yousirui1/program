#include "boa.h"

/******************************************
 *
 *
 * return 
		-1
		 0 
		 1
 *****************************************/
int read_header(request *req)
{
	int bytes, buf_bytes_left;
	char *check, *buffer;

	check = req->client_stream + req->parse_pos;  //解析偏移量
	buffer = req->client_stream;
	bytes = req->client_stream_pos;


	while(check < (buffer + bytes))
	{
		switch(req->status)
		{
				case READ_HEADER:    				//判断是一行结束\r\n 还是报文头结束\r\n\r\n
				if(*check == '\r')
				{
					req->status = ONE_CR;
					req->header_end = check;
				}
				else if(*check == '\n')
				{
					req->status = ONE_LF;
					req->header_end = check;
				}
				break;
			
			case ONE_CR:
				if(*check == '\n')
					req->status = ONE_LF;
				else if(*check != '\r')
					req->status = READ_HEADER;
				break;
			
			case ONE_LF:
				if(*check == '\r')
					req->status == TWO_CR;
				else if(*check == '\n')
					req->status = BODY_READ;
				else
					req->status = READ_HEADER;
				break;
	
			case TWO_CR:
				if(*check == '\n')
					req->status = BODY_READ;
				else if(*check != '\r')
					req->status = READ_HEADER;
				break;
			
			default:
				break;
		}

		req->parse_pos++; 					//update parse position
		check++;
	
		if(req->status == ONE_LF)		//\r\n 第一行结束
		{
			*req->header_end = '\0';
		
			if(req->logline)
			{
				//if(process_option_line(req) == 0)
					return 0;
			}	
			else
			{
				if(process_logline(req) == 0)
					return 0;
				if(req->simple)
					;
					//return process_header_end(req);
			}
	
		}
		else if(req->status == BODY_READ)
		{
			int retval;
			if(retval && req->method == M_POST)
			{
				if(req->content_length)
				{

				}
				else
				{
					return 0;
				}

			}
			return retval;
		}
		

	}
		
		if(req->status < BODY_READ)
		{
			/* only reached if request is split across more than one packet */
			
			buf_bytes_left = CLIENT_STREAM_SIZE - req->client_stream_pos;
			if(buf_bytes_left < 1)
			{
				//log_error_time();
				fputs("buffer overrun - read.c read_header - closing\n", stderr);
				req->status = DEAD;
				return 0;
			}
			
			bytes = read(req->fd, buffer + req->client_stream_pos, buf_bytes_left);
			printf("client_stream %s\r\n", buffer);	
			
			if(bytes < 0)
			{
				if(errno == EINTR)
					return 1;
				if(errno == EAGAIN || errno == EWOULDBLOCK)		//request blocked
					return -1;
					
				//log_error_doc(req);
				perror("header read");
				return 0;
	
			}
			else if(bytes == 0)
			{
				return 0;
			}
			req->client_stream_pos += bytes;
			return 1;
		}
	return 1;
}


int read_body(request *req)
{
	printf("read_body start \r\n");
	int bytes_read, bytes_to_read, bytes_free;
	
	bytes_free = BUFFER_SIZE - (req->header_end - req->header_line);
	bytes_to_read = req->filesize - req->filepos;
	
	if(bytes_to_read > bytes_free)
		bytes_to_read = bytes_free;

	if(bytes_to_read <= 0)
	{
		req->status = BODY_WRITE;	//go write it
		return 1;
	}
	
	bytes_read = read(req->fd, req->header_end, bytes_to_read);

	if(bytes_read == -1)
	{
		if(errno == EWOULDBLOCK || errno == EAGAIN)
		{
			return -1;
		}
		else
		{
			printf( "read body err\r\n");
			return 0;	
		}
	}	
	else if(bytes_read == 0)
	{
		/* this is an error, premature end of body !*/
		//log_error_time();
	    fprintf(stderr, "%s:%d - Premature end of body!!\n",
                __FILE__, __LINE__);
       // send_r_bad_request(req);
		return 0;
	}
	req->status = BODY_WRITE;

	req->header_end += bytes_read;
		
	return 1;
}
