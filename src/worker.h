#ifndef WORKER_H
#define WORKER_H

#include <mpi.h>

class CSET_Worker{
public:
	int m_rank;
	int m_cap;
	int m_num_parameter;
	MPI_Win m_win;
	CSET_Worker(int rank, int cap, int num_parameter, MPI_Win win);
	~CSET_Worker();
};


#endif