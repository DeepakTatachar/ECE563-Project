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

int main(int argc, char* argv[])
{
	workQueueListIterator wQ;

	// TODO Set number of threads from command line argument
	omp_set_num_threads(4);

        int maxThreads = omp_get_max_threads();
        int readerThreads = maxThreads / 2;
	int mapperThreads = maxThreads - readerThreads;

	// Create maximum thread number of reducers so the second parameter is maxThreads 
	initializeWQStructures(readerThreads, mapperThreads, maxThreads);

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
					spawnNewMapperThread(workQ, i);
				}
			}
			
		}

		#pragma omp barrier

		for(int i = 0; i < maxThreads; i++)
		{
			#pragma omp task
			{
				workQueue workQ = getReducerWQ(i);
				spawnNewReducerThread(workQ);
			}
		}

	}

	printf("Hello World!\n");
	return 0;
}
