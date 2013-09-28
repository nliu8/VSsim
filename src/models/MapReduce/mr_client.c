#include "mapred.h"

tw_peid
mr_client_mapping(tw_lpid gid)
{
  return (tw_peid) gid / g_tw_nlp;
}

void
mr_client_init(mr_client_state * s, tw_lp * lp)
{
  int i,j,flag;
  tw_event *e;
  mr_client_message *m;

  /*
    only one  Job Tracker: PID 0
    the rest are Physical nodes
   */

  char filename[32];

  s->job_tracker_id = 0;

  s->pkt_send_counter = 0;
  s->pkt_recv_counter = 0;

  printf("mr_client Success\n");

  // client 1 has a job to submit
  if (lp->gid == 1)
    {
      e = tw_event_new(lp->gid, tw_rand_exponential(lp->rng, 10), lp);
      m = tw_event_data(e);
      m->msg_core.type = VS_MR_CLIENT_SUBMIT_JOB;
 
      prep_src( &m->msg_core);
      prep_dst( &m->msg_core);

      push_src( &m->msg_core, &lp->gid);
      //show_src( &m->msg_core);
      push_dst( &m->msg_core, &s->job_tracker_id);

      // prepare request size
      // 2 blocks, temp
      m->msg_size = 128*1024*1024;

      tw_event_send(e);
    }
}

void
mr_client_event_handler(mr_client_state * s, tw_bf * bf, mr_client_message * msg, tw_lp * lp)
{
  int rand_result, i;
  tw_lpid dest_lp;
  tw_stime ts;
  tw_event *e;
  mr_client_message *m;

  //event_logging(s, msg, lp);
  //event_logging_f(s, msg, lp);

  switch(msg->msg_core.type)
    {
      
    case VS_MR_CLIENT_SUBMIT_JOB:
      {
	printf("Job submitted at 1 \n");
	pop_dst(&msg->msg_core,&dest_lp);
	e = tw_event_new(dest_lp, 10, lp);
	m = tw_event_data(e);
	m->msg_core = msg->msg_core;
	m->msg_core.type = VS_MR_CLIENT_WRITE_SET_UP;
	//tw_event_send(e);
	break;
      }

    case VS_MR_CLIENT_WRITE_START:
      {
	pop_dst(&msg->msg_core,&dest_lp);
	e = tw_event_new(dest_lp, 10, lp);
	m = tw_event_data(e);
	m->msg_core = msg->msg_core;
	m->msg_core.type = VS_MR_CLIENT_WRITE_SET_UP;
	tw_event_send(e);
	break;
      }

    case VS_MR_CLIENT_WRITE_SET_UP:
      {
	// name node gid is after client gid
	// This message is received at Name Node
	if (lp->gid == s->namenode_id)
	  {
	    s->NN_timer = max(s->NN_timer, tw_now(lp));
	    s->NN_timer += NN_process_time;

	    e = tw_event_new(msg->msg_core.src_pid[0], s->NN_timer, lp);
	    m = tw_event_data(e);
	    m->msg_core = msg->msg_core;

	    m->msg_core.src_pid[0] = lp->gid;
	    m->msg_core.type = VS_MR_CLIENT_WRITE_SET_UP_ACK;
	    tw_event_send(e);	    
	  }
	else
	  printf("\tMessage is not at name node, Please check!\n");

	break;
      }

    case VS_MR_CLIENT_WRITE_SET_UP_ACK:
      {

	e = tw_event_new(lp->gid, tw_rand_exponential(lp->rng, WRITE_SET_UP_PREP_TIME), lp);
	m = tw_event_data(e);
	m->msg_core = msg->msg_core;

	//m->msg_core.type = VS_MR_CLIENT_WRITE_DATA_SEND;
	m->msg_core.type = VS_MR_CLIENT_WRITE_SOCKET_SET_UP;
	//pack dst data nodes ID to packet
	prep_src(&m->msg_core);
	prep_dst(&m->msg_core);
	//show_dst(&m->msg_core);
	for (i=0;i<N_REPLICA;i++)
	  push_dst(&m->msg_core,&s->data_node_ID[i]);
	show_dst(&m->msg_core);
	//pop_dst(&m->msg_core,&dest_lp);
	//m->msg_core.replica_counter++;

	tw_event_send(e);
	break;
      }

    case VS_MR_CLIENT_WRITE_SOCKET_SET_UP:
      {
	// if message still hasn't reached the deepest point in the path
	// printf("msg->msg_core.replica_counter is %d\n",msg->msg_core.replica_counter);
	if (msg->msg_core.replica_counter<N_REPLICA)
	  {
	    pop_dst(&(msg->msg_core),&dest_lp);
	    msg->msg_core.replica_counter++;
	    push_src(&(msg->msg_core),&lp->gid);

	    e = tw_event_new(dest_lp, tw_rand_exponential(lp->rng, WRITE_SET_UP_PREP_TIME), lp);
	    m = tw_event_data(e);
	    m->msg_core = msg->msg_core;
	    m->msg_core.type = VS_MR_CLIENT_WRITE_SOCKET_SET_UP;
	    tw_event_send(e);
	    //printf("Appear in LP %d\n",lp->gid);
	  }
	else
	  {
	    msg->msg_core.replica_counter--;
	    pop_src(&(msg->msg_core),&dest_lp);
	    e = tw_event_new(dest_lp, tw_rand_exponential(lp->rng, WRITE_SET_UP_PREP_TIME), lp);
	    m = tw_event_data(e);
	    m->msg_core = msg->msg_core;
	    m->msg_core.type = VS_MR_CLIENT_WRITE_SOCKET_SET_UP_ACK;
	    tw_event_send(e);
	  }
	break;
      }

    case VS_MR_CLIENT_WRITE_SOCKET_SET_UP_ACK:
      {
	if (msg->msg_core.replica_counter==0)
	  {
	    e = tw_event_new(lp->gid, tw_rand_exponential(lp->rng, WRITE_SET_UP_PREP_TIME), lp);
	    m = tw_event_data(e);
	    m->msg_core = msg->msg_core;
	    m->msg_core.type = VS_MR_CLIENT_WRITE_DATA_SEND;
	    tw_event_send(e);
	    //printf("LP %d Appear\n",lp->gid);

	  }
	else
	  {
	    msg->msg_core.replica_counter--;
	    pop_src(&(msg->msg_core),&dest_lp);
	    e = tw_event_new(dest_lp, tw_rand_exponential(lp->rng, WRITE_SET_UP_PREP_TIME), lp);
	    m = tw_event_data(e);
	    m->msg_core = msg->msg_core;
	    m->msg_core.type = VS_MR_CLIENT_WRITE_SOCKET_SET_UP_ACK;
	    tw_event_send(e);

	  }
	break;
      }

    case VS_MR_CLIENT_WRITE_DATA_SEND:
      {

	if (s->pkt_send_counter < Write_Request_size/Pkt_size)
	  {
	    // split request to packet
	    s->pkt_send_counter++;
	    e = tw_event_new(lp->gid, Pkt_size/Buffer_Copy_rate, lp);
	    m = tw_event_data(e);
	    m->msg_core = msg->msg_core;	    

	    m->msg_core.type = VS_MR_CLIENT_WRITE_DATA_SEND;
	    tw_event_send(e);

	    // each packet corresponds to a real send
	    // pick random data node
	    e = tw_event_new(s->data_node_ID[0], Pkt_size/Buffer_Copy_rate, lp);
            m = tw_event_data(e);
            m->msg_core = msg->msg_core;

            m->msg_core.type = VS_MR_CLIENT_WRITE_DATA_SEND_ACK;
	    m->msg_core.src_pid[0] = lp->gid;
            tw_event_send(e);
	  }

	break;
      }

    case VS_MR_CLIENT_WRITE_DATA_SEND_ACK:
      {
	// split request to packet
	e = tw_event_new(msg->msg_core.src_pid[0], Pkt_size/Buffer_Copy_rate, lp);
	m = tw_event_data(e);
	m->msg_core = msg->msg_core;

	m->msg_core.type = VS_MR_CLIENT_WRITE_DONE;
	tw_event_send(e);

	break;
      }

    case VS_MR_CLIENT_WRITE_DONE:
      {
	s->pkt_recv_counter++;
	if ( s->pkt_recv_counter == Write_Request_size/Pkt_size )
	  {	    
	    // send success msg to NN
	    e = tw_event_new(s->namenode_id, CLOSE_TIME, lp);
	    m = tw_event_data(e);

	    m->msg_core = msg->msg_core;
	    push_src(&(m->msg_core),&lp->gid);
	    m->msg_core.type = VS_MR_CLIENT_WRITE_CLOSE;
	    tw_event_send(e);
	  }
	else
	  printf("\tMessage is not at name node, Please check!\n");
	break;
      }

    case VS_MR_CLIENT_WRITE_CLOSE:
      {	    
	//printf("Close finished at %d\n",lp->gid);
	// This message is received at Name Node
	if (lp->gid == s->namenode_id)
	  {
	    pop_src(&(msg->msg_core),&dest_lp);
	    e = tw_event_new(dest_lp, CLOSE_TIME, lp);
	    m = tw_event_data(e);

	    m->msg_core = msg->msg_core;
	    m->msg_core.type = VS_MR_CLIENT_WRITE_CLOSE_ACK;
	    tw_event_send(e);
	  }
	break;
      }

    case VS_MR_CLIENT_WRITE_CLOSE_ACK:
      {	    
	printf("Close finished at %d\n",lp->gid);
	/* e = tw_event_new(msg_core.src_pid[0], CLOSE_TIME, lp); */
	/* m = tw_event_data(e); */

	/* m->msg_core = msg->msg_core; */
	//m->msg_core.type = VS_MR_CLIENT_WRITE_CLOSE_ACK;
	//tw_event_send(e);
	break;
      }

    }
}

void
mr_client_rc_event_handler(mr_client_state * s, tw_bf * bf, mr_client_message * msg, tw_lp * lp)
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
mr_client_final(mr_client_state * s, tw_lp * lp)
{
  //fclose(s->logfile);
}

