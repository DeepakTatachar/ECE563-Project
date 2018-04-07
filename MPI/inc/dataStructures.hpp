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

typedef struct wItem
{
	std::string word;
	int count;

	wItem(std::string wrd, int cnt)
	{
		word = wrd;
		count = cnt;
	}

} workItem;


typedef struct reducerWItem
{
	char word[MAX_STR_SIZE];
	int count;

	reducerWItem(std::string wrd, int cnt)
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
} reducerWorkItem;

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

void enqueueReducerChunk(int reducerThreads, int hashValue, countTable localMap);

void sendWork(int globalRThreadID, countTable localMap);

std::vector<workItem> dequeueMapperChunk(int id, int chunkSize);

std::vector<workItem> dequeueReducerChunk(int id, int chunkSize);

void arbitrateWorkItems(std::vector<workItem> workItems);

std::string getNextSyncedFileName();

void initializeWQStructures(int rnk, int numProc,int readerThreads, int mapperThreads, int reducerThreads);

workQueue getMapperWQ(int i);

workQueue getReducerWQ(int i);

void readerFinshed();

int allReadersDone();

void mapperFinshed();

std::vector<countTable> getCountList();

void enqueueCountTable(countTable table);

int allMappersDone();

#endif
