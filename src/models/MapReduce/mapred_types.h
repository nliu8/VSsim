#ifndef INC_mapred_type_h
#define INC_mapred_type_h

#include "mapred.h"

#define N_CLIENTS 3
#define N_NAMENODES 1
#define N_DATANODES 8
#define N_REPLICA 3

#define SERVER_JOB_REGISTRATION_PROCESSING_TIME 20

#define MAX_N_STRIPES 4
#define Write_Request_size 4 //MB
#define Pkt_size 4
#define Buffer_Copy_rate 0.4 
#define WRITE_SET_UP_PREP_TIME 10
#define CLOSE_TIME 700

#define PATH_DEPTH 8
#define MSG_SRC_NULL -1
#define MSG_DST_NULL -1

#define NN_process_time 20

#define MEAN_REQUEST 30

//mapred
#define BLOCK_SIZE 67108864 //64M

typedef enum mapred_event_t mapred_event_t;

enum mapred_event_t
{
  VS_MR_CLIENT_HEAD,

  VS_MR_CLIENT_SUBMIT_JOB,
  VS_MR_CLIENT_SUBMIT_JOB_SUCCESS,

  VS_MR_CLIENT_TAIL,
  
  VS_MR_CLIENT_WRITE_START,
  VS_MR_CLIENT_WRITE_SET_UP,
  VS_MR_CLIENT_WRITE_SET_UP_ACK,

  VS_MR_CLIENT_WRITE_SOCKET_SET_UP,
  VS_MR_CLIENT_WRITE_SOCKET_SET_UP_ACK,

  VS_MR_CLIENT_WRITE_DATA_SEND,
  VS_MR_CLIENT_WRITE_DATA_RECV,
  VS_MR_CLIENT_WRITE_DATA_SEND_ACK,
  VS_MR_CLIENT_WRITE_DONE,

  VS_MR_CLIENT_WRITE_CLOSE,
  VS_MR_CLIENT_WRITE_CLOSE_ACK,


  VS_MR_SERVER_HEAD,

  VS_MR_SERVER_JOB_REGISTRATION,

  VS_MR_SERVER_SUBMIT_JOB,
  VS_MR_SERVER_TAIL,
  
  VS_MR_SERVER_WRITE_START,
  VS_MR_SERVER_WRITE_SET_UP,
  VS_MR_SERVER_WRITE_SET_UP_ACK,

  VS_MR_SERVER_WRITE_SOCKET_SET_UP,
  VS_MR_SERVER_WRITE_SOCKET_SET_UP_ACK,

  VS_MR_SERVER_WRITE_DATA_SEND,
  VS_MR_SERVER_WRITE_DATA_RECV,
  VS_MR_SERVER_WRITE_DATA_SEND_ACK,
  VS_MR_SERVER_WRITE_DONE,

  VS_MR_SERVER_WRITE_CLOSE,
  VS_MR_SERVER_WRITE_CLOSE_ACK

};

typedef struct
{
  tw_lpid job_tracker_id;

  int group_master;
  int logical_group_id;
  int rand_datanode_id[MAX_N_STRIPES];

  int pkt_send_counter;
  int pkt_recv_counter;
  int data_node_ID[N_REPLICA];

  FILE * logfile;

  int namenode_id;
  double NN_timer;

}mr_client_state;

typedef struct
{
  int job_counter;// used for tracking number of jobs submitted, initialized to 0 upon initialization
  tw_lpid job_tracker_id;// used for recording job tracker id, could be arrays in future federations
  /////////////////

  int group_master;
  int logical_group_id;
  int rand_datanode_id[MAX_N_STRIPES];

  int pkt_send_counter;
  int pkt_recv_counter;
  int data_node_ID[N_REPLICA];

  FILE * logfile;

  int namenode_id;
  double NN_timer;

}mr_server_state;

typedef struct
{
  int dst_pid[PATH_DEPTH];
  int src_pid[PATH_DEPTH];

  mapred_event_t type;

  int replica_counter;

}msg_body;

typedef struct
{
  msg_body msg_core;
  int job_ID;

  long msg_size;

}mr_client_message;

typedef struct
{
  msg_body msg_core;

  int job_ID;


}mr_server_message;

static tw_lpid	 nlp_per_pe = 1024;
static int	 opt_mem = 1000;
static int	 planes_per_mapred = 1;

#endif
