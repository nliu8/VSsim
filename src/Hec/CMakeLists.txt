#INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR})

SET(hec_srcs hec.c hec.h)

SET( CMAKE_C_FLAGS "-DUSE_AVL_TREE -DAVL_NODE_COUNT=262144 -DRAND_NORMAL -DROSS_timing -DROSS_QUEUE_splay -DROSS_RAND_clcg4 -DROSS_NETWORK_mpi -DROSS_CLOCK_i386 -DROSS_GVT_mpi_allreduce -DARCH_i386"  )

SET( CMAKE_SHARED_LINKER_FLAGS "-DROSS_OPTION_LIST='" -DUSE_AVL_TREE -DAVL_NODE_COUNT=262144 -DRAND_NORMAL -DROSS_timing -DROSS_QUEUE_splay -DROSS_RAND_clcg4 -DROSS_NETWORK_mpi -DROSS_CLOCK_i386 -DROSS_GVT_mpi_allreduce -DARCH_i386"'" ) 

INCLUDE_DIRECTORIES(/opt/include)
ADD_EXECUTABLE(hec ${hec_srcs})

TARGET_LINK_LIBRARIES(hec /opt/lib/libROSS.a m)


