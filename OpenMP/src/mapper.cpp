#include <mapper.hpp>
#include <dataStructures.hpp>
#include <unistd.h>

void spawnNewMapperThread(workQueue wQ, int mapperId, int reducerThreads, int CHUNK_SIZE)
{

        std::hash<std::string> hash;
	unsigned int hashValue, reducerThreadID;
	hashTable hashMap;

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
			reducerThreadID = hashValue % reducerThreads;	
			hashMap[reducerThreadID].push_back(*it);
		}

	}
	
	for(hashTable::const_iterator it = hashMap.begin(); it != hashMap.end(); ++it)
	{
	        std::vector<workItem> temp = it->second;
	        enqueueReducerChunk(it->first,temp);
	}

	mapperFinshed();

	return;
}

