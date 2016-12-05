#ifndef MPI_FUNC_H
#define MPI_FUNC_H

#include <mpi.h>


extern "C" int init(int *world_rank, int *world_size)
{
  printf("Init the MPI process\n");
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, world_size);
}


extern "C" int allocate_slots(const int &num_slots, const int &num_parameters, 
                              const int &slot_size,
                              Shared_mem_float &shared_slots)
{
  printf("Allocating slots.\n");
  int size = num_slots * num_parameters * slot_size;
  MPI_Win_allocate(sizeof(float) * size, sizeof(float), MPI_INFO_NULL, MPI_COMM_WORLD, &(shared_slots.ptr), &(shared_slots.win));
  memset(shared_slots.ptr, 0x0, sizeof(float) * size);
}

extern "C" int allocate_table(const int &num_slots, Shared_mem_int &shared_table)
{
  printf("Allocating table.\n");
  int num_info = 3;
  int size = num_info * num_slots;
  MPI_Win_allocate(sizeof(int) * size, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &(shared_table.ptr), &(shared_table.win));
  for (int i=0; i<num_slots; ++i){
    shared_table.ptr[i*num_info] = 0;
    shared_table.ptr[i*num_info + 1] = -1;
    shared_table.ptr[i*num_info + 2] = -1;
  }
}

extern "C" int allocate_file_table(const int &num_files, Shared_mem_int &shared_file_table)
{
  printf("Allocating file table.\n");
  int num_info = 2;
  int size = num_info * num_files;
  MPI_Win_allocate(sizeof(int) * size, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &(shared_file_table.ptr), &(shared_file_table.win));
  for (int i=0; i<num_files; ++i){
    shared_file_table.ptr[i*num_info] = 0;
    shared_file_table.ptr[i*num_info + 1] = 0;
  }
}

extern "C" int open_files(const int &num_files, MPI_File *files, Shared_mem_int &shared_file_table, std::string file_name)
{
  std::string full_name;
  for (int i=0; i<num_files; ++i){
    full_name = file_name + std::to_string(i);
    MPI_File_open(MPI_COMM_WORLD,full_name.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &(files[i]));
  }
  
}

extern "C" int finish(const int &rank, Shared_mem_int &shared_table, Shared_mem_float &shared_slots, Shared_mem_int &shared_file_table)
{
  printf("Worker %d, Finish the MPI program\n", rank);
  MPI_Win_free(&(shared_slots.win)); //will delete the memory
  MPI_Win_free(&(shared_table.win));
  MPI_Win_free(&(shared_file_table.win));
  MPI_Finalize();
}


#endif