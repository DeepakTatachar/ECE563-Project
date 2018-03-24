#include <reducer.hpp>

void spawnNewReducerThread(int reducerId, workQueue reducerWQ)
{
	std::map<std::string, int> wordCount;

	// Dequeue workItem from the workQ 
	
	std::vector<workItem> workChunk = dequeueReducerChunk(reducerId, CHUNK_SIZE);

	// Run until all the mapper threads are done and make sure there is no work left in the queue
	while(!(allMappersDone() && workChunk.size() == 0))
	{
		for(std::vector<workItem>::iterator it = workChunk.begin() ; it != workChunk.end(); ++it)
		{
			wordCount[it->word] += it->count;
		}

		workChunk = dequeueReducerChunk(reducerId, CHUNK_SIZE);
	}

	for (std::map<std::string, int>::iterator it = wordCount.begin(); it != wordCount.end(); ++it)
	{
		std::cout << it->first << ", " << it->second << std::endl;
	}
	
	return;
}
