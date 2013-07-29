#include "hec.h"

tw_peid
mapping(tw_lpid gid)
{
	return (tw_peid) gid / g_tw_nlp;
}

void
init(hec_state * s, tw_lp * lp)
{
  int i;
  tw_event *e;
  hec_message *m;

  /*
    N_routers = 4
    N_nodes = 15
    0 1 2 3: routers (group master)
    4 5 6 7     -> 0
    8 9 10 11   -> 1
    12 13 14 15 -> 2
    16 17 18    -> 3
   */
  s->group_master = (lp->gid-N_ROUTERS)/GROUP_SIZE;

  s->landings = 0;
  s->planes_in_the_sky = 0;
  s->planes_on_the_ground = planes_per_hec;
  s->waiting_time = 0.0;
  s->furthest_flight_landing = 0.0;

  //printf("Init, my id is %d\n",lp->gid);

  if (lp->gid >= N_ROUTERS)
    {
      //printf("In init loop, my id is %d\n",lp->gid);
      for(i = 0; i < planes_per_hec; i++)
	{
	  e = tw_event_new(lp->gid, tw_rand_exponential(lp->rng, MEAN_DEPARTURE), lp);
	  m = tw_event_data(e);
	  m->msg_core.type = HEC_MSG_SEND;
	
	  m->msg_core.src_pid = lp->gid;
	  m->msg_core.src_lid = lp->gid - N_ROUTERS;
	  m->msg_core.dst_lid = (i + N_ROUTERS) % N_NODES;
	  m->msg_core.dst_pid = ( i + N_ROUTERS + lp->gid ) % N_NODES + N_ROUTERS;
	
	  tw_event_send(e);
      }
    }
}

void
event_handler(hec_state * s, tw_bf * bf, hec_message * msg, tw_lp * lp)
{
  int rand_result;
  tw_lpid dest_lp;
  tw_stime ts;
  tw_event *e;
  hec_message *m;

  switch(msg->msg_core.type)
    {
    case HEC_MSG_SEND:
      {
	//printf("Message %d arrive at send\n", msg->msg_core.src_pid);
	e = tw_event_new(s->group_master, 10, lp);
	m = tw_event_data(e);
	m->msg_core = msg->msg_core;
	m->msg_core.type = HEC_MSG_ROUTE;
	m->waiting_time = s->furthest_flight_landing - tw_now(lp);
	s->furthest_flight_landing += ts;
	tw_event_send(e);
	break;
      }

    case HEC_MSG_ROUTE:
      {
	if (lp->gid < N_ROUTERS)
	  {
	    printf("Hey I am message %d in master %d and my dest is %d\n",msg->msg_core.src_lid,lp->gid,msg->msg_core.dst_pid);
	    // translate lid to pid
	    // dest_lp = m->msg_core.dest_lid + N_ROUTERS;
	    e = tw_event_new(msg->msg_core.dst_pid, 10, lp);
	    m = tw_event_data(e);
	    m->msg_core = msg->msg_core;
	    m->msg_core.type = HEC_MSG_RECV;
	    tw_event_send(e);
	    
	  }
	else
	  printf("Message is not at the right router, Please check!\n");

	break;
      }

    case HEC_MSG_RECV:
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
rc_event_handler(hec_state * s, tw_bf * bf, hec_message * msg, tw_lp * lp)
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
final(hec_state * s, tw_lp * lp)
{
	wait_time_avg += ((s->waiting_time / (double) s->landings) / nlp_per_pe);
}

tw_lptype hec_lps[] =
{
	{
		(init_f) init,
		(event_f) event_handler,
		(revent_f) rc_event_handler,
		(final_f) final,
		(map_f) mapping,
		sizeof(hec_state),
	},
	{0},
};

const tw_optdef app_opt [] =
{
	TWOPT_GROUP("Hec Model"),
	//TWOPT_UINT("nhecs", nlp_per_pe, "initial # of hecs(LPs)"),
	TWOPT_UINT("nplanes", planes_per_hec, "initial # of planes per hec(events)"),
	TWOPT_STIME("mean", mean_flight_time, "mean flight time for planes"),
	TWOPT_UINT("memory", opt_mem, "optimistic memory"),
	TWOPT_END()
};

int
main(int argc, char **argv, char **env)
{
	int i;

	tw_opt_add(app_opt);
	tw_init(&argc, &argv);

	nlp_per_pe = (N_NODES+N_ROUTERS)/(tw_nnodes() * g_tw_npe);
	g_tw_events_per_pe =(planes_per_hec * nlp_per_pe / g_tw_npe) + opt_mem;

	tw_define_lps(nlp_per_pe, sizeof(hec_message), 0);

	for(i = 0; i < g_tw_nlp; i++)
		tw_lp_settype(i, &hec_lps[0]);

	tw_run();

	if(tw_ismaster())
	{
		printf("\nHec Model Statistics:\n");
		printf("\t%-50s %11.4lf\n", "Average Waiting Time", wait_time_avg);
		printf("\t%-50s %11lld\n", "Number of hecs", 
			nlp_per_pe * g_tw_npe * tw_nnodes());
		printf("\t%-50s %11lld\n", "Number of planes", 
			planes_per_hec * nlp_per_pe * g_tw_npe * tw_nnodes());
	}

	tw_end();
	
	return 0;
}
