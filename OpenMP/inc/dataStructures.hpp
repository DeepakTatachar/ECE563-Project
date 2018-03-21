#ifndef   DataStructures_H
#define   DataStructures_H

#include <list>
#include <string>
#include <queue>
#include <map>
#include <cstdlib>
#include <iostream>
#include <omp.h>

typedef struct wItem
{
	std::string word;
	int count;
} workItem;

typedef std::queue<workItem> workQueue;

typedef std::vector<workQueue> workQueueList;

typedef workQueueList::const_iterator workQueueListIterator;

// Each mapper thread creates a dictionary of word, count. 
// This is what is accessed by the reduce threads to reduce accrosss all the mapped items

typedef std::map<std::string, int> mappedDictionary;

// Since both the reader and mapper are accessing the workqueue.
// This is will be a region for contention/bottleneck
// Writing chinks of work items and reading chunks of workItems will reduce the bottleneck
void enqueueMapperChunk(int id, std::vector<workItem> wItems);

std::list<workItem> dequeueChunk(int id, int chunkSize);

void arbitrateWorkItems(std::vector<workItem> workItems);

std::string getNextSyncedFileName();

void initializeWQStructures(int mapperThreads);

workQueue getWQ(int i);

#endif
