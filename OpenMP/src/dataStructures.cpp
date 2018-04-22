#include <dataStructures.hpp>
#define NUM_LOCK 25

workQueueList globalWorkQueueList;
workQueueList globalReducerQueueList;
std::vector<countTable> countTableList;

omp_lock_t qListLock, fileCountLock;
omp_lock_t qMLocks[NUM_LOCK];
omp_lock_t qRLocks[NUM_LOCK];
omp_lock_t gRLock;
omp_lock_t readerFinishLock;
omp_lock_t mapperFinishLock;
omp_lock_t arbitrateLock;
omp_lock_t countTableLock;

int readerThreadCount, mapperThreadCount, reducerThreadCount, numPass;
int fileCount = 1, currentMapperThreadQ = 0, mapperThreadFinishCount = 0, readerThreadFinishCount = 0;
int numOfPasses = 0;

workQueue getMapperWQ(int i)
{

	omp_set_lock(&qListLock);

	workQueue value = globalWorkQueueList[i];

	omp_unset_lock(&qListLock);

	return value;
}

workQueue getReducerWQ(int i)
{
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
	else if(fileCount == 20 && numOfPasses < numPass)
	{
		fileCount = 1;
		returnValue = std::to_string((long long int)fileCount) + ".txt";
		numOfPasses++;
	}
	else
	{
		returnValue = "";
	}

	omp_unset_lock(&fileCountLock);
	return returnValue;
}

void initializeWQStructures(int readerThreads, int mapperThreads, int reducerThreads, int noPass)
{
	numPass = noPass;
	readerThreadCount = readerThreads;
	mapperThreadCount = mapperThreads;
	reducerThreadCount = reducerThreads;

	omp_init_lock(&qListLock);
	omp_init_lock(&fileCountLock);
	omp_init_lock(&readerFinishLock);
	omp_init_lock(&arbitrateLock);
	omp_init_lock(&mapperFinishLock);
	omp_init_lock(&countTableLock);
	omp_init_lock(&gRLock);

	for(int i = 0; i < NUM_LOCK; i++)
	{
		omp_init_lock(&qMLocks[i]);
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

	omp_set_lock(&gRLock);

	workQueue* rQ = &(globalReducerQueueList[id]);

	omp_unset_lock(&gRLock);

	omp_set_lock(&qRLocks[id]);

	for(std::vector<workItem>::iterator it = wItems.begin(); it != wItems.end(); ++it)
	{
		rQ->push(*it);
	}

    	omp_unset_lock(&qRLocks[id]);
	
}

void enqueueMapperChunk(int id, std::vector<workItem> wItems)
{

	omp_set_lock(&qListLock);

	workQueue* mapperQ = &(globalWorkQueueList[id]);

    	omp_unset_lock(&qListLock);
	
	omp_set_lock(&qMLocks[id]);

	for(std::vector<workItem>::iterator it = wItems.begin(); it != wItems.end(); ++it)
	{
		mapperQ->push(*it);
	}

    	omp_unset_lock(&qMLocks[id]);
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

	omp_unset_lock(&gRLock);

	workQueue* reducerQ = &(globalReducerQueueList[id]);

	omp_unset_lock(&gRLock);
	

	if(reducerQ->size() == 0 || reducerQ->size() < (unsigned int)chunkSize)
	{
		return workChunk;
	}

	omp_set_lock(&qRLocks[id]);

	int i = chunkSize;
	while(i-- > 0 && reducerQ->size() != 0)
	{
		workChunk.push_back(reducerQ->front());
		reducerQ->pop();
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
	
	omp_set_lock(&qListLock);

	workQueue* mapperQ = &(globalWorkQueueList[id]);

	omp_unset_lock(&qListLock);


	omp_set_lock(&qMLocks[id]);

	int i = chunkSize;
	while(i-- > 0 && mapperQ->size() != 0)
	{
		workChunk.push_back(mapperQ->front());
		mapperQ->pop();
	}

	omp_unset_lock(&qMLocks[id]);

	return workChunk;
}

void enqueueCountTable(countTable table)
{
	omp_set_lock(&countTableLock);

	countTableList.push_back(table);

	omp_unset_lock(&countTableLock);
}

std::vector<countTable> getCountList()
{
	return countTableList;
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
