#include "mapred.h"

void
event_logging_f(mr_client_state *s, mr_client_message * msg, tw_lp * lp)
{
   
  switch(msg->msg_core.type)
    {
    case VS_MR_CLIENT_WRITE_START:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_START","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_SET_UP:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_SET_UP","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_SET_UP_ACK:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_SET_UP_ACK","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_DATA_SEND:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_DATA_SEND","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_DATA_RECV:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_DATA_RECV","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_DATA_SEND_ACK:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_DATA_SEND_ACK","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_DONE:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_DONE","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_SOCKET_SET_UP:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_SOCKET_SET_UP","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_SOCKET_SET_UP_ACK:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_SOCKET_SET_UP_ACK","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_CLOSE:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_CLOSE","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_CLOSE_ACK:
      {
	fprintf(s->logfile,"\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_CLOSE_ACK","Vtime: ",tw_now(lp));
	break;
      }

    default:
      fprintf(s->logfile,"\t%-4s","Unknown event type, please check! ... ... ... ...\n");
    }

}

void
event_logging(mr_client_state *s, mr_client_message * msg, tw_lp * lp)
{

  switch(msg->msg_core.type)
    {
    case VS_MR_CLIENT_WRITE_START:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_START","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_SET_UP:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_SET_UP","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_SET_UP_ACK:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_SET_UP_ACK","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_DATA_SEND:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_DATA_SEND","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_DATA_RECV:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_DATA_RECV","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_DATA_SEND_ACK:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_DATA_SEND_ACK","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_DONE:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_DONE","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_SOCKET_SET_UP:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_SOCKET_SET_UP","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_SOCKET_SET_UP_ACK:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_SOCKET_SET_UP_ACK","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_CLOSE:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_CLOSE","Vtime: ",tw_now(lp));
	break;
      }
    case VS_MR_CLIENT_WRITE_CLOSE_ACK:
      {
	printf("\t%-4s %-6d %-15s %-40s %-6s %6f\n","LP: ",lp->gid,"EventType:","VS_MR_CLIENT_WRITE_CLOSE_ACK","Vtime: ",tw_now(lp));
	break;
      }

    default:
      printf("\t%-4s","Unknown event type, please check! ... ... ... ...\n");
    }
}
