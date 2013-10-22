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
    printf("\tNo src ID found, nothing to pop, bye\n");
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
    printf("\tMax stack size reached, please increase PATH_DEPTH\n");
  else
    while( msg->src_pid[i] != MSG_SRC_NULL )
      i++;
  msg->src_pid[i] = *id;
}

inline void show_src(msg_body *msg)
{
  int i;
  for (i=0;i<PATH_DEPTH;i++)
    printf("\tSrc stack [%d] is %d\n",i,msg->src_pid[i]);
}


inline void prep_dst(msg_body *msg)
{
  int i;
  for (i=0;i<PATH_DEPTH;i++)
    msg->dst_pid[i]=MSG_DST_NULL;
}

inline void pop_dst(msg_body *msg, int * id)
{
  int i=0 ;
  if (msg->dst_pid[0]==MSG_DST_NULL)
    printf("\tNo dst ID found, nothing to pop, bye\n");
  else
    {
      *id = msg->dst_pid[0];
      for (i=1;i<PATH_DEPTH;i++)
	{
	  msg->dst_pid[i-1] = msg->dst_pid[i];
	}
    }
}

inline void push_dst(msg_body *msg, int * id)
{
  int i=0 ;
  if (msg->dst_pid[PATH_DEPTH-1] != MSG_DST_NULL)
    printf("\tMax stack size reached, please increase PATH_DEPTH\n");
  else
    while( msg->dst_pid[i] != MSG_DST_NULL )
      i++;
  msg->dst_pid[i] = *id;
}

inline void show_dst(msg_body *msg)
{
  int i;
  for (i=0;i<PATH_DEPTH;i++)
    printf("\tDst stack [%d] is %d\n",i,msg->dst_pid[i]);
}
