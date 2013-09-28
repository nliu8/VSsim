#ifndef MR_EXTERN_H
#define MR_EXTERN_H

#include "mapred.h"

// mapper reducer client external function
extern void mr_client_mapping( tw_lpid );
extern void mr_client_init(mr_client_state*, tw_lp* );
extern void mr_client_event_handler(mr_client_state*, tw_bf*, mr_client_message*, tw_lp*);
extern void mr_client_rc_event_handler(mr_client_state*, tw_bf*, mr_client_message*, tw_lp*);
extern void mr_client_final(mr_client_state*, tw_lp*);

// mapper reducer server external function
extern void mr_server_mapping( tw_lpid );
extern void mr_server_init(mr_server_state*, tw_lp* );
extern void mr_server_event_handler(mr_server_state*, tw_bf*, mr_server_message*, tw_lp*);
extern void mr_server_rc_event_handler(mr_server_state*, tw_bf*, mr_server_message*, tw_lp*);
extern void mr_server_final(mr_server_state*, tw_lp*);

#endif

