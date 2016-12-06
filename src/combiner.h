#ifndef COMBINER_H
#define COMBINER_H

#include <Eigen/Dense>
#include "redsvd.h"

#include <math.h>
#include <limits>
#include <random>
#include <stdlib.h>
#include <algorithm>
#include <unordered_set>


extern "C" int merge_slots_SVD(float *first_ptr, float *second_ptr,
            const int &slot_size, const int &num_parameters)
{
  Eigen::MatrixXf input(slot_size *2, num_parameters);
  for(int i=0; i<slot_size; ++i){
    for(int j=0; j<num_parameters; ++j){
      input(i, j) = first_ptr[i * num_parameters + j];
    }
  }
  for(int i=0; i<slot_size; ++i){
    for(int j=0; j<num_parameters; ++j){
      input(i + slot_size, j) = second_ptr[i * num_parameters + j];
    }
  }
  RedSVD::RedSVD<Eigen::MatrixXf> redsvd;
  redsvd.compute(input, slot_size);
  Eigen::MatrixXf u = redsvd.matrixU().block(0, 0, slot_size, slot_size);
  Eigen::VectorXf s = redsvd.singularValues();
  Eigen::MatrixXf v = redsvd.matrixV();
  float temp;
  for(int i=0; i<slot_size; ++i){
    for(int j=0; j<num_parameters; ++j){
      temp = 0.0;
      for(int m=0; m<slot_size; ++m){
        temp += u(i, m) * s(m) * v(j, m);
      }
      first_ptr[i * num_parameters + j] = temp;
    }
  }
  return 0;
}

extern "C" float distance(float *first_ptr, float *second_ptr, 
                      const int &num_parameters)
{
  float sqrt_sum = 0.0;
  for(int i=0; i<num_parameters; ++i){
    sqrt_sum += pow(first_ptr[i] - second_ptr[i],2);
  }
  return sqrt(sqrt_sum);
}

extern "C" int copy_slot(float *from_ptr, float *to_ptr, 
                      const int &num_parameters)
{
  for(int i=0; i<num_parameters; ++i){
    to_ptr[i] = from_ptr[i];
  }
  return 0;
}

extern "C" int find_nearest_idx(float *first_ptr, float *second_ptr, 
                      const int &slot_size,
                      const int &num_parameters, const int &target_idx,
                      const std::unordered_set<int> &record)
{
  float cur_best = std::numeric_limits<float>::infinity();
  int cur_idx = -1;
  float temp_dist;
  int temp_idx;
  float *temp_ptr;
  float *target_ptr = target_idx < slot_size ? 
                      &first_ptr[target_idx * num_parameters] :
                      &second_ptr[(target_idx - slot_size) * num_parameters];
  for(auto set_it = record.begin(); set_it != record.end(); ++set_it){
    temp_idx = *set_it;
    temp_ptr = temp_idx < slot_size ? 
                      &first_ptr[temp_idx * num_parameters] :
                      &second_ptr[(temp_idx - slot_size) * num_parameters];
    temp_dist = distance(target_ptr, temp_ptr, num_parameters);
    if (temp_dist < cur_best){
      cur_best = temp_dist;
      cur_idx = temp_idx;
    }
  }
  if (cur_idx == -1){
    printf("ERROR in find_nearest_idx\n");
    exit(-1);
  }
  return cur_idx;
}

extern "C" int merge_slots_ADS(float *first_ptr, float *second_ptr,
            const int &slot_size, const int &num_parameters)
{
  std::unordered_set<int> record = std::unordered_set<int>();
  for(int i=0; i<2*slot_size; ++i){
    record.insert(i);
  }
  std::unordered_set<int>::iterator set_it;
  std::vector<int> idx_recorder = std::vector<int>();
  int target_idx;
  for(int i=0; i<slot_size; ++i){
    set_it = record.begin();
    std::advance(set_it, rand() % record.size());
    target_idx = *set_it;
    // printf("Pushing %d\n", target_idx);
    idx_recorder.push_back(target_idx);
    record.erase(set_it);
    set_it = record.find(find_nearest_idx(first_ptr, second_ptr, 
                      slot_size, num_parameters, target_idx,
                      record));
    if (set_it == record.end()){
      printf("ERROR, something wrong in merge_slots_ADS\n");
      exit(-1);
    }
    // printf("deleting %d\n", *set_it);
    record.erase(set_it);
  }
  /* important */
  std::sort(idx_recorder.begin(), idx_recorder.end());
  /* important */
  int counter = 0;
  float *from_ptr;
  float *to_ptr;
  for (int slot_idx : idx_recorder){
    to_ptr = &first_ptr[counter * num_parameters];
    from_ptr = slot_idx < slot_size ? 
                    &first_ptr[slot_idx * num_parameters] :
                    &second_ptr[(slot_idx - slot_size) * num_parameters];
    copy_slot(from_ptr, to_ptr, num_parameters);
    ++counter;
  }
  return 0;  
}

#endif