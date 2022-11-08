#include <cuda_runtime.h>
#include <iostream>
#include <string.h>
using namespace std;
#include "functions.h"
#include "cudaFunctions.h"


/// @brief Checking if the two letters are in the same group.
/// @param word The word in the group.
/// @param letter1 Letter from A-Z.
/// @param letter2 Letter from A-Z.
/// @return TRUE if find the two letter in the word or FALSE if not.
__device__ int isLettersInWordCuda(const char *word, char letter1, char letter2)
{
    int i = 0;
    int hasletter1 = FALSE, hasletter2 = FALSE;
    while (word[i] != '\0')
    {
        if (word[i] == letter1)
            hasletter1 = TRUE;
        else if (word[i] == letter2)
            hasletter2 = TRUE;
        i++;
    }
    if (hasletter1 == TRUE && hasletter2 == TRUE)
        return TRUE;

    return FALSE;
}

/// @brief  Checking if the two letters are in the same group.
/// @param group The group with words.
/// @param letter1 Letter from A-Z.
/// @param letter2 Letter from A-Z.
/// @param size Size of the group.
/// @return TRUE if find the two letter existing in the group or FALSE if not.
__device__ int isLettersInGroupCuda(const char **group, char letter1, char letter2, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (isLettersInWordCuda(group[i], letter1, letter2) == TRUE)
            return TRUE;
    }
    return FALSE;
}


/// @brief  Fill the matrix with scores for every to letters by GPU.
/// @param devScoreMatrix Empty score matrix 26*26 from the GPU.
/// @param devWeight Weights for each coupling w[0] w[1] w[2] w[3] from the GPU.
__global__ void fillMatrix(int *devScoreMatrix, int *devWeight)
{
    const char *firstGroup[FIRST_GROUP_SIZE] = {"NDEQ", "NEQK", "STA", "MILV", "QHRK", "NHQK", "FYW", "HY", "MILF"};
    const char *secondGroup[SECOND_GROUP_SIZE] = {"SAG", "ATV", "CSA", "SGND", "STPA", "STNK", "NEQHRK", "NDEQHK", "SNDEQK", "HFY", "FVLIM"};

    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;

    if (i < ENGLISH_LETTERS && j < ENGLISH_LETTERS)
    {
        char letter1 = i + 'A';
        char letter2 = j + 'A';
        if (letter1 == letter2)
            devScoreMatrix[i * ENGLISH_LETTERS + j] = devWeight[0];
        else if (isLettersInGroupCuda(firstGroup, letter1, letter2, FIRST_GROUP_SIZE))
            devScoreMatrix[i * ENGLISH_LETTERS + j] = -devWeight[1];
        else if (isLettersInGroupCuda(secondGroup, letter1, letter2, SECOND_GROUP_SIZE))
            devScoreMatrix[i * ENGLISH_LETTERS + j] = -devWeight[2];
        else
            devScoreMatrix[i * ENGLISH_LETTERS + j] = -devWeight[3];
    }
}

/**
 * Check if cuda status success.
 * @param int* pointer to array.
 * @return EXIT_SUCCESS if everything worked properly, EXIT_FAILURE else.
 */
int checkCudaMalloc(int *array)
{
    if (array == NULL)
    {
        printf("Cuda malloc failed!");
        cudaFree(array);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/// @brief Fill the matrix with scores for every to letters.
/// @param scoreMatrix Empty score matrix 26*26 from the cpu.
/// @param weights Weights for each coupling w[0] w[1] w[2] w[3] from the cpu.
/// @return EXIT_SUCCESS or EXIT_FAILURE if the func fill the matrix with scores.
int fillSymbolsWeightsCuda(int *scoreMatrix, int *weights)
{
    int *devWeights = NULL, *devScoreMatrix = NULL;
    int threads = ENGLISH_LETTERS / 2;
    dim3 THREADS(threads, threads);
    dim3 BLOCKS(2, 2);

    cudaMalloc((void **)&devScoreMatrix, ENGLISH_LETTERS * ENGLISH_LETTERS * sizeof(int));
    if (checkCudaMalloc(devScoreMatrix) == EXIT_FAILURE)
        return EXIT_FAILURE;
    cudaMalloc((void **)&devWeights, NUM_OF_WEIGHTS * sizeof(int));
    if (checkCudaMalloc(devWeights) == EXIT_FAILURE)
        return EXIT_FAILURE;

    cudaMemcpy(devScoreMatrix, scoreMatrix, ENGLISH_LETTERS * ENGLISH_LETTERS * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(devWeights, weights, NUM_OF_WEIGHTS * sizeof(int), cudaMemcpyHostToDevice);

    fillMatrix<<<BLOCKS, THREADS>>>(devScoreMatrix, devWeights);
    cudaDeviceSynchronize();

    cudaMemcpy(scoreMatrix, devScoreMatrix, ENGLISH_LETTERS * ENGLISH_LETTERS * sizeof(int), cudaMemcpyDeviceToHost);
    cudaFree(devScoreMatrix);
    cudaFree(devWeights);
    return EXIT_SUCCESS;
}