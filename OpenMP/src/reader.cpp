#include <reader.hpp>

workQueueList wQList;

// Decides which mapper gets which workitem
void arbitrateWorkItems(std::list<workItem> workItems)
{
	if(wQList.empty())
	{
		printf("No work queues, are you sure everyting is okay?\n");
		return;
	}

	//TODO 
	// Implement a round robin work distribution scheme to pump work items into work queues
	// Note current architecture has an arbiter for each reader task/thread.
}

void spawnNewReaderThread(workQueueList wQList)
{
	//TODO
	// 1. Readfile until eof.
	// 2. Break the string into words
	// 3. Create a work item for each word
	// 4. Send a chunk of work items to arbitrate 
}



