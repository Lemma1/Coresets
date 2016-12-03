#include "worker.h"

CSET_Worker::CSET_Worker(int rank, int cap, int num_parameter, MPI_Win win)
{
	m_rank = rank;
	m_cap = cap;
	m_num_parameter = num_parameter;
	m_win = win;
}

CSET_Worker::~CSET_Worker()
{

}

