#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "constants.h"
#include "functions.h"
#include "fileUtil.h"

/**
 * Read the input data from standard input.
 * @param int* pointer to weights array.
 * @param char** pointer to first sequence.
 * @param int* pointer to number of sequences.
 * @return pointer to array of sequences.
 */
int readData(char ***sequences, char **firstSeq, int *numOfSequences, int *weights)
{

    int localWeights[NUM_OF_WEIGHTS]; // array of weights
    char tempString[MAX_SIZE_SEQ_REST ];
    char buffer[BUFFER_SIZE];
    char seq2buf[MAX_SIZE_SEQ_REST];

    // Read weights
    fgets(buffer, BUFFER_SIZE, stdin);
    sscanf(buffer, "%d %d %d %d", &localWeights[0], &localWeights[1], &localWeights[2], &localWeights[3]);

    for (int i = 0; i < NUM_OF_WEIGHTS; i++)
        weights[i] = localWeights[i];

    fgets(tempString, MAX_SIZE_SEQ_REST , stdin);

    tempString[strcspn(tempString, "\n")] = '\0';

    *firstSeq = strdup(tempString);
    if (!(*firstSeq))
    {
        fprintf(stderr, "Allocation error\n");
    }
    sequenceToUpper(*firstSeq);

    fgets(buffer, BUFFER_SIZE, stdin);
    sscanf(buffer, "%d", numOfSequences);

    *sequences = (char **)malloc(sizeof(char *) * (*numOfSequences));
    if (!(*sequences))
    {
        fprintf(stderr, "Allocation error\n");
    }

    for (int i = 0; i < *numOfSequences; i++)
    {
        fgets(seq2buf, MAX_SIZE_SEQ_REST , stdin);
        seq2buf[strcspn(seq2buf, "\n")] = '\0';
        (*sequences)[i] = strdup(seq2buf);
        sequenceToUpper((*sequences)[i]);
    }
    return 0;
}