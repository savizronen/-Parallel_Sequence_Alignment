#include "constants.h"
#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

void *doMalloc(unsigned int nbytes);
void sequenceToUpper(char *sequence);
void fillScoresMatrix(int *weights, int *scoreMatrix);
void printMaxOffsetAndMutantIndexK(Score *maxScores, int arraySize);
void freeAllSequencesForMaster(char **sequences, int numOfSequences);
void fillScore(Score *fillScore, int score, int offset, int mutantIndex);
int checkGroup(const char **group, char letter1, char letter2, int size);
int calcScore(int *scoreMatrix, const char *firstSeq, const char *secondSeq, int secondSeqSize, int offset, int mutant);
void freeAllocationPerProc(char **sequencesPerWorker, int NumOfSeqPerProc, int *weights, char *firstSeq, int *scoreMatrix);
void findMaxScore(int *scoreMatrix, const char *firstSeq, const char *secondSeq, Score *maxScore, int SequentialOrParallel);
void calcMutant(int *scoreMatrix, const char *firstSeq, const char *secondSeq, int secondSeqSize, int offset, int score, Score *maxScore);
#endif