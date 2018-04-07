#include <mapper.hpp>
#include <dataStructures.hpp>

void spawnNewMapperThread(workQueue wQ, int mapperId, int totalReducerThreads)
{

        std::hash<std::string> hash;
	unsigned int hashValue, reducerThreadID;
	hashedDict hashMap;
	countTable emptyMap;

	// Dequeue workItem from the workQ 
	// Combine the words and map to the correct reducer
	// To map the word to the right reducer we hash the input string to find the correct reducer
	
	std::vector<workItem> workChunk;
	std::vector<workItem> reducerChunk;

	
	// Run until all the reader threads are done and make sure there is no work left in the queue
	while(!(allReadersDone() && workChunk.size() == 0))
	{
		workChunk = dequeueMapperChunk(mapperId, CHUNK_SIZE);
	  
		for(std::vector<workItem>::iterator it = workChunk.begin() ; it != workChunk.end(); ++it)
		{
			hashValue = hash(it->word);

			//reducerThreadID here would be IDs across all nodes i.e. more than 20 if number of nodes is more than 1.
			reducerThreadID = hashValue % totalReducerThreads;
			hashMap[reducerThreadID][it->word] += it->count;
		}

	}


	for(int i = 0; i < totalReducerThreads; i++)
	{
		// Check if we need to send stuff to a particular reducer. If not send emptyMap;
		if(hashMap.find(i) == hashMap.end())
		{
			sendWork(i, emptyMap);
		}
		else
		{
			sendWork(i, hashMap.at(i));
		}
	}

	mapperFinshed();
}

