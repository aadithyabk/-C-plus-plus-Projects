#include <stdlib.h>

#define ALPHABET_SIZE 26
#define CHAR_TO_INDEX(c) ((int)c - (int)'a') 	//Get the index of the letter in Alphabet
#define YES true
#define NO false

struct trie_node
{
	int value;
	trie_node *children[ALPHABET_SIZE];
	~trie_node()
	{
		for (int i = 0; i < ALPHABET_SIZE; i++)
		{
			if (children[i])
				free(children[i]);                //Free each node
		} 
	}
};

struct trie_t
{
	trie_node *root;
	int count;
};

struct Node
{
	int visited;
	struct Node* left;
	struct Node* Top;
	struct Node* Right;
	struct Node* Bottom;
	struct Node* RightUpper;
	struct Node* RightLower;
	struct Node* LeftUpper;
	struct Node* LeftLower;
	char data;
};
