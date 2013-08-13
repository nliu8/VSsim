#include "hdfs.h"

tw_peid
mapping(tw_lpid gid)
{
  return (tw_peid) gid / g_tw_nlp;
}

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

  char filename[32];
  sprintf(filename,"log/vssim.log.%d",lp->gid);
  s->logfile = fopen (filename,"w");

  //printf("Init, my id is %d\n",lp->gid);
  // each client initiate a write request

  s->pkt_send_counter = 0;
  s->pkt_recv_counter = 0;
  s->data_node_ID = tw_rand_integer(lp->rng, 1, N_DATANODES) + N_CLIENTS + N_NAMENODES - 1;


  //printf("I am node %d and my associated data node ID is %d\n",lp->gid,s->data_node_ID);

  if (lp->gid < N_CLIENTS)
    {
      e = tw_event_new(lp->gid, tw_rand_exponential(lp->rng, MEAN_REQUEST), lp);
      m = tw_event_data(e);
      m->msg_core.type = HDFS_WRITE_START;
 
      //m->msg_core.src_pid[0] = lp->gid;
      prep_src( &m->msg_core);
      push_src( &m->msg_core, &lp->gid);
      //show_src( &m->msg_core);
      // name node comes right after clients
      // name node Pid is N_CLIENTS
      m->msg_core.dst_pid = N_CLIENTS;
	
      tw_event_send(e);
    }
}

void
event_logging_f(hdfs_state *s, hdfs_message * msg, tw_lp * lp)
{
   
  switch(msg->msg_core.type)
    {
    case HDFS_WRITE_START:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_START","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_SET_UP:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_SET_UP","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_SET_UP_ACK:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_SET_UP_ACK","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_DATA_SEND:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_DATA_SEND","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_DATA_RECV:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_DATA_RECV","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_DATA_SEND_ACK:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_DATA_SEND_ACK","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_DONE:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_DONE","Vtime: ",tw_now(lp));
	break;
      }
    default:
      fprintf(s->logfile,"\t%-4s","Unknown event type, please check! ... ... ... ...\n");
    }

}

void
event_logging(hdfs_state *s, hdfs_message * msg, tw_lp * lp)
{

  switch(msg->msg_core.type)
    {
    case HDFS_WRITE_START:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_START","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_SET_UP:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_SET_UP","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_SET_UP_ACK:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_SET_UP_ACK","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_DATA_SEND:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_DATA_SEND","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_DATA_RECV:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_DATA_RECV","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_DATA_SEND_ACK:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_DATA_SEND_ACK","Vtime: ",tw_now(lp));
	break;
      }
    case HDFS_WRITE_DONE:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","HDFS_WRITE_DONE","Vtime: ",tw_now(lp));
	break;
      }
    default:
      printf("\t%-4s","Unknown event type, please check! ... ... ... ...\n");
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

  event_logging(s, msg, lp);
  event_logging_f(s, msg, lp);

  switch(msg->msg_core.type)
    {
      
    case HDFS_WRITE_START:
      {

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

	    e = tw_event_new(msg->msg_core.src_pid[0], 10, lp);
	    m = tw_event_data(e);
	    m->msg_core = msg->msg_core;

	    m->msg_core.src_pid[0] = lp->gid;
	    m->msg_core.type = HDFS_WRITE_SET_UP_ACK;
	    tw_event_send(e);	    
	  }
	else
	  printf("Message is not at the right router, Please check!\n");

	break;
      }

    case HDFS_WRITE_SET_UP_ACK:
      {

	e = tw_event_new(lp->gid, tw_rand_exponential(lp->rng, WRITE_SET_UP_PREP_TIME), lp);
	m = tw_event_data(e);
	m->msg_core = msg->msg_core;

	m->msg_core.type = HDFS_WRITE_DATA_SEND;
	tw_event_send(e);

	break;
      }

    case HDFS_WRITE_DATA_SEND:
      {

	if (s->pkt_send_counter < Write_Request_size/Pkt_size)
	  {
	    // split request to packet
	    s->pkt_send_counter++;
	    e = tw_event_new(lp->gid, Pkt_size/Buffer_Copy_rate, lp);
	    m = tw_event_data(e);
	    m->msg_core = msg->msg_core;	    

	    m->msg_core.type = HDFS_WRITE_DATA_SEND;
	    tw_event_send(e);

	    // each packet corresponds to a real send
	    // pick random data node
	    e = tw_event_new(s->data_node_ID, Pkt_size/Buffer_Copy_rate, lp);
            m = tw_event_data(e);
            m->msg_core = msg->msg_core;

            m->msg_core.type = HDFS_WRITE_DATA_SEND_ACK;
	    m->msg_core.src_pid[0] = lp->gid;
            tw_event_send(e);

	  }

	break;
      }

    case HDFS_WRITE_DATA_SEND_ACK:
      {

	// split request to packet
	e = tw_event_new(msg->msg_core.src_pid[0], Pkt_size/Buffer_Copy_rate, lp);
	m = tw_event_data(e);
	m->msg_core = msg->msg_core;

	m->msg_core.type = HDFS_WRITE_DONE;
	tw_event_send(e);

	break;
      }

    case HDFS_WRITE_DONE:
      {
	s->pkt_recv_counter++;
	if ( s->pkt_recv_counter == Write_Request_size/Pkt_size )
	  {
	    printf("Write finished at %d\n",lp->gid);
	    /* // split request to packet */
	    /* e = tw_event_new(msg_core.src_pid[0], Pkt_size/Buffer_Copy_rate, lp); */
	    /* m = tw_event_data(e); */

	    /* m->msg_core = msg->msg_core; */
	    /* m->msg_core.type = HDFS_WRITE_DONE; */
	    /* tw_event_send(e); */
	  }
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
  fclose(s->logfile);
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
