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

int main(int argc, char* argv[])
{
        workQueueList wQList;
	workQueueListIterator wQ;

        int maxThreads = omp_get_max_threads();
        int readerThreads = maxThreads / 2;
	int mapperThreads = maxThreads - readerThreads;

	// Each mapper threads gets its own work queue so we create mapperThread number of workQueues
	initializeWQList(wQList, mapperThreads);

	#pragma omp parallel
	{
		#pragma omp master
		{
			for(int i = 0; i < readerThreads; i++)
			{
				#pragma omp task
				spawnNewReaderThread(wQList);				
			}

			for(wQ = wQList.begin(); wQ != wQList.end(); ++wQ)
			{
				#pragma omp task
				spawnNewMapperThreads(*wQ);
			}			
		}

		#pragma omp barrier

		// Do reduce here
	}

	printf("Hello World!\n");
	return 0;
}
