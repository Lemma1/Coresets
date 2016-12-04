#include "config.h"
#include "data.h"
#include "worker.h"

#include "MPI_functions.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

int main(int argc, char** argv) 
{
  ConfReader conf_reader = ConfReader("../config.conf", "GEN");
  int num_slots = conf_reader.get_int("num_slots");
  int num_parameters = conf_reader.get_int("num_parameters");
  int num_files = conf_reader.get_int("num_files");
  int num_data = conf_reader.get_int("num_data");
  int slot_size = conf_reader.get_int("slot_size");
  std::string file_name = conf_reader.get_string("data_file_name");

  int world_rank;
  int world_size;
  init(&world_rank, &world_size);

  // printf("%d, %d\n", world_rank, world_size);


  Shared_mem_int shared_table = Shared_mem_int();
  allocate_table(num_slots, shared_table);
  // printf("For worker %d, the pointer is %x\n", world_rank, shared_table.ptr);

  Shared_mem_float shared_slots = Shared_mem_float();
  allocate_slots(num_slots, num_parameters, shared_slots);
  // printf("For worker %d, the pointer is %x\n", world_rank, shared_slots.ptr);

  Shared_mem_int shared_file_table = Shared_mem_int();
  allocate_file_table(num_files, shared_file_table);
  // printf("For worker %d, the pointer is %x\n", world_rank, shared_file_table.ptr);

  MPI_File *files = new MPI_File[num_files];
  open_files(num_files, files, shared_file_table, file_name);



  // int n_float = 2;
  // float buf[n_float];
  // MPI_Status status;
  // MPI_File_read_at(files[0], world_rank * n_float * sizeof(float), buf, n_float, MPI_FLOAT, &status);

  // for (int i=0; i<n_float; ++i){
  //   printf("in rank %d, now is %f\n", world_rank, buf[i]);
  // }

  CSET_Data *data = new CSET_Data(num_files, num_data, slot_size, num_parameters, files, shared_file_table);

  CSET_Worker *worker = new CSET_Worker(world_rank, num_slots, num_parameters, slot_size,
              shared_table, shared_slots, data);

  worker -> evolve();


  delete files;
  finish(shared_table, shared_slots, shared_file_table);

}