#include <reader.hpp>
#define NUM_LINES 1000

std::vector<workItem> createWorkItems(std::vector<std::string> lines, std::string fileName)
{
	std::vector<workItem> lineWorkItems;
	int count = 0;
	for(std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
	{
		std::istringstream iss(*it);	
		do
		{
			std::string subs;
			iss >> subs;
		
			// Make sure to create a new workItem
			// Check if the string is not all whitespaces
			if(subs.find_first_not_of(' ') != std::string::npos)
			{
				lineWorkItems.push_back(workItem(subs, 1));
			}

			if(subs.compare("Greek") == 0)
			{
				count++;
			}


		} while (iss);
	}

	std::cout << fileName << " Count" << count << std:: endl;

	return lineWorkItems;
}

void spawnNewReaderThread()
{

	std::string fileName = getNextSyncedFileName();
	std::vector<std::string> lines;

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

		if(inputReadFile.is_open())
		{
			// Read chunks of lines

			while(getline(inputReadFile, line))
			{
				lines.push_back(line);
				/*std::cout << line << std::endl;
				int i = 0;
				while(i++ <= NUM_LINES && getline(inputReadFile, line))
				{
					lines.push_back(line);
				}

				// createWorkItems breaks the string into word and creates workItems*/
				
			}

			arbitrateWorkItems(createWorkItems(lines, fileName));
			lines.clear();

		}

		else
		{
			std::cout << fileName << " not found! \n";
		}
		
		inputReadFile.close();

		// Get the next file to read
		fileName = getNextSyncedFileName();
	}

	readerFinshed();
	
}


