#include "hdfs.h"

inline void prep_src(msg_body *msg)
{
  int i;
  for (i=0;i<PATH_DEPTH;i++)
    msg->src_pid[i]=MSG_SRC_NULL;
}

inline void pop_src(msg_body *msg, int * id)
{
  int i=0 ;
  if (msg->src_pid[0]==MSG_SRC_NULL)
    printf("No src ID found, nothing to pop, bye\n");
  else
    while(msg->src_pid[i] != MSG_SRC_NULL)
      i++;
  *id = msg->src_pid[i-1];
  msg->src_pid[i-1]=MSG_SRC_NULL;
}

inline void push_src(msg_body *msg, int * id)
{
  int i=0 ;
  if (msg->src_pid[PATH_DEPTH-1] != MSG_SRC_NULL)
    printf("Max stack size reached, please increase PATH_DEPTH\n");
  else
    while( msg->src_pid[i] != MSG_SRC_NULL )
      i++;
  msg->src_pid[i] = *id;
}

inline void show_src(msg_body *msg)
{
  int i;
  for (i=0;i<PATH_DEPTH;i++)
    printf("Src stack [%d] is %d\n",i,msg->src_pid[i]);
}
