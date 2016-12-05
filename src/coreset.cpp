#include "config.h"
#include "data.h"
#include "worker.h"

#include "MPI_functions.h"

#include <fstream>
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
  allocate_slots(num_slots, num_parameters, slot_size, shared_slots);
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

  int task_idx;
  int max_counter = 100;
  int counter = 0;
  while(true){
    task_idx = worker -> evolve();
    // printf("worker %d, task_idx is now %d\n", world_rank, task_idx);
    if(!task_idx) break;
    if(task_idx == -1) ++counter;
    if(counter == max_counter) break;
  }


  if(world_rank == 0){
    std::ofstream outfile;
    float r;
    std::string outfile_name = conf_reader.get_string("outfile_name");
    outfile.open(outfile_name, std::ios::out | std::ios::binary);
    int slot_idx = -1;
    for(int i=0; i<num_slots; ++i){
      if (shared_table.ptr[i * 3] == 1){
        slot_idx = i;
        break;
      }
    }
    if (slot_idx == -1) {
      printf("ERROR! wrong slot index\n");
      exit(-1);
    }
    float *ptr = &shared_slots.ptr[slot_idx * slot_size * num_parameters];
    MPI_Get(ptr, 
      num_parameters * slot_size, 
      MPI_FLOAT, shared_table.ptr[slot_idx * 3 + 1], num_parameters * slot_size * slot_idx,
      num_parameters * slot_size, MPI_FLOAT, shared_slots.win);
    for(int i=0; i<slot_size * num_parameters; ++i){
      r = ptr[i];
      // printf("%f\n", r);
      outfile.write((char*)&r, sizeof(float));
    }
    outfile.close();

  }

  delete files;
  finish(world_rank, shared_table, shared_slots, shared_file_table);

}