#include "hdfs.h"

tw_peid
mapping(tw_lpid gid)
{
  return (tw_peid) gid / g_tw_nlp;
}

void
init(hdfs_state * s, tw_lp * lp)
{
  int i;
  tw_event *e;
  hdfs_message *m;

  /*
    N_CLIENTS = 4
    N_NAMENODES = 1
    N_DATANODES =8
    0 1 2 3: clients
    4      : name nodes
    5 6 7 8 9 10 11 12 : data nodes
   */

  //printf("Init, my id is %d\n",lp->gid);

  // each client initiate a write request

  if (lp->gid < N_CLIENTS)
    {
      e = tw_event_new(lp->gid, tw_rand_exponential(lp->rng, MEAN_REQUEST), lp);
      m = tw_event_data(e);
      m->msg_core.type = HDFS_WRITE_START;
	
      m->msg_core.src_pid = lp->gid;
      // name node comes right after clients
      m->msg_core.dst_pid = N_CLIENTS;
	
      tw_event_send(e);

    }
}

void
event_handler(hdfs_state * s, tw_bf * bf, hdfs_message * msg, tw_lp * lp)
{
  int rand_result;
  tw_lpid dest_lp;
  tw_stime ts;
  tw_event *e;
  hdfs_message *m;

  switch(msg->msg_core.type)
    {
    case HDFS_WRITE_START:
      {
	printf("Message %d arrive at send\n", msg->msg_core.src_pid);
	e = tw_event_new(msg->msg_core.dst_pid, 10, lp);
	m = tw_event_data(e);
	m->msg_core = msg->msg_core;
	m->msg_core.type = HDFS_WRITE_SET_UP;
	tw_event_send(e);
	break;
      }

    case HDFS_WRITE_SET_UP:
      {
	// name node gid is after client gid
	// This message is received at Name Node
	if (lp->gid == N_CLIENTS)
	  {
	    printf("Hey I am message %d  and my dest is %d\n",msg->msg_core.src_pid,lp->gid);
	    e = tw_event_new(msg->msg_core.src_pid, 10, lp);
	    m = tw_event_data(e);
	    m->msg_core = msg->msg_core;

	    m->msg_core.src_pid = lp->gid;
	    m->msg_core.type = HDFS_WRITE_SET_UP_ACK;
	    tw_event_send(e);
	    
	  }
	else
	  printf("Message is not at the right router, Please chdfsk!\n");

	break;
      }

    case HDFS_WRITE_SET_UP_ACK:
      {
	printf("At %d one message RECV from %d\n",
	       lp->gid,
	       msg->msg_core.src_lid);
	/* s->landings++; */
	/* s->waiting_time += msg->waiting_time; */

	/* e = tw_event_new(lp->gid, tw_rand_exponential(lp->rng, MEAN_DEPARTURE), lp); */
	/* m = tw_event_data(e); */
	/* m->type = DEPARTURE; */
	/* tw_event_send(e); */
	break;
      }

    }
}

void
rc_event_handler(hdfs_state * s, tw_bf * bf, hdfs_message * msg, tw_lp * lp)
{
  switch(msg->msg_core.type)
  {
    /* case ARRIVAL: */
    /* 	s->furthest_flight_landing = msg->saved_furthest_flight_landing; */
    /* 	tw_rand_reverse_unif(lp->rng); */
    /* 	break; */
    /* case DEPARTURE: */
    /* 	tw_rand_reverse_unif(lp->rng); */
    /* 	tw_rand_reverse_unif(lp->rng); */
    /* 	break; */
    /* case LAND: */
    /* 	s->landings--; */
    /* 	s->waiting_time -= msg->waiting_time; */
    /* 	tw_rand_reverse_unif(lp->rng); */
  }
}

void
final(hdfs_state * s, tw_lp * lp)
{
  //wait_time_avg += ((s->waiting_time / (double) s->landings) / nlp_per_pe);
}

tw_lptype hdfs_lps[] =
{
  {
    (init_f) init,
    (event_f) event_handler,
    (revent_f) rc_event_handler,
    (final_f) final,
    (map_f) mapping,
    sizeof(hdfs_state),
  },
  {0},
};

const tw_optdef app_opt [] =
{
  TWOPT_GROUP("Hdfs Model"),
  TWOPT_UINT("nplanes", planes_per_hdfs, "initial # of planes per hdfs(events)"),
  //TWOPT_STIME("mean", mean_flight_time, "mean flight time for planes"),
  TWOPT_UINT("memory", opt_mem, "optimistic memory"),
  TWOPT_END()
};

int main(int argc, char **argv, char **env)
{
  int i;
  
  tw_opt_add(app_opt);
  tw_init(&argc, &argv);
  
  nlp_per_pe = (N_CLIENTS+N_NAMENODES+N_DATANODES)/(tw_nnodes() * g_tw_npe);
  g_tw_events_per_pe =(planes_per_hdfs * nlp_per_pe / g_tw_npe) + opt_mem;
  
  tw_define_lps(nlp_per_pe, sizeof(hdfs_message), 0);
  
  for(i = 0; i < g_tw_nlp; i++)
    tw_lp_settype(i, &hdfs_lps[0]);

  tw_run();

  if(tw_ismaster())
    {
      printf("\nHdfs Model Statistics:\n");
      //printf("\t%-50s %11.4lf\n", "Average Waiting Time", wait_time_avg);
      printf("\t%-50s %11lld\n", "Number of hdfss", 
	     nlp_per_pe * g_tw_npe * tw_nnodes());
      printf("\t%-50s %11lld\n", "Number of planes", 
	     planes_per_hdfs * nlp_per_pe * g_tw_npe * tw_nnodes());
    }

  tw_end();
	
  return 0;
}
