#ifndef   DataStructures_H
#define   DataStructures_H

#include <list>
#include <string>
#include <queue>
#include <map>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <omp.h>
#include <mpi.h>
#define CHUNK_SIZE 10
#define MAX_STR_SIZE 100
#define NUM_LOCK 25
#define NUM_FILES 20
#define FILE_SYNC_TAG 200

typedef struct wItem
{
	char word[MAX_STR_SIZE];
	int count;

	wItem(std::string wrd, int cnt)
	{
		int i;
		int maxLength = MAX_STR_SIZE;

		if(wrd.length() < maxLength)
		{
			maxLength = wrd.length();
		}

		for(i = 0; i < maxLength; i++)
		{
			word[i] = wrd[i];
		}

		for(; i < MAX_STR_SIZE - 1; i++)
		{
			word[i] = ' ';
		}

		word[i] = '\0';
		count = cnt;
	}
} workItem;

typedef std::queue<workItem> workQueue;

typedef std::vector<workQueue> workQueueList;

typedef workQueueList::const_iterator workQueueListIterator;

//typedef std::map<unsigned int, std::vector<workItem>> hashTable;

typedef std::map<std::string, int> countTable;

typedef std::map<unsigned int, countTable> hashedDict;

// Each mapper thread creates a dictionary of word, count. 
// This is what is accessed by the reduce threads to reduce accrosss all the mapped items

typedef std::map<std::string, int> mappedDictionary;

// Since both the reader and mapper are accessing the workqueue.
// This is will be a region for contention/bottleneck
// Writing chinks of work items and reading chunks of workItems will reduce the bottleneck
void enqueueMapperChunk(int id, std::vector<workItem> wItems);

void sendWork(int globalRThreadID, countTable localMap);

std::vector<workItem> dequeueMapperChunk(int id, int chunkSize);

void arbitrateWorkItems(std::vector<workItem> workItems);

std::string getNextSyncedFileName(int localReaderThreadId);

void initializeWQStructures(int rnk, int numProc,int readerThreads, int mapperThreads, int reducerThreads);

workQueue getMapperWQ(int i);

void readerFinshed();

int allReadersDone();

void mapperFinshed();

std::vector<countTable> getCountList();

void enqueueCountTable(countTable table);

int allMappersDone();

#endif
