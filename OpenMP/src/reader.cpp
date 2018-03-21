#include <reader.hpp>

workQueueList wQList;

// Decides which mapper gets which workitem
void arbitrateWorkItems(std::vector<workItem> workItems)
{
	if(wQList.empty())
	{
		printf("No work queues, are you sure everyting is okay?\n");
		return;
	}

	//TODO 
	// Implement a round robin work distribution scheme to pump work items into work queues
	// Note current architecture has an arbiter for each reader task/thread.
}

std::vector<workItem> createWorkItems(std::string line)
{
	std::vector<workItem> lineWorkItems;
	std::istringstream iss(line);

	do
	{
		std::string subs;
		iss >> subs;
		
		// Make sure to create a new workItem
		// Check if the string is not all whitespaces
		if(subs.find_first_not_of(' ') != std::string::npos)
		{
			workItem newWorkItem = {subs, 1};
			lineWorkItems.push_back(newWorkItem);
		}

	} while (iss);

	return lineWorkItems;
}

void spawnNewReaderThread(workQueueList wQList, std::string fileName)
{
	// 1. Readfile until eof.
	// 2. Break the string into words
	// 3. Create a work item for each word
	// 4. Send a chunk of work items to arbitrate

	// Read file as line by line
	std::string line;

	// File stream handle
	std::ifstream inputReadFile;

	inputReadFile.open(fileName);
	if (inputReadFile.is_open())
	{
		while (getline(inputReadFile, line))
		{
			// createWorkItems breaks the string into word and creates workItems
			arbitrateWorkItems(createWorkItems(line));
		}
	}

	inputReadFile.close();
	
}


