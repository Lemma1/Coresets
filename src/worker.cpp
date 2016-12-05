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
      if (level <= 0){
        printf("Error! level can't be non-positive!\n");
        exit(-1);
      }
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
  MPI_Put(&(m_data -> m_shared_file_table.ptr[2*file_idx]), 2, MPI_INT, 
    0, 2*file_idx, 2, MPI_INT, m_data -> m_shared_file_table.win);
  return 0;
}


int CSET_Worker::edit_table_info(const int &slot_idx, const int &status, 
                    const int& rank, const int& level)
{
  if (status != -2) m_shared_table.ptr[3 * slot_idx] = status;
  if (rank != -2) m_shared_table.ptr[3 * slot_idx+1] = rank;
  if (level != -2) m_shared_table.ptr[3 * slot_idx+2] = level;
  MPI_Put(&m_shared_table.ptr[3 * slot_idx], 3, MPI_INT, 
    0, 3 * slot_idx, 3, MPI_INT, m_shared_table.win);
  return 0;
}

std::pair<int, int> CSET_Worker::get_same_level_slots()
{
  // printf("Entering get_same_level_slots\n");
  std::pair<int, int> output = std::pair<int, int>(-1, -1);
  int temp[2]; 
  // printf("size of m_slot_info %d\n", (int) m_slot_info.size());
  for (auto map_it = m_slot_info.begin(); map_it != m_slot_info.end(); ++map_it){
    // printf("Get same level slots (%d, %d)\n", map_it -> first, map_it -> second);
    if (map_it -> first > 0 && map_it -> second > 1){
      int counter = 0; 
      while(counter < 2){
        for(int i=0; i< m_num_slots; ++i){
          if (m_shared_table.ptr[3 * i + 2] == map_it -> first){
            temp[counter] = i;
            ++counter;
          }
        }
      }
      output.first = temp[0];
      output.second = temp[1];
      break;
    }
  }
  // printf("get_same_level_slots out put <%d, %d>\n", output.first, output.second);
  return output;
}

std::pair<int, int> CSET_Worker::get_diff_level_slots()
{
  // printf("Entering get_diff_level_slots\n");
  std::pair<int, int> output = std::pair<int, int>(-1, -1);
  int temp[2]; 
  int counter = 0;
  // printf("size of m_slot_info %d\n", (int) m_slot_info.size());
  for (auto map_it = m_slot_info.begin(); map_it != m_slot_info.end(); ++map_it){
    // printf("Get same level slots (%d, %d)\n", map_it -> first, map_it -> second);
    if (map_it -> first > 0 && map_it -> second > 0){   
      for(int i=0; i< m_num_slots; ++i){
        if (m_shared_table.ptr[3 * i + 2] == map_it -> first){
          temp[counter] = i;
          ++counter;
          break;
        }
      }
    }
    if (counter == 2) break;
  }
  if (counter == 2){
    output.first = temp[0];
    output.second = temp[1];    
  }
  // printf("get_diff_level_slots out put <%d, %d>\n", output.first, output.second);
  return output;
}



int CSET_Worker::retrive_slots(const std::pair<int, int>& slots)
{
  // printf("Entering retrive_slots\n");
  MPI_Get(&(m_shared_slots.ptr[m_num_parameters * m_slot_size * slots.first]), 
        m_num_parameters * m_slot_size, 
        MPI_FLOAT, 0, m_num_parameters * m_slot_size * slots.first,
        m_num_parameters * m_slot_size, MPI_FLOAT, m_shared_slots.win);
  MPI_Get(&(m_shared_slots.ptr[m_num_parameters * m_slot_size * slots.second]), 
        m_num_parameters * m_slot_size, 
        MPI_FLOAT, 0, m_num_parameters * m_slot_size * slots.second,
        m_num_parameters * m_slot_size, MPI_FLOAT, m_shared_slots.win); 
  // printf("Exit retrive_slots\n"); 
  return 0;
}

int CSET_Worker::merge_slots()
{
  return 0;
}

void CSET_Worker::print_slot_table()
{
  for(int i=0; i<m_num_slots; ++i){
    printf("Worker %d, slot %d, [%d, %d, %d]\n", 
      m_rank, i, m_shared_table.ptr[3 * i], m_shared_table.ptr[3 * i+1],
      m_shared_table.ptr[3 * i+2]);
  }
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
      edit_table_info(availabe_slot_idx, -1, -1, -1);
      printf("worker %d, now reading files using slot %d and file %d from %d.\n", 
              m_rank, availabe_slot_idx, availabe_file_idx, 
              m_data -> m_shared_file_table.ptr[2 * availabe_file_idx + 1]);
      unlock_all();

      m_data -> fill_slot(availabe_file_idx, m_shared_slots.ptr, availabe_slot_idx);

      lock_all();
      int counter = m_data -> m_shared_file_table.ptr[2 * availabe_file_idx + 1];
      edit_file_info(availabe_file_idx, 0, counter + m_slot_size);
      edit_table_info(availabe_slot_idx, 1, m_rank, 1);
      // print_slot_table();
      unlock_all();
      return 1;
  }

  std::pair <int, int> slots;
  if ((slots = get_same_level_slots()) != std::pair<int, int>(-1, -1)){
    int slot_level = m_shared_table.ptr[3*slots.first + 2];
    printf("worker %d, now merging SAME LEVEL data from slots %d and %d on level %d.\n", m_rank, slots.first, slots.second, slot_level);
    edit_table_info(slots.first, -1, -2, -2);
    edit_table_info(slots.second, -1, -2, -2);
    unlock_all();

    retrive_slots(slots);
    merge_slots();

    lock_all();
    edit_table_info(slots.first, 1, m_rank, slot_level+1);
    edit_table_info(slots.second, 0, -1, -1);    
    unlock_all();
    return 2;
  }

  if ((slots = get_diff_level_slots()) != std::pair<int, int>(-1, -1)){
    std::pair<int, int> slots_level;
    slots_level.first = m_shared_table.ptr[3*slots.first + 2];
    slots_level.second = m_shared_table.ptr[3*slots.second + 2];
    int max_level = std::max(slots_level.first, slots_level.second);
    printf("worker %d, now merging DIFF LEVEL data from slots %d on level %d and %d on level %d.\n", 
            m_rank, slots.first, slots_level.first, slots.second, slots_level.second);
    edit_table_info(slots.first, -1, -2, -2);
    edit_table_info(slots.second, -1, -2, -2);
    unlock_all();

    retrive_slots(slots);
    merge_slots();

    lock_all();
    edit_table_info(slots.first, 1, m_rank, max_level);
    edit_table_info(slots.second, 0, -1, -1);  
    unlock_all();
    return 3;
  }
  // printf("Worker %d did nothing\n", m_rank);
  if (able_to_finish()){
    unlock_all();
    return 0;
  }

  unlock_all();
  return -1;
}

bool CSET_Worker::able_to_finish()
{
  // print_slot_table();
  int active_sum = 0;
  for (auto map_it = m_slot_info.begin(); map_it != m_slot_info.end(); ++map_it){
    // printf("<%d, %d>\n", map_it->first, map_it -> second);
    if (map_it -> first > 0) active_sum += map_it -> second;
    if (active_sum >= 2) return false;
  }
  if (m_slot_info.find(-1) != m_slot_info.end()) return false;
  return true;
}