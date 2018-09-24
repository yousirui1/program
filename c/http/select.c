#include "boa.h"

static struct timeval req_timeout;

fd_set block_read_fdset;  //读取文件描述符集合
fd_set block_write_fdset; //写文件描述符集合

int ka_max;
int ka_timeout;

void fdset_update();
void select_loop(int server_s)
{
	FD_ZERO(&block_read_fdset); //描述符集合清空
	FD_ZERO(&block_write_fdset); 
	
	/* set server_s and req_timeout */
	req_timeout.tv_sec = 60;     //等待i/o最长时间
	req_timeout.tv_usec = 0l;

	/* preset max_fd */
	max_fd = -1;
	
	while(1)
	{
		max_fd = -1;
		
		/* 判断是否从阻塞到就绪状态 */
		if(request_block)   
			fdset_update();

		
		/* 就绪转入阻塞队列 */	
		process_request(server_s);


		if(total_connections < (max_connections -1))
			BOA_FD_SET(server_s, &block_read_fdset);

		
		
		/* >0  -1 出错 0 超时   判断是读还是写就绪 */
		if(select(max_fd + 1, &block_read_fdset,
							&block_write_fdset, NULL,
							(request_ready || request_block ? &req_timeout : NULL)) == -1) //select 等待i/o最长时间为空一直等待
						//	&req_timeout) == -1)
		{
			if(errno == EINTR)
				continue;
			else if(errno != EBADF)
				;	
		}	
		
	
		time(&current_time);
		if(FD_ISSET(server_s, &block_read_fdset))  //判断server_s 是否有数据read
			pending_requests = 1; 

	}

}



/*
 *  遍历阻塞的请求，检查是否该文件描述符是由select设置的
 *  更新fd_set 的状态
 *
 */
void fdset_update(void)
{
	request *current, *next;
	
	/* 遍历阻塞请求 */
	for(current = request_block; current; current = next)
	{
		time_t time_since = current_time - current->time_last;  //计时
		next = current->next;
		
		/* 超时 */
		if(current->kacount < ka_max &&         //we are in a keepalive
			(time_since >= ka_timeout) && 		//timeout
			!current->logline)					//haven't read anything yet
			current->status = DEAD;
		
		else if(time_since > REQUEST_TIMEOUT)
		{
			//log_error_doc(current);
			fputs("connection timed out\n", stderr);
			current->status = DEAD;	
		}

		//数据准备ok 是否阻塞转就绪 
		if(current->buffer_end && current->status < DEAD)  
		{
			if(FD_ISSET(current->fd, &block_write_fdset))
				ready_request(current);         //阻塞转就绪
			else
				FD_SET(current->fd, &block_write_fdset);
		}
		else
		{
			switch(current->status)
			{
				case WRITE:
				case PIPE_WRITE:
					

					break;
				case BODY_WRITE:
				
					break;
				case PIPE_READ:

					break;
				case DONE:
				
					break;
				case DEAD:
					ready_request(current);	
					break;
				default:
					break;

			}

		}	

		current = next;
	}
}



