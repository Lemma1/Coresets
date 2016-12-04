#include "worker.h"

CSET_Worker::CSET_Worker(const int &rank, const int &num_slots, 
                        const int &num_parameters, const int& slot_size,
                        Shared_mem_int shared_table,
                        Shared_mem_float shared_slots,
                        CSET_Data *data)
{
	m_rank = rank;
	m_num_slots = num_slots;
	m_num_parameters = num_parameters;
  m_slot_size = slot_size;
  m_shared_table = shared_table;
  m_shared_slots = shared_slots;
  m_data = data;
  std::unordered_map<int, int> m_slot_info = std::unordered_map<int, int>();
}

CSET_Worker::~CSET_Worker()
{
  m_slot_info.clear();
}

int CSET_Worker::lock_all()
{
  MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, m_shared_table.win);
  MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, m_data -> m_shared_file_table.win);
  return 0;
}

int CSET_Worker::update_info(){
  MPI_Get(m_shared_table.ptr, 3 * m_num_slots, MPI_INT, 0, 0,
          3 * m_num_slots, MPI_INT, m_shared_table.win);
  MPI_Get(m_data -> m_shared_file_table.ptr, 2 * m_data -> m_num_files, MPI_INT, 0, 0,
          2 * m_data -> m_num_files, MPI_INT, m_data -> m_shared_file_table.win);
  return 0;
}


int CSET_Worker::unlock_all()
{
  MPI_Win_unlock(0, m_shared_table.win);
  MPI_Win_unlock(0, m_data -> m_shared_file_table.win);
  return 0;
}

int CSET_Worker::get_slot_info()
{
  m_slot_info.clear();
  int status;
  // int rank;
  int level;
  for(int i=0; i<m_num_slots; ++i){
    status = m_shared_table.ptr[i * 3];
    level = m_shared_table.ptr[i * 3 + 2];
    if (status == -1 || status == 0){
      if (m_slot_info.find(status) == m_slot_info.end()){
        m_slot_info.insert(std::pair<int, int>(status, 1));
      }
      else{
        (m_slot_info.find(status)) -> second += 1;
      }
    }
    else{
      if (m_slot_info.find(level) == m_slot_info.end()){
        m_slot_info.insert(std::pair<int, int>(level, 1));
      }
      else{
        (m_slot_info.find(level)) -> second += 1;
      }      
    }
  }
  return 0;
}

int CSET_Worker::edit_file_info(const int &file_idx, const int &status, 
                        const int& count)
{
  if (status != -2) m_data -> m_shared_file_table.ptr[2*file_idx] = status;
  if (count != -2) m_data -> m_shared_file_table.ptr[2*file_idx + 1] = count;
  // MPI_Put(, int origin_count, MPI_Datatype
  //   origin_datatype, int target_rank, MPI_Aint target_disp,
  //   int target_count, MPI_Datatype target_datatype, MPI_Win win)
  MPI_Put(m_data -> m_shared_file_table.ptr, 2*m_data -> m_num_files, MPI_INT, 
    0, 0, 2*m_data -> m_num_files, MPI_INT, m_data -> m_shared_file_table.win);
  return 0;
}


int CSET_Worker::edit_table_info(const int &slot_idx, const int &status, 
                    const int& rank, const int& level)
{
  if (status != -2) m_shared_table.ptr[3 * slot_idx] = status;
  if (rank != -2) m_shared_table.ptr[3 * slot_idx+1] = rank;
  if (level != -2) m_shared_table.ptr[3 * slot_idx+2] = level;
  MPI_Put(m_shared_table.ptr, 3*m_num_slots, MPI_INT, 
    0, 0, 3*m_num_slots, MPI_INT, m_shared_table.win);
  return 0;
}

int CSET_Worker::evolve()
{
  lock_all();
  update_info();
  get_slot_info();
  int availabe_file_idx;
  if ((!m_data -> is_finished()) && 
      ((availabe_file_idx = m_data -> get_available()) != -1) &&
      m_slot_info.find(0) != m_slot_info.end()){
      

      int availabe_slot_idx;
      for(int i=0; i< m_num_slots; ++i){
        if (m_shared_table.ptr[3 * i] == 0){
          availabe_slot_idx = i;
          break;
        }
      }
      edit_file_info(availabe_file_idx, -1, -2);
      edit_table_info(availabe_slot_idx, -1, m_rank, 1);
      printf("worker %d, now reading files using slot %d and file %d from %d!\n", 
              m_rank, availabe_slot_idx, availabe_file_idx, 
              m_data -> m_shared_file_table.ptr[2 * availabe_file_idx + 1]);
      unlock_all();

      m_data -> fill_slot(availabe_file_idx, m_shared_slots.ptr, availabe_slot_idx);
      

      lock_all();
      int counter = m_data -> m_shared_file_table.ptr[2 * availabe_file_idx + 1];
      edit_file_info(availabe_file_idx, 0, counter + m_slot_size);
      edit_table_info(availabe_slot_idx, 1, m_rank, 1);
      unlock_all();
      return 1;
  }

  printf("Worker %d did nothing\n", m_rank);
  unlock_all();
  return 0;
}
