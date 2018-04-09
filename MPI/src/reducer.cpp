#include <reducer.hpp>

void spawnNewReducerThread(int globalReducerId, int globalMapperThreadCount, int mapperThreadsPerProcess)
{
	countTable wordCount;

	MPI_Datatype workItemType;
	MPI_Status status;
	MPI_Aint disp[2] = { offsetof( workItem, word), offsetof( workItem, count) };
	MPI_Datatype type[2] = { MPI_CHAR, MPI_INT };
	int blocklen[2] = { MAX_STR_SIZE, 1 };

	MPI_Type_create_struct(2, blocklen, disp, type, &workItemType);
	MPI_Type_commit(&workItemType);

	for(int i = 0; i < globalMapperThreadCount; i++)
	{
	        MPI_Probe(MPI_ANY_SOURCE, globalReducerId, MPI_COMM_WORLD, &status);
		int processNum = status.MPI_SOURCE;
		int size;

		std::cout << "Received from : " << processNum << std::endl; 	

		MPI_Get_count(&status, workItemType, &size);

		workItem* workArray = (workItem*)malloc(sizeof(workItem) * size);
		MPI_Recv(workArray, size, workItemType, processNum, globalReducerId, MPI_COMM_WORLD, &status);

		for(int i = 0; i < size; i++)
		{
			if(workArray[i].count == 0)
		        {
				break;
			}

			wordCount[workArray[i].word] += workArray[i].count;
		}
	}


	enqueueCountTable(wordCount);
}
