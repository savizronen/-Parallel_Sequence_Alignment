#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <omp.h>
#include <mpi.h>
#include "cudaFunctions.h"
#include "fileUtil.h"
#include "functions.h"
#include "constants.h"
#include "mpiUtill.h"

int main(int argc, char *argv[])
{
	Score maxScore;
	fillScore(&maxScore, 0, 0, 0);
	int firstSeqSize, numOfSequences;
	double parallelTime, sequentialTime;
	int rank, numOfProcs, NumOfSeqPerProc;
	char **sequences, **sequencesPerWorker;
	int *weights = (int *)doMalloc(sizeof(int) * NUM_OF_WEIGHTS);
	char *firstSeq = (char *)doMalloc(MAX_SIZE_SEQ1 * sizeof(char) + 1);
	int *scoreMatrix = (int *)doMalloc(ENGLISH_LETTERS * ENGLISH_LETTERS * sizeof(int));

	InitMPI(&argc, &argv, &rank, &numOfProcs);

	if (rank == MASTER)
	{
		if (readData(&sequences, &firstSeq, &numOfSequences, weights) != 0)
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		firstSeqSize = strlen(firstSeq);
		fillSymbolsWeightsCuda(scoreMatrix, weights);
		sequentialTime = sequentialCalc(numOfSequences, scoreMatrix, firstSeq, sequences, maxScore);
	}

	MPI_Bcast(&numOfSequences, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&firstSeqSize, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(firstSeq, firstSeqSize, MPI_CHAR, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(weights, NUM_OF_WEIGHTS, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(scoreMatrix, ENGLISH_LETTERS * ENGLISH_LETTERS, MPI_INT, MASTER, MPI_COMM_WORLD);
	NumOfSeqPerProc = numOfSequences / numOfProcs; // work seq per prosses

	if (rank == MASTER)
	{
		parallelTime = MPI_Wtime(); // Start time for the parallel code.
		sendSeqToWorkers(numOfProcs, numOfSequences, sequences);
		NumOfSeqPerProc += (numOfSequences % numOfProcs); // Adding the remainders for MASTER.
		sequencesPerWorker = (char **)doMalloc(NumOfSeqPerProc * sizeof(char *));
		int compareSeqSize;
		for (int i = 0; i < NumOfSeqPerProc; i++)
		{
			compareSeqSize = strlen(sequences[i]);
			sequencesPerWorker[i] = (char *)doMalloc(compareSeqSize * sizeof(char) + 1);
			strcpy(sequencesPerWorker[i], strdup(sequences[i]));
		}
	}
	else
	{
		sequencesPerWorker = (char **)doMalloc(NumOfSeqPerProc * sizeof(char *));
		char **ptrSequences = recvSeqToWorkers();
		for (int i = 0; i < NumOfSeqPerProc; i++)
		{
			int size = strlen(ptrSequences[i]);
			ptrSequences[i][size] = '\0';
			sequencesPerWorker[i] = (char *)doMalloc((size + 1) * sizeof(char));
			strcpy(sequencesPerWorker[i], strdup(ptrSequences[i]));
		}
	}

	int sortArr[2];
	for (int i = 0; i < NumOfSeqPerProc; i++)
	{
		findMaxScore(scoreMatrix, firstSeq, sequencesPerWorker[i], &maxScore, PARALLEL);
		if (rank != 0)
		{
			sortArr[0] = maxScore.offsetN;
			sortArr[1] = maxScore.mutantIndexK;
			MPI_Send(&sortArr, 2, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
		}
		else
		{
			printf("N = %d\t K = %d\n", maxScore.offsetN, maxScore.mutantIndexK);
		}
	}

	if (rank == MASTER)
	{
		for (int procsNum = 1; procsNum < numOfProcs; procsNum++)
		{
			for (int seqIndex = 0; seqIndex < NumOfSeqPerProc; seqIndex++)
			{
				MPI_Recv(&sortArr, 2, MPI_INT, procsNum, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				printf("N = %d\t K = %d\n", sortArr[0], sortArr[1]);
			}
		}
		parallelTime = MPI_Wtime() - parallelTime;
		printf("Parallel run time: %lf\n", parallelTime);
		printf("---------------------------------------\n");
		freeAllSequencesForMaster(sequences, numOfSequences);
	}
	freeAllocationPerProc(sequencesPerWorker, NumOfSeqPerProc, weights, firstSeq, scoreMatrix);

	MPI_Finalize();
	return EXIT_SUCCESS;
}
