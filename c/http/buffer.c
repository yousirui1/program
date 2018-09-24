#include "boa.h"



/*********************************************
 *
 * 将任何备份缓冲区发送给客户端
 * return -2 for error -1 for blocked 
 ********************************************/

int req_flush(request *req)
{
	int bytes_to_write;
	
	bytes_to_write = req->buffer_end - req->buffer_start;
	if(req->status == DEAD)
		return -2;
	
	if(bytes_to_write)
	{
		int bytes_written;
	
		bytes_written = write(req->fd, req->buffer + req->buffer_start,
								bytes_to_write);
		if(bytes_written < 0)
		{
			if(errno == EWOULDBLOCK || errno == EAGAIN)
				return -1;
			else
			{
				req->buffer_start = req->buffer_end = 0;
				if(errno != EPIPE)
					perror("buffer flush");
				req->status = DEAD;
				req->buffer_end = 0;
				return -2;
			}
		}	
	}

	if(req->buffer_start == req->buffer_end)
		req->buffer_start = req->buffer_end  = 0;
	return req->buffer_end;          //successful
}
