#include "worker.h"
#include "config.h"

#include "MPI_functions.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>




int main(int argc, char** argv) {
  ConfReader conf_reader = ConfReader("../config.conf", "GEN");
  int num_slots = conf_reader.get_int("num_slots");
  int num_parameters = conf_reader.get_int("num_parameters");

  int world_rank;
  int world_size;
  init(&world_rank, &world_size);

  printf("%d, %d\n", world_rank, world_size);


  Shared_mem shared_table = Shared_mem();
  allocate_table(num_slots, shared_table);
  printf("For worker %d, the pointer is %x\n", world_rank, shared_table.ptr);

  Shared_mem shared_slots = Shared_mem();
  allocate_slots(num_slots, num_parameters, shared_slots);
  printf("For worker %d, the pointer is %x\n", world_rank, shared_slots.ptr);



  finish(shared_table, shared_slots);

  // int *   sm_ptr = NULL;
  // MPI_Win sm_win;
  // MPI_Win_allocate(sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &sm_ptr, &sm_win);

  // // We are assuming at least 2 processes for this task
  // if (world_size != 2) {
  //   fprintf(stderr, "World size must be two for %s\n", argv[0]);
  //   MPI_Abort(MPI_COMM_WORLD, 1);
  // }

  // int ping_pong_count = 0;
  // int partner_rank = (world_rank + 1) % 2;
  // while (ping_pong_count < PING_PONG_LIMIT) {
  //   if (world_rank == ping_pong_count % 2) {
  //     // Increment the ping pong count before you send it
  //     ping_pong_count++;
  //     MPI_Send(&ping_pong_count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD);
  //     printf("%d sent and incremented ping_pong_count %d to %d\n",
  //            world_rank, ping_pong_count, partner_rank);
  //   } else {
  //     MPI_Recv(&ping_pong_count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD,
  //              MPI_STATUS_IGNORE);
  //     printf("%d received ping_pong_count %d from %d\n",
  //            world_rank, ping_pong_count, partner_rank);
  //   }
  // }
  // MPI_Finalize();
}