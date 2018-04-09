#include <dataStructures.hpp>

workQueueList globalWorkQueueList;
workQueueList globalReducerQueueList;
std::vector<countTable> countTableList;

omp_lock_t qListLock, fileCountLock;
omp_lock_t readerFinishLock;
omp_lock_t mapperFinishLock;
omp_lock_t arbitrateLock;
omp_lock_t countTableLock;

int readerThreadCount, mapperThreadCount, reducerThreadCount, rank, numP;
int fileCount = 1, currentMapperThreadQ = 0, mapperThreadFinishCount = 0, readerThreadFinishCount = 0;

MPI_Datatype workItemType;
MPI_Aint disp[2] = { offsetof( workItem, word), offsetof( workItem, count) };
MPI_Datatype type[2] = { MPI_CHAR, MPI_INT };
int blocklen[2] = { MAX_STR_SIZE, 1 };

workQueue getMapperWQ(int i)
{
	omp_set_lock(&qListLock);

	workQueue value = globalWorkQueueList[i];

	omp_unset_lock(&qListLock);

	return value;
}

std::string getNextSyncedFileName()
{
	std::string returnValue;
	omp_set_lock(&fileCountLock);
	int maxFileNum = ((rank + 1) * NUM_FILES) / numP;
	
	if(fileCount < maxFileNum)
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

void initializeWQStructures(int rnk, int numProc, int readerThreads, int mapperThreads, int reducerThreads)
{
	rank = rnk;
	numP = numProc;
	readerThreadCount = readerThreads;
	mapperThreadCount = mapperThreads;
	reducerThreadCount = reducerThreads;

	omp_init_lock(&qListLock);
	omp_init_lock(&fileCountLock);
	omp_init_lock(&readerFinishLock);
	omp_init_lock(&arbitrateLock);
	omp_init_lock(&mapperFinishLock);
	omp_init_lock(&countTableLock);


	for(int i = 0; i < mapperThreads; i++)
	{
		globalWorkQueueList.push_back(std::queue<workItem>());
	}

	for(int i = 0; i < reducerThreads; i++)
	{
		globalReducerQueueList.push_back(std::queue<workItem>());
	}

	fileCount = rank * (NUM_FILES / numP);

	MPI_Type_create_struct(2, blocklen, disp, type, &workItemType);
	MPI_Type_commit(&workItemType);
}

void sendWork(int globalRThreadID, countTable localMap)
{
	int processNum = globalRThreadID / reducerThreadCount;
	MPI_Request Req;
	int index = 0;

	std::cout << rank << " sending to : " << processNum << std::endl;

	workItem* structArray = (workItem*)malloc(sizeof(workItem) * localMap.size());

	for(countTable::iterator it = localMap.begin(); it != localMap.end(); ++it)
	{
		structArray[index++] = workItem(it->first, it->second);
	}

	MPI_Isend(structArray, localMap.size(), workItemType, processNum, globalRThreadID, MPI_COMM_WORLD, &Req);
}

void enqueueMapperChunk(int id, std::vector<workItem> wItems)
{

	omp_set_lock(&qListLock);
	
	for(std::vector<workItem>::iterator it = wItems.begin(); it != wItems.end(); ++it)
	{
		globalWorkQueueList[id].push(*it);
	}

    	omp_unset_lock(&qListLock);
	
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

	if(mapperThreadFinishCount == mapperThreadCount)
	{
		std::cout << "All Mappers are done on this node" << std::endl;
	}
	
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

std::vector<workItem> dequeueMapperChunk(int id, int chunkSize)
{
	std::vector<workItem> workChunk;

	if(globalWorkQueueList[id].size() == 0)
	{
		return workChunk;
	}
	
	omp_set_lock(&qListLock);

	int i = chunkSize;
	while(i-- > 0 && globalWorkQueueList[id].size() != 0)
	{
		workChunk.push_back(globalWorkQueueList[id].front());
		globalWorkQueueList[id].pop();
	}

	omp_unset_lock(&qListLock);

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
