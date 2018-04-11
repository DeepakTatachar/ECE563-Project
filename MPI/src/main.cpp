#include <stdio.h>
#include <omp.h>
#include <mpi.h>

#ifndef DataStructures_H 
#include <dataStructures.hpp>
#endif

#ifndef FileSyncManager_H
#include <fileSyncManager.hpp>
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
	int numP, rank, provided;

	workQueueListIterator wQ;

	// TODO Set number of threads from command line argument
	omp_set_num_threads(6);

        int maxThreads = omp_get_max_threads();
        int readerThreads = 2;
	int mapperThreads = 2;
	int reducerThreads = 2;//maxThreads; 

	MPI_Init_thread( 0, 0, MPI_THREAD_MULTIPLE, &provided);
	MPI_Comm_size(MPI_COMM_WORLD, &numP);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int totalReducerThreads = reducerThreads * numP;

	MPI_Barrier(MPI_COMM_WORLD);

	// Create maximum thread number of reducers so the second parameter is maxThreads 
	initializeWQStructures(rank, numP, readerThreads, mapperThreads, reducerThreads);

	//std::cout << "Rank process starting " << rank << std::endl << "Max threads per node :" << maxThreads << std::endl;

	#pragma omp parallel
	{
		#pragma omp master
		{
			if(rank == 0)
			{
				#pragma omp task
				spawnNewFileManager(readerThreads * numP);
			}

			for(int i = 0; i < readerThreads; i++)
			{
				#pragma omp task
				spawnNewReaderThread(i);				
			}

			for(int i = 0; i < mapperThreads; i++)
			{
				#pragma omp task
				{
					workQueue workQ = getMapperWQ(i);
					spawnNewMapperThread(workQ, i, totalReducerThreads);
				}
			}
		

			for(int i = 0; i < reducerThreads; i++)
			{
				int globalRId = rank * reducerThreads + i;

				#pragma omp task
				spawnNewReducerThread(globalRId, numP * mapperThreads, mapperThreads);
			}

			#pragma omp taskwait
		}

	}

	// Write the results to the file
	writeFile("OutputFile" + std::to_string((long long int)rank) + ".txt");
	
	MPI_Finalize();
	return 0;
}
