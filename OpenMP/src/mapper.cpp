#include <mapper.hpp>

void spawnNewMapperThread(workQueue wQ, int mapperId)
{

	// Dequeue workItem from the workQ 
	// Combine the words and map to the correct reducer
	// To combine the words we hash the input string to find the correct location to update
	
	std::vector<workItem> workChunk = dequeueMapperChunk(mapperId, CHUNK_SIZE);

	// Run until all the reader threads are done
	while(!(allReadersDone() && workChunk.size() == 0))
	{
		for(std::vector<workItem>::iterator it = workChunk.begin() ; it != workChunk.end(); ++it)
		{
			std::cout << mapperId << " : " << it->word << ", "  << it->count << std::endl; 
		}

		workChunk = dequeueMapperChunk(mapperId, CHUNK_SIZE);
	}
	
	return;
}
