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
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>


#define MAX_PATH_LEN 255
#define ERROR    -1
#define SUCCESS   0
#define THREAD_STACK_SIZE  21164888
#define MSGSIZE 180
#define LOGFILE ""
#define MQ_LIMIT_INFINITY 1
#define LOG_SIZE 256

void  PRINT(int loglevel,char * print_format, ...) __attribute__((format(printf,2,3))) ;


#define ON  1
#define OFF 0

typedef struct wrapper 
{
char *buff
}WRAPPER;
typedef struct  __attribute__ ((__packed__)) msg_data
{	
	int   len;
	char *data;
}MSG_DATA;

typedef struct msgq_info 
{   int       qindex;
	pthread_t mq_thread;
	mqd_t     mq_des;    
}MSGQ_info;

typedef struct msgq_attr 
{
	char   path[MAX_PATH_LEN];
	struct mq_attr msgq_attr ;	
}MSGQ_attr;

typedef struct msg_spec 
{
unsigned long q_index;
unsigned long msg_count;
unsigned long msg_buff_size;
}MSG_spec;

typedef enum OP 
{
READ=0,
WRITE
} op_type;

typedef enum log_level 
{
LOG_DEBUG=0,
LOG_CRITICAL	
}LOG_LEVEL;

struct timeval time_diff(struct timeval time_from, struct timeval time_to);
int    send_msg(int mqdes,MSG_DATA *p_msg,int priority);
int    messageq_init(MSGQ_info **msgq_info,int no_of_msgq,MSGQ_attr *msgq_attr,op_type op);
int    messageq_terminate(MSGQ_info **msgq_info,int no_of_msgq);
int    start_writer_threads(MSGQ_info **msgq_info,int no_of_msgq,unsigned long msgcount,unsigned long msgsize);
void *writer_thread(void *write_data_arg);
void message_read(MSGQ_info **msgq_info,int no_of_msgq);




