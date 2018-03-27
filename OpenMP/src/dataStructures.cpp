#include <dataStructures.hpp>
#define NUM_LOCK 25

workQueueList globalWorkQueueList;
workQueueList globalReducerQueueList;

omp_lock_t qListLock, fileCountLock;
omp_lock_t qLocks[NUM_LOCK];
omp_lock_t qRLocks[NUM_LOCK];
omp_lock_t readerFinishLock;
omp_lock_t mapperFinishLock;
omp_lock_t arbitrateLock;

int readerThreadCount, mapperThreadCount, reducerThreadCount;
int fileCount = 1, currentMapperThreadQ = 0, mapperThreadFinishCount = 0, readerThreadFinishCount = 0;

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
	
	if(fileCount <= 2)
	{
		returnValue = std::to_string((long long int)fileCount) + ".txt";
		std::cout << returnValue << std::endl;
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
	readerThreadCount = readerThreads;
	mapperThreadCount = mapperThreads;
	reducerThreadCount = reducerThreads;

	omp_init_lock(&qListLock);
	omp_init_lock(&fileCountLock);
	omp_init_lock(&readerFinishLock);
	omp_init_lock(&arbitrateLock);
	omp_init_lock(&mapperFinishLock);

	for(int i = 0; i < NUM_LOCK; i++)
	{
		omp_init_lock(&qLocks[i]);
	}

	for(int i = 0; i < NUM_LOCK; i++)
	{
		omp_init_lock(&qRLocks[i]);
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

void enqueueReducerChunk(int hashValue, std::vector<workItem> wItems)
{
	int id = hashValue % reducerThreadCount;

	omp_set_lock(&qRLocks[id]);

	for(std::vector<workItem>::iterator it = wItems.begin(); it != wItems.end(); ++it)
	{
		globalReducerQueueList[id].push(*it);
	}

    	omp_unset_lock(&qRLocks[id]);
	
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


void mapperFinshed()
{
	omp_set_lock(&readerFinishLock);

	mapperThreadFinishCount++;
	
	omp_unset_lock(&readerFinishLock);
}

int allReadersDone()
{
	// Note reading requires no lock

	if(readerThreadFinishCount == readerThreadCount)
		return 1;
	else
		return 0;
}

int allMappersDone()
{
	// Note reading requires no lock

	if(mapperThreadFinishCount == mapperThreadCount)
		return 1;
	else
		return 0;
}

std::vector<workItem> dequeueReducerChunk(int id, int chunkSize)
{
	std::vector<workItem> workChunk;

	if(globalReducerQueueList[id].size() == 0)
	{
		return workChunk;
	}
	
	omp_set_lock(&qRLocks[id]);

	int i = chunkSize;
	while(i-- > 0 && globalReducerQueueList[id].size() != 0)
	{
		workChunk.push_back(globalReducerQueueList[id].front());
		globalReducerQueueList[id].pop();
	}

	omp_unset_lock(&qRLocks[id]);

	return workChunk;
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
	int pos;
	omp_set_lock(&arbitrateLock);
	
	pos = currentMapperThreadQ++ % mapperThreadCount;

	omp_unset_lock(&arbitrateLock);

	// Lock here, mapperChunk synchronizes correctly
	enqueueMapperChunk(pos, workItems);	
}

void print_queue(std::queue<workItem> q)
{
  while (!q.empty())
  {
    std::cout << q.front().word << " " << q.front().count;
    q.pop();
  }
  std::cout << std::endl;
}
