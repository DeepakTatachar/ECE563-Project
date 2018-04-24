#include <stdio.h>
#include <omp.h>

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

//Uncomment the line below to simulate serial implementation
//#define Serial

int main(int argc, char* argv[])
{
	workQueueListIterator wQ;

        int readerThreads = atoi(argv[1]);
	int mapperThreads = atoi(argv[2]);
	int reducerThreads = atoi(argv[3]); 
	int numPasses = atoi(argv[4]);
	int chunkSize = atoi(argv[5]);
	int reducerChunkSize = atoi(argv[6]);

        int maxThreads = readerThreads + mapperThreads;

	omp_set_num_threads(maxThreads);

	// Create maximum thread number of reducers so the second parameter is maxThreads 
	initializeWQStructures(readerThreads, mapperThreads, reducerThreads, numPasses);

	double time = -omp_get_wtime();

	#pragma omp parallel
	{
		#pragma omp master
		{	
			for(int i = 0; i < readerThreads; i++)
			{
				#pragma omp task
				spawnNewReaderThread();				
			}

		#ifdef Serial

		}

		#pragma omp taskwait

		#pragma omp master
		{

		#endif

			for(int i = 0; i < mapperThreads; i++)
			{
				#pragma omp task
				{
					workQueue workQ = getMapperWQ(i);
					spawnNewMapperThread(workQ, i, reducerThreads, chunkSize);
				}
			}
			
		}

		#pragma omp taskwait

		#pragma omp master
		{
			for(int i = 0; i < reducerThreads; i++)
			{
				#pragma omp task
				{
				  	workQueue workQ = getReducerWQ(i);
				  	spawnNewReducerThread(i, workQ, reducerChunkSize);
				}
			}

			#pragma omp taskwait
		}

	}

	time += omp_get_wtime();

	std::cout << time << std::endl;

	// Write the results to the file
	writeFile("OutputFile.txt");

	return 0;
}
