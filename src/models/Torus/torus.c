#include "torus.h"

//If the number of LPs is not evenly divisible the the number of processors, there is a remainder
int
getRem()
{
     return 0;
}
/*Get the MPI process ID mapped to the torus node ID*/
int 
getProcID( tw_lpid lpid )
{
	  return lpid - N_nodes;
}

/*Takes a MPI LP id and a torus node LP ID, returns the process ID on which the lp is mapped */
tw_peid 
mapping( tw_lpid gid )
{
    int rank;
    int rem = 0;

    if(gid < N_nodes)   {
         rank = gid / nlp_nodes_per_pe;
      }
    else {
         rank = getProcID( gid ) / nlp_mpi_procs_per_pe;
      }
    return rank;
}

/*Initialize the torus model, this initialization part is borrowed from Ning's torus model */
void
torus_init( nodes_state * s, 
	   tw_lp * lp )
{
    int i, j;
    int dim_N[ N_dims + 1 ];
    dim_N[ 0 ]=( int )lp->gid;

  // calculate my torus co-ordinates
  for ( i=0; i < N_dims; i++ ) 
    {
      s->dim_position[ i ] = dim_N[ i ]%dim_length[ i ];
      dim_N[ i + 1 ] = ( dim_N[ i ] - s->dim_position[ i ] )/dim_length[ i ];

      half_length[ i ] = dim_length[ i ] / 2;
    }

  factor[ 0 ] = 1;
  for ( i=1; i < N_dims; i++ )
    {
      factor[ i ] = 1;
      for ( j = 0; j < i; j++ )
        factor[ i ] *= dim_length[ j ];
    }

  int temp_dim_pos[ N_dims ];
  for ( i = 0; i < N_dims; i++ )
    temp_dim_pos[ i ] = s->dim_position[ i ];

//if( lp->gid == TRACK_LP )
// printf("\n LP %d assigned dimensions %d %d %d %d %d", (int)lp->gid, s->dim_position[ 0 ], s->dim_position[ 1 ], s->dim_position[ 2 ], s->dim_position[ 3 ], s->dim_position[ 4 ]);

  // calculate minus neighbour's lpID
  for ( j = 0; j < N_dims; j++ )
    {
      temp_dim_pos[ j ] = (s->dim_position[ j ] -1 + dim_length[ j ]) % dim_length[ j ];

      s->neighbour_minus_lpID[ j ] = 0;
      
      for ( i = 0; i < N_dims; i++ )
	{
        s->neighbour_minus_lpID[ j ] += factor[ i ] * temp_dim_pos[ i ];
	}
     
/*	if(lp->gid == 967)
		printf("Neighbor %d ", s->neighbour_minus_lpID[ j ]);*/
      temp_dim_pos[ j ] = s->dim_position[ j ];

    }
  // calculate plus neighbour's lpID
  for ( j = 0; j < N_dims; j++ )
    {
      temp_dim_pos[ j ] = ( s->dim_position[ j ] + 1 + dim_length[ j ]) % dim_length[ j ];

      s->neighbour_plus_lpID[ j ] = 0;
      
      for ( i = 0; i < N_dims; i++ )
        s->neighbour_plus_lpID[ j ] += factor[ i ] * temp_dim_pos[ i ];

/*	if(lp->gid == 967)
		printf("Neighbor %d ", s->neighbour_plus_lpID[ j ]);*/
      temp_dim_pos[ j ] = s->dim_position[ j ];
    }

  for( j=0; j < 2 * N_dims; j++ )
   {
    for( i = 0; i < NUM_VC; i++ )
     {
       s->buffer[ j ][ i ] = 0; //NUM_BUF_SLOTS * num_chunks;
       s->next_link_available_time[ j ][ i ] = 0.0;
      // s->next_credit_available_time[j][i] = 0.0; // Commented out on May 22
     }
   }
  // record LP time
    s->packet_counter = 0;
    s->waiting_list = tw_calloc(TW_LOC, "waiting list", sizeof(struct waiting_packet), WAITING_PACK_COUNT);
    
    for (j = 0; j < WAITING_PACK_COUNT - 1; j++) {
    s->waiting_list[j].next = &s->waiting_list[j + 1];
    s->waiting_list[j].dim = -1;
    s->waiting_list[j].dir = -1;
    s->waiting_list[j].packet = NULL; 
    }
    
    s->waiting_list[j].next = NULL;
    s->waiting_list[j].dim = -1;
    s->waiting_list[j].dir = -1;
    s->waiting_list[j].packet = NULL;
 
    s->head = &s->waiting_list[0];
    s->wait_count = 0;
//    s->next_available_time = 0;
//    s->N_wait_to_be_processed = 0;

}
// initialize MPI process LP
void 
mpi_init( mpi_process * s, 
	 tw_lp * lp)
{
    tw_event *e;
    tw_stime ts;
    nodes_message *m;
    s->message_counter = 0;

    s->row = getProcID(lp->gid) / num_rows;
    s->col = getProcID(lp->gid) % num_cols;

    //Start a GENERATE event on each LP
    ts =  tw_rand_exponential(lp->rng, MEAN_INTERVAL);
    e = tw_event_new( lp->gid, ts, lp );
    m = tw_event_data( e );
    m->type = MPI_SEND;

    s->available_time = 0.0;
    s->zone_id = getProcID(lp->gid) / NUM_ZONE_NODES;

    tw_event_send( e );
}

/*Returns the next neighbor to which the packet should be routed by using DOR (Taken from Ning's code of the torus model)*/
void 
dimension_order_routing( nodes_state * s,
			     tw_lpid * dst_lp, 
			     int * dim, 
			     int * dir )
{
  int dim_N[ N_dims ], 
      dest[ N_dims ],
      i;

  dim_N[ 0 ] = *dst_lp;

  // find destination dimensions using destination LP ID 
  for ( i = 0; i < N_dims; i++ )
    {
      dest[ i ] = dim_N[ i ] % dim_length[ i ];
      dim_N[ i + 1 ] = ( dim_N[ i ] - dest[ i ] ) / dim_length[ i ];
    }

  for( i = 0; i < N_dims; i++ )
    {
      if ( s->dim_position[ i ] - dest[ i ] > half_length[ i ] )
	{
	  *dst_lp = s->neighbour_plus_lpID[ i ];
	  *dim = i;
	  *dir = 1;
	  break;
	}
      if ( s->dim_position[ i ] - dest[ i ] < -half_length[ i ] )
	{
	  *dst_lp = s->neighbour_minus_lpID[ i ];
	  *dim = i;
	  *dir = 0;
	  break;
	}
      if ( ( s->dim_position[ i ] - dest[ i ] <= half_length[ i ] ) && ( s->dim_position[ i ] - dest[ i ] > 0 ) )
	{
	  *dst_lp = s->neighbour_minus_lpID[ i ];
	  *dim = i;
	  *dir = 0;
	  break;
	}
      if (( s->dim_position[ i ] - dest[ i ] >= -half_length[ i ] ) && ( s->dim_position[ i ] - dest[ i ] < 0) )
	{
	  *dst_lp = s->neighbour_plus_lpID[ i ];
	  *dim = i;
	  *dir = 1;
	  break;
	}
    }
}
/*Generates a packet. If there are two buffer slots available, then the packet is 
injected in the network. Else, the packet is placed in the injection queue */
void 
packet_generate( nodes_state * s, 
		tw_bf * bf, 
		nodes_message * msg, 
		tw_lp * lp )
{
    int i, j, tmp_dir=-1, tmp_dim=-1;
    tw_stime ts;

//    event triggered when packet head is sent
    tw_event * e_h;
    nodes_message *m;

    if(TRAFFIC == NEAREST_NEIGHBOR)
     {
	int dest_counter = msg->dest_lp;
	if( dest_counter < N_dims)
	   msg->dest_lp = s->neighbour_minus_lpID[dest_counter];
	  else if(dest_counter >= N_dims && dest_counter < 2 * N_dims)
	     msg->dest_lp = s->neighbour_plus_lpID[dest_counter-N_dims];
     }
   
   if(TRAFFIC == DIAGNOL)
     {
        msg->dest_lp = 0;
	int i, dest[N_dims];
	for( i = 0; i < N_dims; i++ )
	 {
	   dest[i] = dim_length[i] - s->dim_position[i] -1;
	   msg->dest_lp += factor[ i ] * dest[ i ];
	 }
//	printf("\n LP GID %d sending message to %d dim %d %d %d %d %d", (int)lp->gid, (int)msg->dest_lp, dest[0], dest[1], dest[2], dest[3], dest[4]);
     }
  
    tw_lpid dst_lp = msg->dest_lp; //s->neighbour_plus_lpID[0];
    dimension_order_routing( s, &dst_lp, &tmp_dim, &tmp_dir );

  if(tmp_dir == -1 || tmp_dim == -1)
    printf("\n LP %d dest LP %d dim %d dir %d ", (int)lp->gid, (int)msg->dest_lp, tmp_dim, tmp_dir);

    for(j = 0; j < num_chunks; j++)
    { 
       ts = 0.1 + tw_rand_exponential(lp->rng, (j + MEAN_INTERVAL/200));
       e_h = tw_event_new( lp->gid, ts, lp );
       m = tw_event_data( e_h );
       m->next_stop = dst_lp;
       m->dest_lp = msg->dest_lp;
       m->count = msg->count;
       m->travel_start_time = msg->travel_start_time;
       m->packet_ID = msg->packet_ID;
       m->chunk_id = j;
       m->sender_lp = -1;

       int dim_N[ N_dims ];
       dim_N[ 0 ] = m->dest_lp;
   
      // find destination dimensions using destination LP ID 
       for (i=0; i < N_dims; i++)
        {
           m->dest[ i ] = dim_N[ i ] % dim_length[ i ];
           dim_N[ i + 1 ] = ( dim_N[ i ] - m->dest[ i ] ) / dim_length[ i ];
        }

//       For reverse computation  17-05 placed these two lindes out of the if condition below
       if(s->buffer[ tmp_dir + ( tmp_dim * 2 ) ][ 0 ] < NUM_BUF_SLOTS * num_chunks)
        {
	 m->my_N_hop = 0;
	 m->wait_type = -1;
	
        // Send the packet out
	 m->type = SEND;
         m->source_direction = tmp_dir;
         m->source_dim = tmp_dim;
#if DEBUG
/*if(m->packet_ID == TRACK)
   printf("\n (%lld) Packet generated %lld 
		     Buffer space %d 
		     time %lf tmp_dir %d tmp_dim %d 
		     num_chunks %d dest_lp %lld", 
		     lp->gid, m->packet_ID, 
		     s->buffer[ tmp_dir + ( tmp_dim * 2 ) ][ 0 ], 
		     tw_now(lp), tmp_dir, tmp_dim, 
		     num_chunks, msg->dest_lp );*/
#endif
        }
      else 
       {
#if DEBUG
//if(m->packet_ID == TRACK)
//   printf("\n %d Packet queued in line, dir %d dim %d buffer space %d dest LP %d wait type %d ", (int)lp->gid, tmp_dir, tmp_dim, s->buffer[ tmp_dir + ( tmp_dim * 2 ) ][ 0 ], msg->dest_lp, msg->wait_type);
#endif
            m->wait_type = GENERATE;
	    m->type = WAIT;		
	    m->wait_dir = tmp_dir;	
	    m->wait_dim = tmp_dim;
       }
       tw_event_send(e_h);
   }
}
/*Sends a 8-byte credit back to the torus node LP that sent the message */
void 
credit_send( nodes_state * s, 
	    tw_bf * bf, 
	    tw_lp * lp, 
	    nodes_message * msg)
{
#if DEBUG
//if(lp->gid == TRACK_LP)
//	printf("\n (%lf) sending credit tmp_dir %d tmp_dim %d %lf ", tw_now(lp), msg->source_direction, msg->source_dim, credit_delay );
#endif
    bf->c1 = 0;
    tw_event * buf_e;
    nodes_message *m;
    tw_stime ts;
    int src_dir = msg->source_direction;
    int src_dim = msg->source_dim;

    msg->saved_available_time = s->next_credit_available_time[(2 * src_dim) + src_dir][0];
    s->next_credit_available_time[(2 * src_dim) + src_dir][0] = max(s->next_credit_available_time[(2 * src_dim) + src_dir][0], tw_now(lp));
    ts =  credit_delay + tw_rand_exponential(lp->rng, credit_delay/100);
    s->next_credit_available_time[(2 * src_dim) + src_dir][0] += ts;

    buf_e = tw_event_new( msg->sender_lp, s->next_credit_available_time[(2 * src_dim) + src_dir][0] - tw_now(lp), lp);
   //   buf_e = tw_event_new( msg->sender_lp, ts, lp);

    m = tw_event_data(buf_e);
    m->source_direction = msg->source_direction;
    m->source_dim = msg->source_dim;

    m->type = CREDIT;
    tw_event_send( buf_e );
    if(m->source_dim == 2 && m->source_direction == 1 && msg->sender_lp == 967)
     {
	bf->c1 = 1;
	credit_sent++;	
//	printf("\n Credit from %d ", (int)lp->gid);
     }
}
/*Inserts a packet in the injection queue which then waits to be sent over the network */
void 
update_waiting_list( nodes_state * s, 
		     tw_bf * bf,
		     nodes_message * msg, 
		     tw_lp * lp )
{
   int loc = s->wait_count;

   if(loc >= WAITING_PACK_COUNT)
	printf(" Reached maximum count of linked list %d ", s->wait_count);

/*   printf("\n Inserting message wait type %d buffer %d dir %d dim %d", msg->wait_type,
						  s->buffer[(msg->wait_dim * 2) + msg->wait_dir][0],
						  msg->wait_dir,
						  msg->wait_dim);*/
   s->waiting_list[loc].dim = msg->wait_dim;
   s->waiting_list[loc].dir = msg->wait_dir;
   s->waiting_list[loc].packet = msg;

   s->wait_count = s->wait_count + 1;
}
// send a packet from one torus node to another torus node
// A packet can be up to 256 bytes on BG/L and BG/P and up to 512 bytes on BG/Q
void 
packet_send( nodes_state * s, 
	         tw_bf * bf, 
		 nodes_message * msg, 
		 tw_lp * lp )
{ 
    bf->c3 = 0; 
    bf->c2 = 0;
    bf->c1 = 0;
    int i, vc = 0, tmp_dir, tmp_dim;
    tw_stime ts;
    tw_event *e;
    nodes_message *m;
    tw_lpid dst_lp = msg->dest_lp;
    int tokens_min = 0;
  
    if( msg->next_stop == -1 )  
     {
 	 dimension_order_routing( s, &dst_lp, &tmp_dim, &tmp_dir );     
     }
    else
      {
	// this is a waiting message 
	 dst_lp = msg->next_stop; 
 
	 if(msg->wait_type == -1)
	  {
	   tmp_dir = msg->source_direction;
	   tmp_dim = msg->source_dim;
 	  }
 	 else
 	 {
           tmp_dim = msg->wait_dim;
           tmp_dir = msg->wait_dir;
	 }
     }

    if(s->buffer[ tmp_dir + ( tmp_dim * 2 ) ][ 0 ] >= NUM_BUF_SLOTS * num_chunks)
    {
         // re-schedule the message in the future
	 ts = 0.1 + tw_rand_exponential( lp->rng, MEAN_INTERVAL/100);
	 e = tw_event_new( lp->gid, ts, lp );
	 m = tw_event_data( e );	
         m->wait_type = SEND;
	 m->type = WAIT;
  	 
         m->wait_dim = tmp_dim;
         m->wait_dir = tmp_dir;
         
         m->next_stop = dst_lp;
	 m->chunk_id = msg->chunk_id;
         m->dest_lp = msg->dest_lp;
         m->count = msg->count;
         
	 // added may 20
	 m->source_direction = msg->source_direction;
	 m->source_dim = msg->source_dim;

	 m->packet_ID = msg->packet_ID;
	 m->sender_lp = msg->sender_lp;

	 for (i=0; i < N_dims; i++)
           m->dest[i] = msg->dest[i];

	 tw_event_send(e);
   }
  else
  {
       bf->c3 = 1;
       msg->saved_src_dir = tmp_dir;
       msg->saved_src_dim = tmp_dim;
       ts = tw_rand_exponential( lp->rng, ( double )head_delay/10 )+
                              head_delay + OVERHEAD;

//    For reverse computation 
      msg->saved_available_time = s->next_link_available_time[tmp_dir + ( tmp_dim * 2 )][0];

      s->next_link_available_time[tmp_dir + ( tmp_dim * 2 )][0] = max( s->next_link_available_time[ tmp_dir + ( tmp_dim * 2 )][0], tw_now(lp) );
      s->next_link_available_time[tmp_dir + ( tmp_dim * 2 )][0] += ts;
    
      e = tw_event_new( dst_lp, s->next_link_available_time[tmp_dir + ( tmp_dim * 2 )][0] - tw_now(lp), lp );
    
      m = tw_event_data( e );
      m->type = ARRIVAL;
 
      //Carry on the message info
      m->source_dim = tmp_dim;
      m->source_direction = tmp_dir;
      m->next_stop = dst_lp;
      m->sender_lp = lp->gid;
      m->chunk_id = msg->chunk_id;
   
      for( i = 0; i < N_dims; i++ )
         m->dest[ i ] = msg->dest[ i ];
     
      m->dest_lp = msg->dest_lp;
  
      m->packet_ID = msg->packet_ID;
      m->travel_start_time = msg->travel_start_time;
      m->count = msg->count;
  
      m->my_N_hop = msg->my_N_hop;
      tw_event_send( e );

      if(lp->gid == 967 && tmp_dir == 1 && tmp_dim == 2)
       {
//	printf("\n Sending packet to LP %d ", dst_lp);
	bf->c2 = 1;	
	packet_sent++;
       }
		
      s->buffer[ tmp_dir + ( tmp_dim * 2 ) ][ 0 ]++;
    
      if(msg->chunk_id == num_chunks - 1 && msg->sender_lp == -1)
      {
        bf->c1 = 1;
        int index = floor( N_COLLECT_POINTS * ( tw_now( lp ) / g_tw_ts_end ) );
        N_generated_storage[ index ]++;           
     }
  } // end else
}
/*Once a credit arrives at the node, this method picks a waiting packet in the injection queue and schedules it */
void
waiting_packet_free(nodes_state * s, int loc)
{
  int i, max_count = s->wait_count - 1;

  for(i = loc; i < max_count; i++) {
	s->waiting_list[i].dim = s->waiting_list[i + 1].dim;
	s->waiting_list[i].dir = s->waiting_list[i + 1].dir;
	s->waiting_list[i].packet = s->waiting_list[i + 1].packet;
    }
  s->waiting_list[max_count].dim = -1;
  s->waiting_list[max_count].dir = -1;
  s->waiting_list[max_count].packet = NULL; 
  
  s->wait_count = s->wait_count - 1; 
}

/*Processes the packet after it arrives on the from the neighboring torus node */
void packet_arrive( nodes_state * s, 
		    tw_bf * bf, 
		    nodes_message * msg, 
		    tw_lp * lp )
{
  int i, delay = head_delay;//HOP_DELAY + tw_rand_exponential(lp->rng, HOP_DELAY/1000);
  tw_event *e;
  tw_stime ts;
  nodes_message *m;

  credit_send( s, bf, lp, msg); // Commented on May 22nd to check if the credit needs to be sent from the final destination or not
  
  msg->my_N_hop++;
  ts = 0.1 + tw_rand_exponential(lp->rng, MEAN_INTERVAL/1000);
  if( lp->gid == msg->dest_lp )
    {   
        if( msg->chunk_id == num_chunks - 1 )    
        {
	        e = tw_event_new(lp->gid + N_nodes, ts, lp);
		m = tw_event_data(e);
	        m->type = MPI_RECV;
	        m->travel_start_time = msg->travel_start_time;
		m->my_N_hop = msg->my_N_hop;
		m->packet_ID = msg->packet_ID;
		m->count = msg->count;
		tw_event_send(e);
        }
    }
  else
    {
      e = tw_event_new(lp->gid, ts , lp);
      m = tw_event_data( e );
      m->type = SEND;
      
      // Carry on the message info
      for( i = 0; i < N_dims; i++ )
	m->dest[i] = msg->dest[i];

      m->dest_lp = msg->dest_lp;
      
      m->source_dim = msg->source_dim;
      m->source_direction = msg->source_direction;
      
      m->packet_ID = msg->packet_ID;	  
      m->travel_start_time = msg->travel_start_time;

      m->my_N_hop = msg->my_N_hop;
      m->count = msg->count;
      m->sender_lp = msg->sender_lp;
      m->chunk_id = msg->chunk_id;

      m->next_stop = -1;
      tw_event_send(e);
   }
}

/*Each MPI LP in this model generates a MPI message until a certain message count is reached.
This method 
          (i) keeps generating MPI messages, 
	 (ii) breaks a MPI message down to torus packets 
         (iii) sends those packets to the underlying torus node LP */
void mpi_msg_send(mpi_process * p, 
	          tw_bf * bf, 
		  nodes_message * msg, 
		  tw_lp * lp)
{
    tw_stime ts;
    tw_event *e;
    nodes_message *m;
    tw_lpid final_dst;
    int i;//, pack_size = mpi_message_size;
    bf->c3 = 0;
    bf->c4 = 0;

    if(p->message_counter >= injection_limit)
     {
	bf->c4 = 1;

	return;
     }

    switch(TRAFFIC)
	{
	  case UNIFORM_RANDOM:
		{
                    bf->c3 = 1;

		    final_dst = tw_rand_integer( lp->rng, N_nodes, 2 * N_nodes - 1);
	
		    if( final_dst == lp->gid )
		      {
                        final_dst = N_nodes + (lp->gid + N_nodes/2) % N_nodes;
		      }
		}
	  break;

	 case DRAGONFLY_ZONES:
		{
		    bf->c3 = 1;

  		    final_dst = N_nodes + ((p->zone_id + 1) % num_zones * NUM_ZONE_NODES) + tw_rand_integer( lp->rng, 0, NUM_ZONE_NODES-1);
		
		}
	 break;

	case TRANSPOSE:
		{
		   if( p->col == p->row )
		    {
		      bf->c4 = 1;

                      return;
                    }

		   final_dst = N_nodes + p->col * num_rows + p->row;
		}
         break;

       case NEAREST_NEIGHBOR:
         {
           final_dst = p->message_counter% 10;
        }
        break;

	case DIAGNOL:
	  {
	    final_dst = -1;
	  }	
     }
      tw_stime base_time = MEAN_PROCESS;
	
      for( i=0; i < num_packets; i++ ) 
       {
	      // Send the packet out
	     ts = 0.1 + tw_rand_exponential(lp->rng, MEAN_INTERVAL/100); 
             msg->saved_available_time = p->available_time;
	     p->available_time = max( p->available_time, tw_now(lp) );
	     p->available_time += ts;

	     e = tw_event_new( getProcID(lp->gid), p->available_time - tw_now(lp), lp );

	     //e = tw_event_new( getProcID(lp->gid), ts, lp );
	     m = tw_event_data( e );
	     m->type = GENERATE;
             m->packet_ID = packet_offset * ( lp->gid * num_mpi_msgs * num_packets ) + p->message_counter;

             p->message_counter++;
	     m->count = i;
	     m->travel_start_time = tw_now( lp ) + ts;

	     if(TRAFFIC == NEAREST_NEIGHBOR || TRAFFIC == DIAGNOL)
		m->dest_lp = final_dst;
	     else
	       {
	        m->dest_lp = getProcID( final_dst );
	 	}

 	     m->next_stop = -1; 
             tw_event_send( e );
     } 
     ts = 0.1 + tw_rand_exponential( lp->rng, MEAN_INTERVAL );
     e = tw_event_new( lp->gid, ts, lp );
     m = tw_event_data( e );
     m->type = MPI_SEND;
     tw_event_send( e );
}
/*Computes final latencies after a message is received */
void mpi_msg_recv(mpi_process * p, 
		  tw_bf * bf, 
		  nodes_message * msg, 
		  tw_lp * lp)
{
 // Message arrives at final destination
   bf->c3 = 0; 
   N_finished_msgs++;
    
// For torus-dragonfly comparison only place the end time here
    N_finished_packets++;
    int index = floor(N_COLLECT_POINTS*(tw_now(lp)/g_tw_ts_end));
    N_finished_storage[index]++;

    total_time += tw_now( lp ) - msg->travel_start_time;
    total_hops += msg->my_N_hop;

    if (max_latency < tw_now( lp ) - msg->travel_start_time) {
	  bf->c3 = 1;
	  msg->saved_available_time = max_latency;
          max_latency=tw_now( lp ) - msg->travel_start_time;
     }
}
void mpi_event_handler( mpi_process * p, 
		       tw_bf * bf, 
		       nodes_message * msg, 
		       tw_lp * lp )
{
  *(int *) bf = (int) 0;
  switch(msg->type)
  {
     case MPI_SEND:
   	      mpi_msg_send(p, bf, msg, lp);
     break;

     case MPI_RECV:
              mpi_msg_recv(p, bf, msg, lp);
      break; 
  
     DEFAULT:
		printf("\n Wrong mapping");
      break;
  }
}

void mpi_event_rc_handler( mpi_process * p,
                           tw_bf * bf,
                           nodes_message * msg,
                           tw_lp * lp)
{
 switch(msg->type)
  {
     case MPI_SEND:
                {
		  if(bf->c4)
		    return;

		  if(bf->c3)
		       tw_rand_reverse_unif(lp->rng);

		    int i; 
		    for( i=0; i < num_packets; i++ )
		     {
			     tw_rand_reverse_unif(lp->rng);

			     p->available_time = msg->saved_available_time;
			     p->message_counter--;			
		     }
		    tw_rand_reverse_unif(lp->rng);
	     }
     break;

     case MPI_RECV:
		  {
			int index = floor( N_COLLECT_POINTS * ( tw_now( lp ) / g_tw_ts_end ) );
			N_finished_msgs--;
			N_finished_packets--;
			N_finished_storage[index]--;
			total_time -= tw_now( lp ) - msg->travel_start_time;
			total_hops-=msg->my_N_hop;

			if(bf->c3)
			  max_latency= msg->saved_available_time;
		  }
      break;
  }
}

void
final( nodes_state * s, tw_lp * lp )
{ 

}

tw_lp * torus_mapping_to_lp( tw_lpid lpid )
{
    int index;

    if(lpid < N_nodes)
       index = lpid - g_tw_mynode * nlp_nodes_per_pe;
    else
       index = nlp_nodes_per_pe + (lpid - g_tw_mynode * nlp_mpi_procs_per_pe - N_nodes);

    return g_tw_lp[index];
}

void packet_buffer_process( nodes_state * s, tw_bf * bf, nodes_message * msg, tw_lp * lp )
{
   msg->wait_loc = -1;
   s->buffer[ msg->source_direction + ( msg->source_dim * 2 ) ][  0 ]-=1;
  
   //if(lp->gid == 967 && msg->source_direction == 1 && msg->source_dim == 2 && s->buffer[ msg->source_direction + ( msg->source_dim * 2 ) ][  0 ] < 0)   
//    if(s->buffer[ msg->source_direction + ( msg->source_dim * 2 ) ][  0 ] < 0)
	//printf("\n Buffer count went below zero LP %d count %d dir %d dim %d ", lp->gid, s->buffer[ msg->source_direction + ( msg->source_dim * 2 ) ][  0 ], msg->source_direction, msg->source_dim);
//     printf("\n LP ID %lld Dir %d Dim %d Count %d ", lp->gid, msg->source_direction, msg->source_dim, s->buffer[ msg->source_direction + ( msg->source_dim * 2 ) ][  0 ]);
 
  tw_event * e_h;
  nodes_message * m;
  tw_stime ts;
  bf->c3 = 0;
  int loc=s->wait_count, j=0;
  for(j = 0; j < loc; j++)
   {
    if( s->waiting_list[j].dim == msg->source_dim && s->waiting_list[j].dir == msg->source_direction)
     {
        bf->c3=1;
	ts = tw_rand_exponential(lp->rng, MEAN_INTERVAL/100);
        e_h = tw_event_new( lp->gid, ts, lp );
        m = tw_event_data( e_h );
     	memcpy(m, s->waiting_list[j].packet, sizeof(nodes_message));
        
//        For reverse computation, also copy data to the msg 
   	memcpy(msg, s->waiting_list[j].packet, sizeof(nodes_message));
	msg->wait_loc = j;
	msg->type = CREDIT;
	m->type = SEND;
        tw_event_send(e_h);
        waiting_packet_free(s, j);
        break;
    }
  }
}
void 
node_rc_handler(nodes_state * s, tw_bf * bf, nodes_message * msg, tw_lp * lp)
{
  switch(msg->type)
    {
       case GENERATE:
		   {
		     int i;
		     for(i=0; i < num_chunks; i++)
  		        tw_rand_reverse_unif(lp->rng);	
		   }
	break;
	
	case ARRIVAL:
		   {
			if(bf->c1)
			   credit_sent--; 
		     msg->my_N_hop--;
  		     tw_rand_reverse_unif(lp->rng);
		     tw_rand_reverse_unif(lp->rng);
		     int next_dim = msg->source_dim;
		     int next_dir = msg->source_direction;
		     s->next_credit_available_time[next_dir + ( next_dim * 2 )][0] = msg->saved_available_time;
		   }
	break;	

	case SEND:
		 {
	            tw_rand_reverse_unif(lp->rng);
		    if(bf->c3)
		     {
                        int next_dim = msg->saved_src_dim;
			int next_dir = msg->saved_src_dir;

			s->next_link_available_time[next_dir + ( next_dim * 2 )][0] = msg->saved_available_time;
			
			s->buffer[ next_dir + ( next_dim * 2 ) ][ 0 ] --;
//			if( s->buffer[ next_dir + ( next_dim * 2 ) ][ 0 ] < 0)
//			   printf("\n LESS THAN ZERO!!! %d %d ", next_dir, next_dim);
			
                        if(bf->c1)
			  {
			    int index = floor( N_COLLECT_POINTS * ( tw_now( lp ) / g_tw_ts_end ) );
			    N_generated_storage[ index ]--;
			  }
			if(bf->c2)
			  packet_sent--;
		    }
		 }
	break;

        case WAIT:
		{
		     s->wait_count-=1;
		     int loc = s->wait_count;
		     s->waiting_list[loc].dim = -1;
		     s->waiting_list[loc].dir = -1;
		     s->waiting_list[loc].packet = NULL;
		}
        break;

       case CREDIT:
		{
		  s->buffer[ msg->source_direction + ( msg->source_dim * 2 ) ][  0 ]++;
		  if(bf->c3)
		  {
		     tw_rand_reverse_unif(lp->rng);
		     int loc = msg->wait_loc, i;
                     int max_count = s->wait_count;
		     if(s->wait_count >= WAITING_PACK_COUNT)
			printf("\n Exceeded maximum count!!! ");
		     for(i = max_count; i > loc ; i--)  
                      {
		  	  s->waiting_list[i].dim = s->waiting_list[i-1].dim;
			  s->waiting_list[i].dir = s->waiting_list[i-1].dir;
			  s->waiting_list[i].packet = s->waiting_list[i-1].packet;
                      }
		     s->waiting_list[loc].dim = msg->source_dim;
		     s->waiting_list[loc].dir = msg->source_direction;
		     s->waiting_list[loc].packet = msg;		 
		     s->wait_count = s->wait_count + 1;
		 }
              }
       break;
     }
}

void
event_handler(nodes_state * s, tw_bf * bf, nodes_message * msg, tw_lp * lp)
{
 *(int *) bf = (int) 0;
 switch(msg->type)
 {
  case GENERATE:
    packet_generate(s,bf,msg,lp);
  break;
  case ARRIVAL:
    packet_arrive(s,bf,msg,lp);
  break;
  case SEND:
   packet_send(s,bf,msg,lp);
  break;
  case CREDIT:
    packet_buffer_process(s,bf,msg,lp);
   break;
  case WAIT:
    update_waiting_list(s, bf, msg, lp);
   break;
  DEFAULT:
	printf("\n Being sent to wrong LP");
  break;
 }
}
tw_lptype nodes_lps[] =
{
	{
		(init_f) torus_init,
		(event_f) event_handler,
		(revent_f) node_rc_handler,
		(final_f) final,
		(map_f) mapping,
		sizeof(nodes_state),
	},
	{
               (init_f) mpi_init,
	       (event_f) mpi_event_handler,
	       (revent_f) mpi_event_rc_handler,
	       (final_f) final,
	       (map_f) mapping,
	       sizeof(mpi_process),
	},
	{0},
};

void torus_mapping(void)
{
  tw_lpid kpid;
  tw_pe * pe;
  int nkp_per_pe=16;

  for(kpid = 0; kpid < nkp_per_pe; kpid++)
   tw_kp_onpe(kpid, g_tw_pe[0]);

  int i;
  for(i = 0; i < nlp_nodes_per_pe; i++)
   {
     kpid = i % g_tw_nkp;
     pe = tw_getpe(kpid % g_tw_npe);
     tw_lp_onpe(i, pe, g_tw_mynode * nlp_nodes_per_pe + i + getRem() );
     tw_lp_onkp(g_tw_lp[i], g_tw_kp[kpid]);
     tw_lp_settype(i, &nodes_lps[0]);
   }
  for(i = 0; i < nlp_mpi_procs_per_pe; i++)
   {
     kpid = i % g_tw_nkp;
     pe = tw_getpe(kpid % g_tw_npe);
     tw_lp_onpe(nlp_nodes_per_pe+i, pe, N_nodes + g_tw_mynode * nlp_mpi_procs_per_pe + i + getRem() );
     tw_lp_onkp(g_tw_lp[nlp_nodes_per_pe + i], g_tw_kp[kpid]);
     tw_lp_settype(nlp_nodes_per_pe + i, &nodes_lps[1]);
   }
}
const tw_optdef app_opt [] =
{
	TWOPT_GROUP("Nodes Model"),
	TWOPT_UINT("memory", opt_mem, "optimistic memory"),
	TWOPT_ULONG("mpi-message-size", mpi_message_size, "mpi-message-size"),
	TWOPT_UINT("mem_factor", mem_factor, "mem_factor"),
	TWOPT_UINT("traffic", TRAFFIC, "UNIFORM RANDOM=1, DRAGONFLY ZONES=2, TRANSPOSE=3, NEAREST NEIGHBOR=4"), 
	TWOPT_STIME("arrive_rate", MEAN_INTERVAL, "packet arrive rate"),
	TWOPT_END()
};


int
main(int argc, char **argv, char **env)
{
	int i;
	tw_opt_add(app_opt);
	tw_init(&argc, &argv);

	for (i=0; i<N_dims; i++)
        {
	  N_nodes*=dim_length[i];
	  N_mpi_procs*=dim_length[i];
	}
	nlp_nodes_per_pe = N_nodes/tw_nnodes()/g_tw_npe;
	nlp_mpi_procs_per_pe = N_mpi_procs/tw_nnodes()/g_tw_npe;

	num_rows = sqrt(N_nodes);
	num_cols = num_rows;

	total_lps = g_tw_nlp * tw_nnodes();
	node_rem = N_nodes % (tw_nnodes()/g_tw_npe);

	 /*if(g_tw_mynode < node_rem)
	   {
		nlp_nodes_per_pe++;
		nlp_mpi_procs_per_pe++;
	   }*/
        num_packets=1;
        num_chunks = PACKET_SIZE/CHUNK_SIZE;
        num_zones = N_nodes/NUM_ZONE_NODES;

	if( N_nodes % NUM_ZONE_NODES != 0)
	   num_zones++;
      
        if( mpi_message_size > PACKET_SIZE)
         {
          num_packets = mpi_message_size / PACKET_SIZE;  
          
	  if(mpi_message_size % PACKET_SIZE != 0 )
	    num_packets++;
         }

	g_tw_mapping=CUSTOM;
     	g_tw_custom_initial_mapping=&torus_mapping;
        g_tw_custom_lp_global_to_local_map=&torus_mapping_to_lp;

	g_tw_events_per_pe = mem_factor * 1024 * (nlp_nodes_per_pe/g_tw_npe + nlp_mpi_procs_per_pe/g_tw_npe) + opt_mem;
	tw_define_lps(nlp_nodes_per_pe + nlp_mpi_procs_per_pe, sizeof(nodes_message), 0);

	head_delay = (1 / BANDWIDTH) * CHUNK_SIZE;
	
        // BG/L torus network paper: Tokens are 32 byte chunks that is why the credit delay is adjusted according to bandwidth * 32
	credit_delay = (1 / BANDWIDTH) * 8;
	packet_offset = (g_tw_ts_end/MEAN_INTERVAL) * num_packets; 
	
	injection_limit = 40000/MEAN_INTERVAL;

	if(tw_ismaster())
	{
		printf("\nTorus Network Model Statistics:\n");
		printf("\t%-50s %11d\n", "Number of nodes", N_nodes);
		printf(" Injection limit %d \n", injection_limit);
	}

	tw_run();
	unsigned long long total_finished_storage[N_COLLECT_POINTS];
	unsigned long long total_generated_storage[N_COLLECT_POINTS];
	unsigned long long wait_length,event_length,N_total_packets_finish, N_total_msgs_finish, N_total_hop;
	tw_stime total_time_sum,g_max_latency;

	for( i=0; i<N_COLLECT_POINTS; i++ )
	  {
	    MPI_Reduce( &N_finished_storage[i], &total_finished_storage[i],1,
                        MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	    MPI_Reduce( &N_generated_storage[i], &total_generated_storage[i],1,
                        MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	  }

	MPI_Reduce( &total_time, &total_time_sum,1, 
		    MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce( &N_finished_packets, &N_total_packets_finish,1, 
		    MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce( &N_finished_msgs, &N_total_msgs_finish,1, 
                    MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

	MPI_Reduce( &total_hops, &N_total_hop,1, 
		    MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce( &max_latency, &g_max_latency,1, 
		    MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	for( i=1; i<N_COLLECT_POINTS; i++ )
	  {
	    total_finished_storage[i]+=total_finished_storage[i-1];
	    total_generated_storage[i]+=total_generated_storage[i-1];
	  }

//	if( total_lp_time > 0 )
//	   total_lp_time /= num_mpi_msgs;

	if(tw_ismaster())
	  {
	    printf("\n ****************** \n");
	    printf("\n total packets finished:         %lld and %lld; \n",
		   total_finished_storage[N_COLLECT_POINTS-1],N_total_packets_finish);
	    printf("\n total MPI messages finished:         %lld; \n",
                   N_total_msgs_finish);
	    printf("\n total generate:       %lld; \n",
		   total_generated_storage[N_COLLECT_POINTS-1]);
	    printf("\n total hops:           %lf; \n",
		   (double)N_total_hop/total_finished_storage[N_COLLECT_POINTS-1]);
	    printf("\n average travel time:  %lf; \n\n",
		   total_time_sum/N_total_msgs_finish);
	    for( i=0; i<N_COLLECT_POINTS; i++ )
	      {
		printf(" %d ",i*100/N_COLLECT_POINTS);
		printf("total finish: %lld; generate: %lld; alive: %lld \n ",
		       total_finished_storage[i],
		       total_generated_storage[i],
		       total_generated_storage[i]-total_finished_storage[i]);
//#if REPORT_BANDWIDTH	    
//		if(i > 0)
//		   printf(" diff finished: %lld\n", total_finished_storage[i] - total_finished_storage[i-1]); 
//#endif
	      }

#if REPORT_BANDWIDTH	    
	    // capture the steady state statistics
        tw_stime bandwidth;
        tw_stime interval = (g_tw_ts_end / N_COLLECT_POINTS);
i	interval = interval/(1000.0 * 1000.0 * 1000.0); //convert ns to seconds
        for( i=1; i<N_COLLECT_POINTS; i++ )
           {
                bandwidth = total_finished_storage[i] - total_finished_storage[i - 1];
                bandwidth = (bandwidth * PACKET_SIZE) / (1024.0 * 1024.0 * 1024.0); // convert bytes to GB
                bandwidth = bandwidth / interval; 
                printf("\n Interval %lf Bandwidth %lf ", interval, bandwidth);
           }

	    unsigned long long steady_sum=0;
	    for( i = N_COLLECT_POINTS/2; i<N_COLLECT_POINTS;i++)
	      steady_sum+=total_generated_storage[i]-total_finished_storage[i];
	    printf("\n Steady state, packet alive: %lld\n",
		   2*steady_sum/N_COLLECT_POINTS);
#endif
	    printf("\nMax latency is %lf\n\n",g_max_latency);
	}
//	  if(packet_sent > 0 || credit_sent > 0)
//	    printf("\n Packet sent are %d, credit sent %d ", packet_sent, credit_sent);
	tw_end();
	return 0;
}
