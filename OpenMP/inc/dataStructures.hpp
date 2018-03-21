#ifndef   DataStructures_H
#define   DataStructures_H

#include <list>
#include <string>
#include <queue>
#include <map>
#include <cstdlib>

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

// Each mapper thread has it own queue
void initializeWQList(workQueueList wQList, int mapperThreads);

// Since both the reader and mapper are accessing the workqueue.
// This is will be a region for contention/bottleneck
// Writing chinks of work items and reading chunks of workItems will reduce the bottleneck
void enqueueChunk(workQueue wQ, std::list<workItem> wItems);

std::list<workItem> dequeueChunk(workQueue wQ, int chunkSize);

#endif
