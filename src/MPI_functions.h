#ifndef MPI_FUNC_H
#define MPI_FUNC_H

#include <mpi.h>

struct Shared_mem{
  MPI_Win win;
  float *ptr;
};

extern "C" int init(int *world_rank, int *world_size)
{
  printf("Init the MPI process\n");
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, world_size);
}


extern "C" int allocate_slots(const int &num_slots, const int &num_parameters, Shared_mem &shared_slots)
{
  printf("Allocating slots.\n");
  int size = num_slots * num_parameters;
  MPI_Win_allocate(sizeof(float) * size, sizeof(float), MPI_INFO_NULL, MPI_COMM_WORLD, &(shared_slots.ptr), &(shared_slots.win));
}

extern "C" int allocate_table(const int &num_slots, Shared_mem &shared_table)
{
  printf("Allocating table.\n");
  int num_info = 3;
  int size = num_info * num_slots;
  MPI_Win_allocate(sizeof(int) * size, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &(shared_table.ptr), &(shared_table.win));
}

extern "C" int finish(Shared_mem &shared_table, Shared_mem &shared_slots)
{
  printf("Finish the MPI program\n");
  MPI_Win_free(&(shared_slots.win)); //will delete the memory
  MPI_Win_free(&(shared_table.win));
  MPI_Finalize();
}


#endif