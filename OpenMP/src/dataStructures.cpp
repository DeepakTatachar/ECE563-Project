#include <dataStructures.hpp>
#define NUM_LOCK 50

workQueueList globalWorkQueueList;
omp_lock_t qListLock, fileCountLock;
omp_lock_t qLocks[NUM_LOCK];
int fileCount = 1;

workQueue getWQ(int i)
{
	std::cout << "Do not call this method unless you know what you are doing" << std::endl;
	return globalWorkQueueList[i];
}

std::string getNextSyncedFileName()
{
	std::string returnValue;
	omp_set_lock(&fileCountLock);
	
	if(fileCount < 20)
	{
		returnValue = fileCount + ".txt";
		fileCount++;
	}
	else
	{
		returnValue = "";
	}

	omp_unset_lock(&fileCountLock);
	return returnValue;
}

void initializeWQStructures(int mapperThreads)
{
	omp_init_lock(&qListLock);
	omp_init_lock(&fileCountLock);

	for(int i = 0; i < NUM_LOCK; i++)
	{
		omp_init_lock(&qLocks[i]);
	}

	for(int i = 0; i < mapperThreads; i++)
	{
		globalWorkQueueList.push_back(std::queue<workItem>());
	}
}

void enqueueMapperChunk(int id, std::vector<workItem> wItems)
{
	workQueue mapperQ = globalWorkQueueList[id];

	omp_set_lock(&qLocks[id]);

	for(std::vector<workItem>::iterator it = wItems.begin(); it != wItems.end(); ++it)
	{
		mapperQ.push(*it);
	}

    	omp_unset_lock(&qLocks[id]);
	
}

std::list<workItem> dequeueChunk(int id, int chunkSize)
{
	std::list<workItem> dequeuedItems;
	/*int i = chunkSize;
	
	while(i >= 0 && !wQ.empty())
	{
		// TODO accquire locks
		workItem temp = wQ.front();
		dequeuedItems.push_back(temp);
		i--;
	}*/

	return dequeuedItems;
}

// Decides which mapper gets which workitem
void arbitrateWorkItems(std::vector<workItem> workItems)
{
	workQueueListIterator wQ;
	workQueue minWorkQueue = globalWorkQueueList[0];
	int minQSize = -1, minPos = 0, i = 0;
	
	for(wQ = globalWorkQueueList.begin(); wQ != globalWorkQueueList.end(); ++wQ)
	{
		int tempSize = (*wQ).size();
		if(tempSize < minQSize)
		{
			tempSize = minQSize;
			minPos = i;
		}

		i++;	
	}

	// Lock here, mapperChunk synchronizes correctly
	enqueueMapperChunk(minPos, workItems);	
		
	// TODO 
	// Implement a round robin work distribution scheme to pump work items into work queues
	// Note current architecture has an arbiter for each reader task/thread.
}
