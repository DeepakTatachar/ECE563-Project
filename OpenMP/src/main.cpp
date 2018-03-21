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

int fileCount = 0;

std::string getNextSyncedFileName(int threads, int nodes = 1)
{
	std::string returnValue ("1.txt");

	// TODO comeup with a way to do this
	// Since all the files are numbered just increment fileCount and return fileName
	return returnValue;
}

int main(int argc, char* argv[])
{
        workQueueList mapperWQList;
	workQueueListIterator wQ;

	// TODO Set number of threads from command line argument
	omp_set_num_threads(2);

        int maxThreads = omp_get_max_threads();
        int readerThreads = maxThreads / 2;
	int mapperThreads = maxThreads - readerThreads;


	std::vector<mappedDictionary> dictArray;

	// Each mapper threads gets its own work queue so we create mapperThread number of workQueues
	initializeWQList(mapperWQList, mapperThreads);

	#pragma omp parallel
	{
		#pragma omp master
		{
			
			for(int i = 0; i < readerThreads; i++)
			{
				std::string fileName = getNextSyncedFileName(readerThreads);
 
				#pragma omp task
				spawnNewReaderThread(mapperWQList, fileName);				
			}

			int i = 0;
			for(wQ = mapperWQList.begin(); wQ != mapperWQList.end(); ++wQ)
			{
				#pragma omp task
				dictArray[i++] = spawnNewMapperThreads(*wQ);
			}			
		}

		#pragma omp barrier
		// How many reducer threads?
		// Do reduce here

		#pragma omp master
		{
			// TODO spwan reduce threads
		}

	}

	printf("Hello World!\n");
	return 0;
}
