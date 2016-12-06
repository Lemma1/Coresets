#include "config.h"

#include <fstream>
#include <string>

int main(int argc, char** argv)
{
  ConfReader conf_reader = ConfReader("../config.conf", "GEN");
  int num_parameters = conf_reader.get_int("num_parameters");
  int num_data = conf_reader.get_int("num_data");
  std::string file_name = conf_reader.get_string("data_file_name");

  std::ofstream f;
  f.open(file_name + std::to_string(0), std::ios::out | std::ios::binary);

  float r;
  float upper = 100.0;
  for(int i=0; i < num_data * num_parameters; ++i){
    r = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/upper));
    printf("%f\n", r);
    f.write((char*)&r, sizeof(float));
  }

  f.close();
  return 0;
}