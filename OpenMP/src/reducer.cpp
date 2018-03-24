#include <reducer.hpp>

void spawnNewReducerThread(int reducerId, workQueue reducerWQ)
{
	// Dequeue workItem from the workQ 
	
	std::vector<workItem> workChunk = dequeueReducerChunk(reducerId, CHUNK_SIZE);

	// Run until all the mapper threads are done and make sure there is no work left in the queue
	while(!(allMappersDone() && workChunk.size() == 0))
	{
		for(std::vector<workItem>::iterator it = workChunk.begin() ; it != workChunk.end(); ++it)
		{
			std::cout << reducerId << " : " << it->word << ", "  << it->count << std::endl; 
		}

		workChunk = dequeueReducerChunk(reducerId, CHUNK_SIZE);
	}
	
	return;
}
