#include "mapred.h"

tw_lptype mapred_lps[] =
{
  {
    (init_f) mr_server_init,
    (event_f) mr_server_event_handler,
    (revent_f) mr_server_rc_event_handler,
    (final_f) mr_server_final,
    (map_f) mr_server_mapping,
    sizeof(mr_server_state),
  },
  {
    (init_f) mr_client_init,
    (event_f) mr_client_event_handler,
    (revent_f) mr_client_rc_event_handler,
    (final_f) mr_client_final,
    (map_f) mr_client_mapping,
    sizeof(mr_client_state),
  },
  {0},
};

const tw_optdef app_opt [] =
{
  TWOPT_GROUP("Mapred Model"),
  TWOPT_UINT("nplanes", planes_per_mapred, "initial # of planes per mapred(events)"),
  TWOPT_UINT("memory", opt_mem, "optimistic memory"),
  TWOPT_END()
};

int main(int argc, char **argv, char **env)
{
  int i;
     
  tw_opt_add(app_opt);
  tw_init(&argc, &argv);
  
  nlp_per_pe = (N_CLIENTS+N_NAMENODES+N_DATANODES)/(tw_nnodes() * g_tw_npe);
  g_tw_events_per_pe =(planes_per_mapred * nlp_per_pe / g_tw_npe) + opt_mem; 

  tw_define_lps(nlp_per_pe, sizeof(mr_client_message), 0);

  printf("g tw nlp is %d and nlp per pe is %d\n\n",g_tw_nlp,nlp_per_pe);

  tw_lp_settype(0, &mapred_lps[0]);
  for(i = 1; i < g_tw_nlp; i++)
    tw_lp_settype(i, &mapred_lps[1]);

  tw_run();

  if(tw_ismaster())
    {
      printf("\nMapred Model Statistics:\n");
      //printf("\t%-50s %11.4lf\n", "Average Waiting Time", wait_time_avg);
      printf("\t%-50s %11lld\n", "Number of mapreds", 
	     nlp_per_pe * g_tw_npe * tw_nnodes());
      printf("\t%-50s %11lld\n", "Number of planes", 
	     planes_per_mapred * nlp_per_pe * g_tw_npe * tw_nnodes());
    }

  tw_end();

  return 0;
}
