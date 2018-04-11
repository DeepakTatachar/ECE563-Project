#include <fileSyncManager.hpp>
#include <iostream>

int fileCount = 1;
int sentDone = 0;

void spawnNewFileManager(int globalReaderThreads)
{
	MPI_Status status;
	int readerThreadId;

	while(sentDone < globalReaderThreads)
	{
		MPI_Recv(&readerThreadId, 1, MPI_INT, MPI_ANY_SOURCE, FILE_SYNC_TAG, MPI_COMM_WORLD, &status);
		int processNum = status.MPI_SOURCE;

		if(fileCount >= NUM_FILES)
		{
			sentDone++;
			int allDone = -1;
			MPI_Send(&allDone, 1, MPI_INT, processNum, readerThreadId, MPI_COMM_WORLD);
		}
		else
		{
			MPI_Send(&fileCount, 1, MPI_INT, processNum, readerThreadId, MPI_COMM_WORLD);
			fileCount++;
		}
		
	}
}
