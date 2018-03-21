#ifndef   Reader_H
#define   Reader_H

#include <dataStructures.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

void spawnNewReaderThread(workQueueList wQList, std::string fileName);

#endif
