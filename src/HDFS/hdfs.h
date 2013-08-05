#ifndef INC_hdfs_h
#define INC_hdfs_h

#include <ross.h>

#define N_CLIENTS 4
#define N_NAMENODES 1
#define N_DATANODES 8
#define MAX_N_STRIPES 4

#define MEAN_REQUEST 30

typedef enum hdfs_event_t hdfs_event_t;

enum hdfs_event_t
{
  HDFS_WRITE_START,
  HDFS_WRITE_SET_UP,
  HDFS_WRITE_SET_UP_ACK,
  HDFS_WRITE_DATA_SEND,
  HDFS_WRITE_DATA_SEND_ACK,
  HDFS_WRITE_CLOSE,
  HDFS_WRITE_CLOSE_ACK
};

typedef struct
{
  int group_master;
  int logical_group_id;
  int rand_datanode_id[MAX_N_STRIPES];

}hdfs_state;

typedef struct
{
  int dst_lid;
  int dst_pid;
  int src_lid;
  int src_pid;
  hdfs_event_t type;
}msg_body;

typedef struct
{
  msg_body msg_core;
}hdfs_message;

static tw_lpid	 nlp_per_pe = 1024;
static int	 opt_mem = 1000;
static int	 planes_per_hdfs = 1;

#endif
