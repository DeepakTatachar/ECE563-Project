#include <dataStructures.hpp>

workQueueList globalWorkQueueList;
workQueueList globalReducerQueueList;
std::vector<countTable> countTableList;

omp_lock_t qListLock, fileCountLock;
omp_lock_t qLocks[NUM_LOCK];
omp_lock_t qRLocks[NUM_LOCK];
omp_lock_t readerFinishLock;
omp_lock_t mapperFinishLock;
omp_lock_t arbitrateLock;
omp_lock_t countTableLock;

int readerThreadCount, mapperThreadCount, reducerThreadCount, rank, numP;
int fileCount = 1, currentMapperThreadQ = 0, mapperThreadFinishCount = 0, readerThreadFinishCount = 0;

MPI_Datatype workItemType;
MPI_Aint disp[2] = { offsetof( reducerWorkItem, word), offsetof( reducerWorkItem, count) };
MPI_Datatype type[2] = { MPI_CHAR, MPI_INT };
int blocklen[2] = { MAX_STR_SIZE, 1 };

workQueue getMapperWQ(int i)
{
	//std::cout << "Do not call GetMapperWQ method unless you know what you are doing" << std::endl;
	return globalWorkQueueList[i];
}

workQueue getReducerWQ(int i)
{
	//std::cout << "Do not call GetReducerWQ method unless you know what you are doing" << std::endl;
	return globalReducerQueueList[i];
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

	fileCount = rank * (NUM_FILES / numP);

	MPI_Type_create_struct(2, blocklen, disp, type, &workItemType);
	MPI_Type_commit(&workItemType);
}

void sendWork(int globalRThreadID, countTable localMap)
{  
	int processNum = globalRThreadID / reducerThreadCount;
	MPI_Request Req;
	int index = 0;
	int data[2] = { 0, rank };
	std::cout << "Sending to : " << processNum << std::endl;

	if(localMap.size() == 0)
	{
		MPI_Send(data, 2, MPI_INT, processNum, globalRThreadID, MPI_COMM_WORLD);
		std::cout <<" No work for reducer ID "<< globalRThreadID << std::endl;
	}
	else
	{
		countTable::iterator itr1;

		/*//Display for testing.
		for(itr1 = localMap.begin(); itr1 != localMap.end(); itr1++)
		{
			std::cout << "Global Reducer ID: " << globalRThreadID << " Word: " << itr1->first << " Word count: " << itr1->second << std::endl;
		}
		//Display for testing ends.*/

		reducerWorkItem* structArray = (reducerWorkItem*)malloc(sizeof(reducerWorkItem) * localMap.size());

		for(countTable::iterator it = localMap.begin(); it != localMap.end(); ++it)
		{
			structArray[index++] = reducerWorkItem(it->first, it->second);
		}

		/*//Display for testing.
		for(int i = 0; i < index; i++)
		{
			std::cout << "Value in the struct array: "<< structArray[i].word << structArray[i].count << std::endl; 
		}
		//Display for testing ends.*/

		data[0] = index;
		MPI_Send(data, 2, MPI_INT, processNum, globalRThreadID, MPI_COMM_WORLD);
		MPI_Send(structArray, localMap.size(), workItemType, processNum, globalRThreadID, MPI_COMM_WORLD);
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
