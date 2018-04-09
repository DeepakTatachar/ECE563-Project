#include <reducer.hpp>

void spawnNewReducerThread(int globalReducerId, int globalMapperThreadCount, int mapperThreadsPerProcess)
{
	countTable wordCount;
	int data[2];

	MPI_Datatype workItemType;
	MPI_Status status;
	MPI_Aint disp[2] = { offsetof( reducerWorkItem, word), offsetof( reducerWorkItem, count) };
	MPI_Datatype type[2] = { MPI_CHAR, MPI_INT };
	int blocklen[2] = { MAX_STR_SIZE, 1 };

	MPI_Type_create_struct(2, blocklen, disp, type, &workItemType);
	MPI_Type_commit(&workItemType);

	for(int i = 0; i < globalMapperThreadCount; i++)
	{
		std::cout << "Waiting to receive from : " << i / mapperThreadsPerProcess << std::endl; 
		MPI_Recv(data, 2, MPI_INT, i / mapperThreadsPerProcess, globalReducerId, MPI_COMM_WORLD, &status);
		std::cout << "Received from : " << i / mapperThreadsPerProcess << std::endl; 	

		int size = data[0];
		int processNum = data[1];

		if(size <= 0)
		{
			continue;
		}


		reducerWorkItem* workArray = (reducerWorkItem*)malloc(sizeof(reducerWorkItem) * size);
		MPI_Recv(workArray, size, workItemType, processNum, globalReducerId, MPI_COMM_WORLD, &status);

		for(int i = 0; i < size; i++)
		{
			wordCount[workArray[i].word] += workArray[i].count;
		}
	}


	enqueueCountTable(wordCount);
}
