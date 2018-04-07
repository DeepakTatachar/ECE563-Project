#include <stdio.h>
#include <omp.h>
#include <mpi.h>

#ifndef DataStructures_H 
#include <dataStructures.hpp>
#endif

#ifndef Reader_H
#include <reader.hpp>
#endif

#ifndef Mapper_H
#include <mapper.hpp>
#endif

#ifndef Reducer_H
#include <reducer.hpp>
#endif

#ifndef Writer_H
#include <writer.hpp>
#endif

int main(int argc, char* argv[])
{
	int numP, rank;

	workQueueListIterator wQ;

	// TODO Set number of threads from command line argument
	omp_set_num_threads(8);

        int maxThreads = omp_get_max_threads();
        int readerThreads = 2;
	int mapperThreads = 2;
	int reducerThreads = maxThreads; 

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numP);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


	MPI_Barrier(MPI_COMM_WORLD);

	// Create maximum thread number of reducers so the second parameter is maxThreads 
	initializeWQStructures(rank, numP, readerThreads, mapperThreads, reducerThreads);

	std::cout << " Rank process starting " << rank << std::endl;

	#pragma omp parallel
	{
		#pragma omp master
		{
			for(int i = 0; i < readerThreads; i++)
			{
				#pragma omp task
				spawnNewReaderThread();				
			}

			for(int i = 0; i < mapperThreads; i++)
			{
				#pragma omp task
				{
					workQueue workQ = getMapperWQ(i);
					spawnNewMapperThread(workQ, i, reducerThreads);
				}
			}
		
		}


		#pragma omp barrier

		#pragma omp master
		{
			for(int i = 0; i < reducerThreads; i++)
			{
				#pragma omp task
				{
				  	workQueue workQ = getReducerWQ(i);
				  	spawnNewReducerThread(i, workQ);
				}
			}

			#pragma omp taskwait
		}

	}

	// Write the results to the file
	writeFile("OutputFile" + std::to_string((long long int)rank) + ".txt");

	return 0;
}
