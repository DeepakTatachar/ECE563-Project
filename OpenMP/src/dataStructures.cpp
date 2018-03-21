#include <dataStructures.hpp>

void initializeWQList(workQueueList wQList, int mapperThreads)
{
	// TODO
	// For each mapperThreads create a new queue and add to the workQueueList
}

void enqueueChunk(workQueue wQ, std::list<workItem> wItems)
{
	// TODO
}

std::list<workItem> dequeueChunk(workQueue wQ, int chunkSize)
{
	std::list<workItem> dequeuedItems;
	int i = chunkSize;
	
	while(i >= 0 && !wQ.empty())
	{
		// TODO accquire locks
		workItem temp = wQ.front();
		dequeuedItems.push_back(temp);
		i--;
	}

	return dequeuedItems;
}
