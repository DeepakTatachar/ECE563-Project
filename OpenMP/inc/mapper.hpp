#ifndef   Mapper_H
#define   Mapper_H

#include <dataStructures.hpp>
#include <functional>

void spawnNewMapperThread(workQueue wQ, int mapperId, int reducerThreads, int CHUNK_SIZE);

#endif
