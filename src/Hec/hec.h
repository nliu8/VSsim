#ifndef INC_hec_h
#define INC_hec_h

#include <ross.h>

#define MEAN_DEPARTURE 30.0
#define MEAN_LAND 10.0
#define GROUP_SIZE 4
#define ROUTING_DELAY 500
#define N_NODES 16
#define N_ROUTERS 4

typedef enum hec_event_t hec_event_t;

enum hec_event_t
{
	HEC_MSG_SEND, 
	HEC_MSG_RECV,
	HEC_MSG_ROUTE
};

typedef struct
{
  int group_master;
  int logical_group_id;

	int		landings;
	int		planes_in_the_sky;
	int		planes_on_the_ground;
	tw_stime	waiting_time;
	tw_stime	furthest_flight_landing;
}hec_state;

typedef struct
{
  int dst_lid;
  int dst_pid;
  int src_lid;
  int src_pid;
  hec_event_t type;
}msg_body;

typedef struct
{
  msg_body msg_core;
  tw_stime waiting_time;
  tw_stime saved_furthest_flight_landing;
}hec_message;

static tw_lpid	 nlp_per_pe = 1024;
static tw_stime	 mean_flight_time = 1;
static int	 opt_mem = 1000;
static int	 planes_per_hec = 1;
static tw_stime	 wait_time_avg = 0.0;

#endif
