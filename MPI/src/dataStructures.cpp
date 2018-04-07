#include <dataStructures.hpp>
#define NUM_LOCK 25
#define NUM_FILES 20

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
}

MPI_Datatype workItemType;
MPI_Aint disp[2] = { offsetof( reducerWorkItem, word), offsetof( reducerWorkItem, count) };
MPI_Datatype type[2] = { MPI_CHAR, MPI_INT };
int blocklen[2] = { 100, 1 };

void sendWork(int globalRThreadID, countTable localMap)
{  
  int processNum = globalRThreadID/reducerThreadCount;
  MPI_Request Req;
  int index = 0;
  if(localMap.size() == 0)
    {
      MPI_Isend(&index, 1, MPI_INT, processNum, globalRThreadID, MPI_COMM_WORLD, &Req);
      std::cout <<" No work for reducer ID "<< globalRThreadID << std::endl;
    }
  else
    {
      countTable::iterator itr1;

      //Display for testing.
	for(itr1 = localMap.begin(); itr1 != localMap.end(); itr1++)
	      {
		std::cout << "Global Reducer ID: " << globalRThreadID << " Word: " << itr1->first << " Word count: " << itr1->second << std::endl;
	      }
	//Display for testing ends.

	reducerWorkItem* structArray = (reducerWorkItem*)malloc(sizeof(reducerWorkItem) * localMap.size());
	
	for(countTable::iterator it = localMap.begin(); it != localMap.end(); ++it)
	{
		structArray[index++] = reducerWorkItem(it->first, it->second);
	}

	//Display for testing.
	for(int i = 0; i < index; i++)
	  {
	    std::cout << "Value in the struct array: "<< structArray[i].word << structArray[i].count << std::endl; 
	  }
	//Display for testing ends.

	MPI_Isend(&index, 1, MPI_INT, processNum, globalRThreadID, MPI_COMM_WORLD, &Req);
	MPI_Isend(structArray, localMap.size(), workItemType, processNum, globalRThreadID, MPI_COMM_WORLD, &Req);
    }
}
/*
void enqueueReducerChunk(int reducerThreads, int hashValue, countTable  localMap)
{
	int local_id = hashValue % reducerThreads;
	int node_id = hashValue / numP;

	countTable::iterator itr1;
	for(itr1 = localMap.begin(); itr1 != localMap.end(); itr1++)
	      {
		std::cout << "Local Reducer ID: " << local_id << " Node ID: " << node_id << " Word: " << itr1->first << " Word count: " << itr1->second << std::endl;
	      }
	
	
	omp_set_lock(&qRLocks[local_id]);

	for(std::vector<workItem>::iterator it = wItems.begin(); it != wItems.end(); ++it)
	{
		globalReducerQueueList[id].push(*it);
	}

    	omp_unset_lock(&qRLocks[id]);
	
}
*/
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
