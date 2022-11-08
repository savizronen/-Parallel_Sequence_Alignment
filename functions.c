#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <ctype.h>
#include "functions.h"
#include "constants.h"

/**
 * Changing all the letters in sequence to upper case parallel with openMp.
 * @param char* pointer to sequence.
 */
void sequenceToUpper(char *sequence)
{
#pragma omp parallel for
    for (int i = 0; i < strlen(sequence); i++)
        sequence[i] = toupper(sequence[i]);
}

/**
   Sequential
 * Calculate the score of the sequence.
 * @param char* Pointer to matrix that we want to fill with Weights to all the couple letters.
 * @param char* Pointer to first sequence.
 * @param char* Pointer to second sequence.
 * @param int   Second sequence size.
 * @param int   Sequence offset
 * @param int  Mutant index.
 * @return The score of the sequence.
 */
int calcScore(int *scoreMatrix, const char *firstSeq, const char *secondSeq, int secondSeqSize, int offset, int mutant)
{
    int row, col, afterMutant = 0, score = 0;

    for (int i = 0; i <= secondSeqSize; i++)
    {
        if (i != mutant)
        {
            row = firstSeq[i + offset] - 'A';
            col = secondSeq[i - afterMutant] - 'A';
            score += scoreMatrix[row * ENGLISH_LETTERS + col];
        }
        else
            afterMutant = 1; // Reduce 1 from the index after the mutant
    }

    return score;
}

/**
 * Fill a score Struct.
 * @param Score* Score struct to fill.
 * @param char* Score number.
 * @param char* Score offset.
 * @param int   Mutant index.
 */
void fillScore(Score *fillScore, int score, int offset, int mutantIndex)
{
    fillScore->alignmentScore = score;
    fillScore->offsetN = offset;
    fillScore->mutantIndexK = mutantIndex;
}

/// @brief Calculation the mutant for each offset.
/// @param scoreMatrix Pointer to matrix with scores to all the couple of letters.
/// @param firstSeq Pointer to first sequence.
/// @param secondSeq Pointer to second sequence.
/// @param secondSeqSize Second sequence size.
/// @param offset The offset that we want to know the mutant for him.
/// @param score The score for the last option.
/// @param maxScore Score that we want to fill with the max score.
void calcMutant(int *scoreMatrix, const char *firstSeq, const char *secondSeq, int secondSeqSize, int offset, int score, Score *maxScore)
{
    for (int mutant = 1; mutant <= secondSeqSize; mutant++)
    {
        score = calcScore(scoreMatrix, firstSeq, secondSeq, secondSeqSize, offset, mutant);
        if (score > maxScore->alignmentScore)
            fillScore(maxScore, score, offset, mutant);
    }
}

/// @brief Fill a struct named maxScore in a maximum value score by considering the mutation and offset.
/// @param scoreMatrix Pointer to matrix with scores to all the couple of letters.
/// @param firstSeq Pointer to first sequence.
/// @param secondSeq Pointer to second sequence.
/// @param maxScore Pointer to maxScore struct that we want to fill.
/// @param SequentialOrParallel run SEQUENTIAL = 0 or PARALLEL = 1 .
void findMaxScore(int *scoreMatrix, const char *firstSeq, const char *secondSeq, Score *maxScore, int SequentialOrParallel)
{
    int offset, mutant;
    int maxOffset, score;
    int firstSeqSize = strlen(firstSeq);
    int secondSeqSize = strlen(secondSeq);

    maxOffset = firstSeqSize - secondSeqSize;
    score = calcScore(scoreMatrix, firstSeq, secondSeq, secondSeqSize, maxOffset, secondSeqSize); // Calculate the score for the last option
    fillScore(maxScore, score, offset, mutant);

    if (SequentialOrParallel == SEQUENTIAL)

        for (offset = 0; offset < maxOffset; offset++)
            calcMutant(scoreMatrix, firstSeq, secondSeq, secondSeqSize, offset, score, maxScore);

    if (SequentialOrParallel == PARALLEL)

#pragma omp parallel for private(score, offset, mutant) shared(scoreMatrix, firstSeq, secondSeq, maxScore) firstprivate(secondSeqSize)
        for (offset = 0; offset < maxOffset; offset++)
            calcMutant(scoreMatrix, firstSeq, secondSeq, secondSeqSize, offset, score, maxScore);
}

void *doMalloc(unsigned int nbytes)
{
    void *p = malloc(nbytes);

    if (p == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    return p;
}
void printMaxOffsetAndMutantIndexK(Score *maxScores, int arraySize)
{
    for (int i = 0; i < arraySize; i++)
        printf("N = %d\t K = %d\n", maxScores[i].offsetN, maxScores[i].mutantIndexK);
}

void freeAllSequencesForMaster(char **sequences, int numOfSequences)
{
    int i;
#pragma omp parallel for
    for (i = 0; i < numOfSequences; i++)
        free(sequences[i]);
    free(sequences);
}

void freeAllocationPerProc(char **sequencesPerWorker,int NumOfSeqPerProc, int *weights,char *firstSeq,int*scoreMatrix)
{
    int i;
#pragma omp parallel for
    for (i = 0; i < NumOfSeqPerProc; i++)
        free(sequencesPerWorker[i]);
    free(sequencesPerWorker);
    free(weights);
    free(firstSeq);
    free(scoreMatrix);
}