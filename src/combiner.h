#ifndef COMBINER_H
#define COMBINER_H

#include <Eigen/Dense>
#include "redsvd.h"

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
  Eigen::VectorXf s = redsvd.singularValues();
  Eigen::MatrixXf v = redsvd.matrixV();
  float scalar;
  for(int i=0; i<slot_size; ++i){
    scalar = s(i);
    for(int j=0; j<num_parameters; ++j){
      first_ptr[i * num_parameters + j] = v(j, i) * scalar;
    }
  }

  return 0;
}

#endif