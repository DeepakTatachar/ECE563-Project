#include <mapper.hpp>

void spawnNewMapperThread(workQueue wQ, int mapperId)
{

	// Dequeue workItem from the workQ 
	// Combine the words and map to the correct reducer
	// To combine the words we hash the input string to find the correct location to update
	
	std::vector<workItem> workChunk = dequeueMapperChunk(mapperId, CHUNK_SIZE);

	// Run until all the reader threads are done
	while(mapperRun())
	{
		for(int i = 0; i < CHUNK_SIZE; i++)
		{
		}

		workChunk = dequeueMapperChunk(mapperId, CHUNK_SIZE);
	}
	
	return;
}
