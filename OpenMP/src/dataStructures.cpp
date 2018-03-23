#include <dataStructures.hpp>
#define NUM_LOCK 50

workQueueList globalWorkQueueList;
workQueueList globalReducerQueueList;
omp_lock_t qListLock, fileCountLock;
omp_lock_t qLocks[NUM_LOCK];
omp_lock_t readerFinishLock;
int fileCount = 1;
int readerThreadFinishCount = 0;
int readerThreadsNum;

workQueue getMapperWQ(int i)
{
	std::cout << "Do not call GetMapperWQ method unless you know what you are doing" << std::endl;
	return globalWorkQueueList[i];
}

workQueue getReducerWQ(int i)
{
	std::cout << "Do not call GetReducerWQ method unless you know what you are doing" << std::endl;
	return globalReducerQueueList[i];
}

std::string getNextSyncedFileName()
{
	std::string returnValue;
	omp_set_lock(&fileCountLock);
	
	if(fileCount < 20)
	{
		returnValue = std::to_string((long long int)fileCount) + ".txt";
		fileCount++;
	}
	else
	{
		returnValue = "";
	}

	omp_unset_lock(&fileCountLock);
	return returnValue;
}

void initializeWQStructures(int readerThreads, int mapperThreads, int reducerThreads)
{
	readerThreadsNum = readerThreads;
	omp_init_lock(&qListLock);
	omp_init_lock(&fileCountLock);
	omp_init_lock(&readerFinishLock);

	for(int i = 0; i < NUM_LOCK; i++)
	{
		omp_init_lock(&qLocks[i]);
	}

	for(int i = 0; i < mapperThreads; i++)
	{
		globalWorkQueueList.push_back(std::queue<workItem>());
	}

	for(int i = 0; i < reducerThreads; i++)
	{
		globalReducerQueueList.push_back(std::queue<workItem>());
	}
}

void enqueueMapperChunk(int id, std::vector<workItem> wItems)
{
	omp_set_lock(&qLocks[id]);

	
	for(std::vector<workItem>::iterator it = wItems.begin(); it != wItems.end(); ++it)
	{
		globalWorkQueueList[id].push(*it);
	}

    	omp_unset_lock(&qLocks[id]);
	
}

void readerFinshed()
{
	omp_set_lock(&readerFinishLock);

	readerThreadFinishCount++;
	
	omp_unset_lock(&readerFinishLock);
}

int allReadersDone()
{
	// Note reading requires no lock

	if(readerThreadFinishCount == readerThreadsNum)
		return 1;
	else
		return 0;
}

std::vector<workItem> dequeueMapperChunk(int id, int chunkSize)
{
	std::vector<workItem> workChunk;

	if(globalWorkQueueList[id].size() == 0)
	{
		return workChunk;
	}
	
	omp_set_lock(&qLocks[id]);

	int i = chunkSize;
	while(i-- > 0 && globalWorkQueueList[id].size() != 0)
	{
		workChunk.push_back(globalWorkQueueList[id].front());
		globalWorkQueueList[id].pop();
	}

	omp_unset_lock(&qLocks[id]);

	return workChunk;
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
		if(tempSize <= minQSize)
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
