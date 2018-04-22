#include <fileSyncManager.hpp>
#include <iostream>

int fileCount = 1;
int sentDone = 0;
int numOfPasses = 0;

void spawnNewFileManager(int globalReaderThreads, int numPasses)
{
	MPI_Status status;
	int readerThreadId;
	numOfPasses = 0;

	while(sentDone < globalReaderThreads)
	{
		MPI_Recv(&readerThreadId, 1, MPI_INT, MPI_ANY_SOURCE, FILE_SYNC_TAG, MPI_COMM_WORLD, &status);
		int processNum = status.MPI_SOURCE;

		if(fileCount >= NUM_FILES && numOfPasses >= numPasses)
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


		if(fileCount >= NUM_FILES && numOfPasses < numPasses)
		{
			fileCount = 1;
			numOfPasses++;
		}
		
	}
}
