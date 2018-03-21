#include <reader.hpp>

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

void spawnNewReaderThread()
{

	std::string fileName = getNextSyncedFileName();

	while(fileName.compare(""))
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

		// Get the next file to read
		fileName = getNextSyncedFileName();
	}
	
}


