#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <omp.h>
#include <mpi.h>
#include "fileUtil.h"
#include "functions.h"
#include "constants.h"

/// @brief initialize mpi communication
void InitMPI(int *argc, char **argv[], int *rank, int *num_procs)
{
    MPI_Init(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, rank);
    MPI_Comm_size(MPI_COMM_WORLD, num_procs);
}

/// @brief Function that connect two strings and add zero to the end example s1:"A0" s2:"B" return "A0B0".
/// @param s1 pointer to char array.
/// @param s2 pointer to char array.
/// @return new pointer of the connect two strings.
char *mystrcatt(char *s1, char *s2)
{
    int a = strlen(s1);
    int b = strlen(s2);
    int i, size_ab = a + b;

    char *s3 = (char *)doMalloc(((size_ab + 2) * sizeof(char))); // sizeof(char) is always 1

#pragma omp parallel for
    for (i = 0; i < a; i++)
    { // inefficient
        (s3[i]) = s1[i];
    }
#pragma omp parallel for
    for (i = 0; i < b; i++)
    { // inefficient
        (s3[i + a]) = s2[i];
    }

    s3[size_ab] = '0';
    s3[size_ab + 1] = '\0';

    return s3;
}
/// @brief Split string by delimeter.
/// @param a_str Pointer to char array.
/// @param a_delim A delimeter to split.
/// @return ** pointer to array of strings.
char **str_split(char *a_str, const char a_delim)
{
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char **)doMalloc(sizeof(char *) * count);

    if (result)
    {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
/// @brief Send sequences by the master to all the workers.
/// @param numOfProcs  Num of process.
/// @param numOfSequences  Num of sequences.
/// @param sequences Pointer to all the sequences.
void sendSeqToWorkers(int numOfProcs, int numOfSequences, char **sequences)
{
    int NumOfSeqPerProc = (numOfSequences / numOfProcs);
    int masterSeqSizeWithRest = NumOfSeqPerProc + (numOfSequences % numOfProcs); // Don't send the ones reserved for MASTER
    char *sequencesPerProc = strdup("");
    int compareSeqSize;
    int sequencesSendSize = 0;
    for (int i = 1; i < numOfProcs; i++)
    {

        for (int j = 0; j < NumOfSeqPerProc; j++)
        {
            compareSeqSize = strlen(*(sequences + masterSeqSizeWithRest));
            sequencesSendSize += compareSeqSize + 1;
            sequencesPerProc = mystrcatt(sequencesPerProc, *(sequences + masterSeqSizeWithRest));
            masterSeqSizeWithRest++;
        }

        MPI_Send(&sequencesSendSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Send(sequencesPerProc, sequencesSendSize, MPI_CHAR, i, 0, MPI_COMM_WORLD);

        // printf(" %s %d  \n",sequencesPerProc,sequencesSendSize);
        sequencesPerProc = strdup("");
        sequencesSendSize = 0;
    }
}
/// @brief  Recive sequences Per Procss and sequences Size for each worker.
/// @return  sequences Per Procss.
char **recvSeqToWorkers()
{
    int sequencesSendSize;
    MPI_Recv(&sequencesSendSize, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char *sequencesPerProc = (char *)doMalloc((sequencesSendSize + 1) * sizeof(char));
    MPI_Recv(sequencesPerProc, sequencesSendSize + 1, MPI_CHAR, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char **ptrSequences = str_split(sequencesPerProc, '0');
    return ptrSequences;
}
/// @brief Sequential program.
/// @param numOfSequences Num of sequences.
/// @param scoreMatrix Matrix with scores for every two letters.
/// @param firstSeq The First sequence.
/// @param sequences Pointer to array with all the strings.
/// @param maxScore Max score struct that we going to fill.
/// @return double sequential Time
double sequentialCalc(int numOfSequences, int *scoreMatrix, char *firstSeq, char **sequences, Score maxScore)
{
    // Sequential.
    // Start time for the sequential code.
    double sequentialTime = MPI_Wtime();
    printf("---------------------------------------\n");
    Score *maxScoreArray = (Score *)doMalloc(sizeof(Score) * numOfSequences);
    for (int i = 0; i < numOfSequences; i++)
    {
        findMaxScore(scoreMatrix, firstSeq, sequences[i], &maxScore, SEQUENTIAL);
        maxScoreArray[i] = maxScore;
    }
    printMaxOffsetAndMutantIndexK(maxScoreArray, numOfSequences);
    sequentialTime = MPI_Wtime() - sequentialTime;
    printf("Sequential run time: %lf\n", sequentialTime);
    printf("---------------------------------------\n");
    return sequentialTime;
}