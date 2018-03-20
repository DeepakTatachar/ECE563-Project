#ifndef   DataStructures_H
#define   DataStructures_H

#include <list>
#include <string>
#include <queue>

struct workItem
{
	std::string word;
	int count;
};

typedef std::queue<workItem> workQueue;

typedef std::list<workQueue> workQueueList;

typedef workQueueList::const_iterator workQueueListIterator;


// Each mapper thread has it own queue
void initializeWQList(workQueueList wQList, int mapperThreads);

// Since both the reader and mapper are accessing the workqueue.
// This is will be a region for contention/bottleneck
// Writing chinks of work items and reading chunks of workItems will reduce the bottleneck
void enqueueChunk(workQueue wQ, std::list<workItem> wItems);

std::list<workItem> dequeueChunk(workQueue wQ, int chunkSize);

#endif
