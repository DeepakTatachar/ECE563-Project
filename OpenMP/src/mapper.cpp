#include <mapper.hpp>
#include <dataStructures.hpp>
#define reducerThreads 4

void spawnNewMapperThread(workQueue wQ, int mapperId)
{

        std::hash<std::string> hash;
	unsigned int hashV;
	//std::map<unsigned int, workQueue> Hashedmap;
	std::map<unsigned int, std::vector<workItem> > Hashedmap;
	omp_lock_t hLocks[reducerThreads];

	// Dequeue workItem from the workQ 
	// Combine the words and map to the correct reducer
	// To map the word to the right reducer we hash the input string to find the correct reducer
	
	std::vector<workItem> workChunk;
	std::vector<workItem> reducerChunk;

	for(int i = 0; i < reducerThreads; i++)
	{
		omp_init_lock(&hLocks[i]);
	}
	
	// Run until all the reader threads are done and make sure there is no work left in the queue
	while(!(allReadersDone() && workChunk.size() == 0))
	{
	  workChunk = dequeueMapperChunk(mapperId, CHUNK_SIZE);
	  
		for(std::vector<workItem>::iterator it = workChunk.begin() ; it != workChunk.end(); ++it)
		{
		  std::cout << "word in work queue: "<<it->word << " "<<it->count<<"\n";
		   hashV = hash(it->word);
		   hashV = hashV % 4;
		   std::cout << hashV << std::endl;
		   
		   omp_set_lock(&hLocks[hashV]);
		   
		   Hashedmap[hashV].push_back(*it);
		   
		   omp_unset_lock(&hLocks[hashV]);

		}

	}
	
	for(std::map<unsigned int, std::vector<workItem>>::const_iterator it = Hashedmap.begin(); it != Hashedmap.end(); ++it)
	{
	        //workQueue temp = it->second;
	        std::vector<workItem> temp = it->second;
	        enqueueReducerChunk(it->first,temp);
	        for(std::vector<workItem>::const_iterator it_0 = temp.begin(); it_0 != temp.end(); ++it_0)

		             std::cout << it->first << "word: "<< it_0->word << "count: " << it_0->count << std::endl;
		//print_queue(temp);
	  }

	mapperFinshed();

	return;
}

