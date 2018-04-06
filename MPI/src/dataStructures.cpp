#include <dataStructures.hpp>
#define NUM_LOCK 25

workQueueList globalWorkQueueList;
workQueueList globalReducerQueueList;
std::vector<countTable> countTableList;
int rank;

omp_lock_t qListLock, fileCountLock;
omp_lock_t qLocks[NUM_LOCK];
omp_lock_t qRLocks[NUM_LOCK];
omp_lock_t readerFinishLock;
omp_lock_t mapperFinishLock;
omp_lock_t arbitrateLock;
omp_lock_t countTableLock;

int readerThreadCount, mapperThreadCount, reducerThreadCount;
int fileCount = 1, currentMapperThreadQ = 0, mapperThreadFinishCount = 0, readerThreadFinishCount = 0;

int numProcessesDone(int* arr, int size)
{
	int sum = 0;
	for(int i = 0; i < size; i++)
	{
		sum += *arr;
	}

	return sum;
}

void CreateFileSyncThread(int numP)
{
	int* buf = (int*)calloc(sizeof(int), numP);
	MPI_Request* request = (MPI_Request*) malloc(sizeof(MPI_Request) * numP);
	MPI_Status fileNumStatus;
	int valueToSend = -1;

	int reqNum = -1;

	for(int i = 0; i < numP; i++)
	{
		MPI_Irecv(buf + i, 1, MPI_INT, i, ALL_FILES_READ_TAG, MPI_COMM_WORLD, request + i);
	}

	while(numProcessesDone(buf, numP) != numP)
	{
		MPI_Recv(&reqNum, 1, MPI_INT, MPI_ANY_SOURCE, GET_FILE_NUM_TAG, MPI_COMM_WORLD, &fileNumStatus);
	
		omp_set_lock(&fileCountLock);

		if(fileCount <= 20)
		{
			valueToSend = fileCount;
			fileCount++;
		}
		else
		{
			valueToSend = -1;
		}

		omp_unset_lock(&fileCountLock);


		MPI_Send(&valueToSend, 1, MPI_INT, reqNum, GET_FILE_NUM_TAG, MPI_COMM_WORLD);

	}
	
	std::cout << "All Processess are done asking for new files" << std::endl;
	free(buf);
	free(request);
}

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
	int fileNum;
	MPI_Status fileNumStatus;

	MPI_Send(&rank, 1, MPI_INT, 0, GET_FILE_NUM_TAG, MPI_COMM_WORLD);
	MPI_Recv(&fileNum, 1, MPI_INT, 0, GET_FILE_NUM_TAG, MPI_COMM_WORLD, &fileNumStatus);

	if(fileNum != -1)
	{
		returnValue = std::to_string((long long int)fileNum) + ".txt";
	}
	else
	{
		returnValue = "";
	}


	return returnValue;
}

void initializeWQStructures(int rnk, int readerThreads, int mapperThreads, int reducerThreads)
{
	rank = rnk;
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
	MPI_Status fileNumStatus;
	int fileNum;

	omp_set_lock(&readerFinishLock);

	readerThreadFinishCount++;
	
	omp_unset_lock(&readerFinishLock);

	int done = 1;
	MPI_Send(&done, 1, MPI_INT, 0, ALL_FILES_READ_TAG, MPI_COMM_WORLD);

	// Perform a read request to break the main rank file distro thread while loop
	MPI_Send(&rank, 1, MPI_INT, 0, GET_FILE_NUM_TAG, MPI_COMM_WORLD);
	MPI_Recv(&fileNum, 1, MPI_INT, 0, GET_FILE_NUM_TAG, MPI_COMM_WORLD, &fileNumStatus);
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
