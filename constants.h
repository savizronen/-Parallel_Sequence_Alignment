#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

enum size
{
	MASTER = 0,
	SCORES_SIZE = 3,
	NUM_OF_WEIGHTS = 4,
	FIRST_GROUP_SIZE = 9,
	SECOND_GROUP_SIZE = 11,
	PARALLEL = 1,
	SEQUENTIAL = 0,
	ENGLISH_LETTERS = 26,
	BUFFER_SIZE = 200,
	MAX_SIZE_SEQ1 = 3000,
	MAX_SIZE_SEQ_REST = 2000,
	TRUE = 1,
	FALSE = 0
};

typedef struct Score
{
	int alignmentScore; // Alignment score.
	int offsetN;		// Offset.
	int mutantIndexK;	// Hyphen position mutant.
} Score;

#endif