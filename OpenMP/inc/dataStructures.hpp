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


// Each mapper thread has it own queue
void InitializeWQList(workQueueList wQList, int mapperThreads);

#endif
