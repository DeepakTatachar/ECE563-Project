#include <mapper.hpp>
#include <dataStructures.hpp>

void spawnNewMapperThread(workQueue wQ, int mapperId, int totalReducerThreads)
{

        std::hash<std::string> hash;
	unsigned int hashValue, reducerThreadID;
	hashedDict hashMap;
	countTable emptyMap;
	//hashTable hashMap;
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
	countTable::iterator itr1;
	hashedDict::iterator itr2;
	for(int i = 0; i < totalReducerThreads; i++)
	  {
	    if(hashMap.find(i) == hashMap.end())
	      {
		sendWork(i, emptyMap);
	      }
	    else
	      sendWork(i, hashMap.at(i));
	  }

	/*
	for(itr2 = hashMap.begin(); itr2 != hashMap.end(); itr2++)
	  {
	    //sends reducer thread ID and nested hash map to the reducer queue.
	    enqueueReducerChunk(8,itr2->first, itr2->second);

	    //for display purposes
	    for(itr1 = itr2->second.begin(); itr1 != itr2->second.end(); itr1++)
	      {
		std::cout << "Reducer ID: " << itr2->first << " Word: " << itr1->first << " Word count: " << itr1->second << std::endl;
	      }
	  }
 
	for(hashTable::const_iterator it = hashMap.begin(); it != hashMap.end(); ++it)
	{
	        std::vector<workItem> temp = it->second;
	        enqueueReducerChunk(it->first,temp);
	}
	*/
	mapperFinshed();

	return;
}

