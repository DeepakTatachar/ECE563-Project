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
        int readerThreads = 2;
	int mapperThreads = 2;
	int reducerThreads = maxThreads; 

	// Create maximum thread number of reducers so the second parameter is maxThreads 
	initializeWQStructures(readerThreads, mapperThreads, reducerThreads);

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

	std::vector<countTable> wordCountTable = getCountList();
	for(std::vector<countTable>::iterator it = wordCountTable.begin(); it != wordCountTable.end(); ++it)
	{
		countTable table = *it;
		for(countTable::const_iterator itr = table.begin(); itr != table.end(); ++itr)
		{
			std::cout << itr->first << " : " << itr->second << std::endl;
		}
	}

	return 0;
}
