#include "data.h"

CSET_Data::CSET_Data(const int &num_files, const int &num_data, 
                      const int &slot_size, const int &num_parameters,
                      MPI_File *files, Shared_mem_int shared_file_table)
{
  m_num_data = num_data;
  m_num_files = num_files;
  m_slot_size = slot_size;
  m_num_parameters = num_parameters;
  m_files = files;
  m_shared_file_table = shared_file_table;
}

CSET_Data::~CSET_Data()
{

}

bool CSET_Data::is_finished()
{
  for (int i=0; i< m_num_files; ++i){
    if (m_num_data - m_shared_file_table.ptr[i * 2 + 1] >= m_slot_size){
      // printf("is_finished returning false\n");
      return false;
    }
  }
  // printf("is_finished returning true\n");
  return true;
}

int CSET_Data::get_available()
{
  for (int i=0; i< m_num_files; ++i){
    if (m_num_data - m_shared_file_table.ptr[i * 2 + 1] >= m_slot_size){
      if (m_shared_file_table.ptr[i * 2] == 0) {
        // printf("get_available returning %d\n", i);
        return i;
      }
    }
  }
  // printf("get_available returning -1\n");
  return -1;
}


int CSET_Data::fill_slot(const int &file_idx, float *start_ptr, 
                        const int &num_slot)
{
  MPI_Status status;
  MPI_File_read_at(m_files[file_idx], m_shared_file_table.ptr[file_idx * 2 + 1], 
                  &(start_ptr[num_slot * m_num_parameters * m_slot_size]), 
                  m_num_parameters * m_slot_size, MPI_FLOAT, &status);  
}



