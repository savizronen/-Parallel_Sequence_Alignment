#ifndef _MPIUTILL_H_
#define _MPIUTILL_H_
char **recvSeqToWorkers();
char *mystrcatt(char *s1, char *s2);
char **str_split(char *a_str, const char a_delim);
void InitMPI(int *argc, char **argv[], int *rank, int *num_procs);
void sendSeqToWorkers(int numOfProcs, int numOfSequences, char **sequences);
void initMatrixScoresFirstSeqWighths(int *scoreMatrix, char *firstSeq, int *weights);
double sequentialCalc(int numOfSequences, int *scoreMatrix, char *firstSeq, char **sequences, Score maxScore);

#endif