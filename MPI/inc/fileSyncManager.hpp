#ifndef   FileSyncManager_H
#define   FileSyncManager_H

#include <dataStructures.hpp>
#include <mpi.h>

void spawnNewFileManager(int globalReaderThreads, int numPasses);

#endif
