#include "boa.h"

request *request_ready = NULL;    //就绪 list head
request *request_block = NULL;    //阻塞 list head
request *request_free = NULL;   //释放 list head


void enqueue(request **head, request *req);
void dequeue(request **head, request *req);
/************************************
 *  
 *
 *  从就绪队列里移动一个request到阻塞队列
 ***************************************/
void block_request(request *req)
{
	dequeue(&request_ready, req);
	enqueue(&request_block, req);
	
	if(req->buffer_end)
	{
	
	}
	else
	{



	}
}





/************************************
 *  
 *
 *  从阻塞队列里移动一个request到就绪队列
 ***************************************/
void ready_request(request *req)
{
	dequeue(&request_block, req);
	enqueue(&request_ready, req);
	
	if(req->buffer_end)
	{
	
	}
	else
	{



	}

}



/************************************
 *  
 *
 * 出队列
 ***************************************/
void dequeue(request **head, request *req)
{
	if(*head == req)
		*head = req->next;
	
	if(req->prev)
		req->prev->next = req->next;
	
	if(req->next)
		req->next->prev = req->prev;
	
	req->next = NULL;
	req->prev = NULL;

}


/************************************
 *  
 *
 * 入队列
 ***************************************/
void enqueue(request **head, request *req)
{
	if(*head)
		(*head)->prev = req;  //previous head's prev is us
	
	req->next = *head;    //our next is previous head
	req->prev=  NULL;     //first in list
	
	*head = req; 	      //now we are head
}

