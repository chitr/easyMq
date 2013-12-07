/*
 * vim:expandtab:shiftwidth=4:tabstop=4:
 *
 * Copyright   (2013)      Contributors
 * Contributor : chitr   chitr.prayatan@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * ---------------------------------------
 */
#include "easyMq_PosixMQ.h"

MSGQ_info *mq_info;
unsigned long no_of_msg = 0;
unsigned long msg_size  = 0;
fd_set read_fdset;


void  PRINT(int loglevel,char * print_format, ...) {
        char logbuff[LOG_SIZE];
        va_list arg;
        va_start(arg,print_format);
        vsprintf (logbuff,print_format, arg);
		if(loglevel > 0)
        printf("%s \n",logbuff);
        va_end(arg);

}


struct timeval time_diff(struct timeval time_from, struct timeval time_to) {
	struct timeval result;

	if(time_to.tv_usec < time_from.tv_usec)
	{
		result.tv_sec = time_to.tv_sec - time_from.tv_sec - 1;
		result.tv_usec = 1000000 + time_to.tv_usec - time_from.tv_usec;
	}
	else
	{
		result.tv_sec = time_to.tv_sec - time_from.tv_sec;
		result.tv_usec = time_to.tv_usec - time_from.tv_usec;
	}

	return result;
}

int    messageq_init(MSGQ_info **p_p_msgq_info,int no_of_msgq,MSGQ_attr *msgq_attr,op_type op) {
	
#ifdef MQ_LIMIT_INFINITY	

	struct rlimit msglimit ={RLIM_INFINITY ,RLIM_INFINITY };
	if(setrlimit(RLIMIT_MSGQUEUE ,&msglimit)) {
		PRINT(LOG_CRITICAL,"Unable to set limit %d",1);
		exit(1);
	}
#endif

	int q_count = 0 ;
	if(!(*p_p_msgq_info)  && (no_of_msgq > 0)) 
		*p_p_msgq_info = malloc(sizeof(MSGQ_info) * no_of_msgq);
	else
		return ERROR;
	memset((*p_p_msgq_info),0,sizeof(MSGQ_info)*no_of_msgq);


	if(READ==op)
		FD_ZERO(&read_fdset);

	for( q_count=0;q_count<no_of_msgq;q_count++) {
		char shared_mem_file[2*MAX_PATH_LEN];
		snprintf(shared_mem_file,2*MAX_PATH_LEN,"%sdontDeleteMQ_%d",msgq_attr->path,q_count);		
		((*p_p_msgq_info)[q_count]).mq_des = mq_open (shared_mem_file, O_RDWR | O_CREAT,S_IRUSR | S_IWUSR,&(msgq_attr->msgq_attr));


		PRINT(LOG_CRITICAL,"\n %d \n",errno);
		if(((*p_p_msgq_info)[q_count]).mq_des == (mqd_t) -1)
			perror("mq_open()");
		((*p_p_msgq_info)[q_count]).qindex=q_count;

		if(READ==op)
			FD_SET(((*p_p_msgq_info)[q_count]).mq_des,&read_fdset);
	}
	return SUCCESS;
}

int messageq_terminate(MSGQ_info **p_p_msgq_info,int no_of_msgq) {
	int q_count = 0;
	if(*p_p_msgq_info) {
		for( q_count=0;q_count<no_of_msgq;q_count++) {
			if((*p_p_msgq_info)[q_count].mq_des > 0 && mq_close((*p_p_msgq_info)[q_count].mq_des)) {
				PRINT(LOG_CRITICAL,"Unable to terminate msgq %d \n",q_count);
				return ERROR;
			}
		}
		free(*p_p_msgq_info);
		*p_p_msgq_info = NULL;
	}
	return SUCCESS;
}

int send_msg(int qid,MSG_DATA *p_msg,int priority) {
	if(p_msg && (priority >0)) {
		if (mq_send ((mq_info[qid]).mq_des, p_msg->data, p_msg->len, priority) == -1) {
			perror ("mq_send()");
			return ERROR;
		}
	}
	return SUCCESS;	
}

int  start_writer_threads(MSGQ_info **p_p_msgq_info,int no_of_msgq,unsigned long msgcount,unsigned long msgsize)
{

	int rc = 0;
	pthread_attr_t attr_thr;
	unsigned long q_count = 0;

	no_of_msg = msgcount;
	msg_size  = msgsize;

	/* Init for thread parameter (mostly for scheduling) */
	pthread_attr_init(&attr_thr);

	pthread_attr_setscope(&attr_thr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&attr_thr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setstacksize(&attr_thr, THREAD_STACK_SIZE);

	/* Starting all of the threads */
	for(q_count = 0; q_count < no_of_msgq; q_count++)
	{

		if((rc = pthread_create(&((*p_p_msgq_info)[q_count].mq_thread), &attr_thr, writer_thread,
						(void *)&((*p_p_msgq_info)[q_count].qindex))) != 0)
		{
			PRINT(LOG_CRITICAL,"Thread create error: %d", rc);
			return ERROR;
		}
		PRINT(LOG_CRITICAL,"Created queue: %d\n", q_count);
	}
	return SUCCESS;

}

void *writer_thread(void *write_data_arg) {
	int thread_index  = *((int *) write_data_arg);
	PRINT(LOG_CRITICAL,"\n %d ",thread_index);

	unsigned long prio = 0;
	char buf[MSGSIZE] = {0};
	for (prio = 0; prio < no_of_msg-1; prio += 1) {
		snprintf(buf,100,"msg no %lu of %d",prio,thread_index);
		PRINT(LOG_CRITICAL,"\nThread no : %d is writing a message with priority %lu .\n",thread_index,prio);
		if (mq_send ((mq_info[thread_index]).mq_des, buf, sizeof(buf), (prio % 100)) == -1)
			perror ("mq_send()");
		PRINT(LOG_CRITICAL,"write thread Prio %lu threadindex %d",prio,thread_index);
	}
   PRINT(LOG_CRITICAL,"Exiting thread %d:",thread_index);
}

void message_read(MSGQ_info ** msgq_info,int no_of_msgq) {
	char buf[MSGSIZE+1]={0};
	struct mq_attr attr, old_attr; 
	fd_set trans_fdset;
	struct timeval timeout;
	struct timeval start,end,diff;
	FD_ZERO(&trans_fdset);
	trans_fdset = read_fdset;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	int q_count=0;
	int ret=0;
	int i=0,prio;

	memset(&attr,0,sizeof(struct mq_attr));
	memset(&old_attr,0,sizeof(struct mq_attr));
	while(1) {


		gettimeofday(&start,NULL);

		if((ret = select((mq_info[no_of_msgq-1]).mq_des +1 ,&trans_fdset,NULL,NULL,NULL))>0) {
			for (i=0;i<no_of_msgq;i++) {
				PRINT(LOG_CRITICAL,"QUEUE No:%d\n",i);
				if(FD_ISSET(mq_info[i].mq_des,&trans_fdset)) {
								  PRINT(LOG_DEBUG,"QUEUE No:%d\n",i);
					mq_getattr (mq_info[i].mq_des, &attr);
									  printf ("%d messages are currently on the queue.\n", 
					attr.mq_curmsgs);

					if ( attr.mq_curmsgs != 0) {

						/*
						There are some messages on this queue.
						First set the queue to not block any calls    
						*/
						attr.mq_flags = O_NONBLOCK ;//MQ_NONBLOCK;
						mq_setattr (mq_info[i].mq_des, &attr, &old_attr);	  
						PRINT(LOG_CRITICAL,"\n %d   %d ",old_attr.mq_maxmsg,old_attr.mq_msgsize);
						
						while (mq_receive (mq_info[i].mq_des, &buf, MSGSIZE+1, &prio) != -1)  {
							MSG_DATA *msg =(MSG_DATA *)&buf;
							msg->data[msg->len]='\0';
							PRINT(LOG_CRITICAL,"Received a message with priority %d.\n", prio);
							PRINT(LOG_CRITICAL,"Message  %s.:length %d\n", msg->data,msg->len);
							memset(buf,0,sizeof(buf));

						}
					}
					if (errno != EAGAIN) { 
						perror ("mq_receive()");
						exit (-1);
					}

					/* Now restore the old attributes */
					FD_ZERO(&trans_fdset);
					mq_setattr (mq_info[i].mq_des, &old_attr, 0);			  
				}
			}
			trans_fdset = read_fdset;
		}

		gettimeofday(&end,NULL);
		diff = time_diff(start,end);
		printf ("DIFF::%llu.%.6llu",(unsigned long long)diff.tv_sec, (unsigned long long)diff.tv_usec);
	}

}
	





