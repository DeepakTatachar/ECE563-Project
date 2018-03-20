#include <stdio.h>
#include <omp.h>

#ifndef DataStructures_H 
#include <dataStructures.hpp>
#endif

#ifndef Reader_H
#include <reader.hpp>
#endif


int main(int argc, char* argv[])
{
        workQueueList wQList;
        int maxThreads = omp_get_max_threads();
        int readerThreads = maxThreads / 2;
	int mapperThreads = maxThreads - readerThreads;

	InitializeWQList(wQList, mapperThreads);

	#pragma omp parallel
	{
		#pragma omp master
		{
			for(int i = 0; i < readerThreads; i++)
			{
				#pragma omp task
				spawnNewReaderThread(wQList);				
			}

			for(int i = 0; i < mapperThreads; i++)
			{
				//#pragma omp task
				//spawnNewMapperThread();				
			}			
		}
	}
	printf("Hello World!\n");
	return 0;
}
