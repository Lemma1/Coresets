#ifndef DATA_H
#define DATA_H

#include <stdio.h>
#include <mpi.h>

struct Shared_mem_int{
  MPI_Win win;
  int *ptr;
};

class CSET_Data{
public:
  int m_num_files;
  int m_num_data;
  int m_slot_size;
  int m_num_parameters;
  MPI_File *m_files;
  Shared_mem_int m_shared_file_table;
  CSET_Data *m_data;

  CSET_Data(const int &num_files, const int &num_data, 
            const int &slot_size, const int &num_parameters,
            MPI_File *files, Shared_mem_int shared_file_table);
  ~CSET_Data();
  int fill_slot(const int &file_idx, float *start_ptr, 
                const int &slot_idx);
  bool is_finished();
  int get_available();
};

#endif