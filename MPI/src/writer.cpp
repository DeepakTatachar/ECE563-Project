#include <writer.hpp>
#include <iostream>
#include <fstream>

void writeFile(std::string fileName)
{
	std::ofstream outputFile;
	outputFile.open(fileName);

	std::vector<countTable> wordCountTable = getCountList();
	for(std::vector<countTable>::iterator it = wordCountTable.begin(); it != wordCountTable.end(); ++it)
	{
		countTable table = *it;
		for(countTable::const_iterator itr = table.begin(); itr != table.end(); ++itr)
		{
			outputFile << itr->first << " : " << itr->second << std::endl;
		}
	}

	outputFile.close();
}
