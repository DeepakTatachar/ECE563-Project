#ifndef  Reducer_H
#define  Reducer_H

#include <dataStructures.hpp>
#include <map>

void spawnNewReducerThread(int id, workQueue reducerQ, int CHUNK_SIZE);

#endif
