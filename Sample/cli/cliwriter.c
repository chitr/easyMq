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
#include <Pmq.h>
#include <stdlib.h>
#include <mqueue.h>
#include <assert.h>
#define NB_MAX_WRITER_THREAD 10

extern MSGQ_info *mq_info;

void *status[NB_MAX_WRITER_THREAD];

int  main(int argc,char **argv) {


	struct rlimit msglimit ={RLIM_INFINITY ,RLIM_INFINITY };


	char *path=argv[1];
	struct mq_attr attr;
	unsigned long msg_size=atol(argv[2]);        
	unsigned long msg_count = atol(argv[3]);
	int  no_of_threads = 1/*atoi(argv[4])*/;
	int i=0;    
	int more_message=1;
	MSG_DATA msg;
	int msge_size=0;

	MSGQ_attr msg_attr;
	struct timeval start,end,diff;


    memset(&msg,0,sizeof(MSG_DATA));
	memset(&attr,0,sizeof(struct mq_attr));
	memset(&msg_attr,0,sizeof(MSGQ_attr));

    assert((msg.data=malloc(msg_size))>0);
	strncpy(msg_attr.path,path,MAX_PATH_LEN);


	/* First we need to set up the attribute structure */
	attr.mq_maxmsg = msg_count;
	attr.mq_msgsize = MSGSIZE;
	attr.mq_flags = 0;

	memcpy(&(msg_attr.msgq_attr),&attr,sizeof(struct mq_attr));
	if(messageq_init(&mq_info,no_of_threads,&msg_attr,WRITE)) {
		PRINT(LOG_CRITICAL,"message_init failed %lu",msg_count);
		exit(1);
	}
        
    while(more_message) {
		printf("Enter the messgae \n");
		scanf("%s",msg.data);
		msg.len=strlen(msg.data);
		printf("%s %d \n",msg.data,msg.len);
		getchar();
	    if(ERROR == send_msg(0,&msg,1)){
			PRINT(LOG_CRITICAL,"Unable to send message\n");
	    	}
		
    

	}   
	gettimeofday(&start,NULL);
	gettimeofday(&end,NULL);
	diff = time_diff(start,end);
	printf ("DIFF::%llu.%.6llu",(unsigned long long)diff.tv_sec, (unsigned long long)diff.tv_usec);

}


