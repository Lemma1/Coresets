#ifndef WORKER_H
#define WORKER_H

#include "data.h"

#include <mpi.h>
#include <stdio.h>
#include <unordered_map>

struct Shared_mem_float{
  MPI_Win win;
  float *ptr;
};

class CSET_Worker{
public:
	int m_rank;
	int m_num_slots;
	int m_num_parameters;
  int m_slot_size;
  Shared_mem_int m_shared_table;
  Shared_mem_float m_shared_slots;
  CSET_Data *m_data;
  std::unordered_map<int, int> m_slot_info;

	CSET_Worker(const int &rank, const int &num_slots, 
              const int &num_parameters, const int& slot_size,
              Shared_mem_int shared_table,  
              Shared_mem_float shared_slots,
              CSET_Data *data);
	~CSET_Worker();
  int lock_all();
  int lock_file_table();
  int lock_slot_table();
  int unlock_file_table();
  int unlock_slot_table();
  int update_info();
  int edit_file_info(const int &file_idx, const int &status, 
                        const int& count);
  int edit_table_info(const int &slot_idx, const int &status, 
                        const int& rank, const int& level);
  int unlock_all();
  int get_slot_info();
  int evolve();
  std::pair<int, int> get_same_level_slots();
  std::pair<int, int> get_diff_level_slots();
  void print_slot_table();
  int retrive_slots(const std::pair<int, int>& slots);
  int merge_slots(std::pair<int, int> slots);
  bool able_to_finish();
};


#endif