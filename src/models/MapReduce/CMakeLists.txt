#INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR})

SET(mapred_srcs mapred.c mapred.h mapred_extern.h mapred_logging.c mapred_helper.c mr_client.c mr_server.c)

SET( CMAKE_C_FLAGS "-DUSE_AVL_TREE -DAVL_NODE_COUNT=262144 -DRAND_NORMAL -DROSS_timing -DROSS_QUEUE_splay -DROSS_RAND_clcg4 -DROSS_NETWORK_mpi -DROSS_CLOCK_i386 -DROSS_GVT_mpi_allreduce -DARCH_i386"  )

SET( CMAKE_SHARED_LINKER_FLAGS "-DROSS_OPTION_LIST='" -DUSE_AVL_TREE -DAVL_NODE_COUNT=262144 -DRAND_NORMAL -DROSS_timing -DROSS_QUEUE_splay -DROSS_RAND_clcg4 -DROSS_NETWORK_mpi -DROSS_CLOCK_i386 -DROSS_GVT_mpi_allreduce -DARCH_i386"'" ) 

INCLUDE_DIRECTORIES(/opt/include)
ADD_EXECUTABLE(mapred ${mapred_srcs})

TARGET_LINK_LIBRARIES(mapred /opt/lib/libROSS.a m)


