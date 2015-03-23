#include<time.h>
#include<stdio.h>
#include<string>
#include<stdlib.h>
#include<conio.h>
#include<cstdlib>
#include<iostream>
#include<vector>
#include"Boggle.h"

trie_node *getNewNode()
{
	//Creates new node for the trie
	trie_node *newNode;
	newNode = (trie_node*)malloc(sizeof(trie_node));
	if (newNode)
	{
		newNode->value = 0;
		for (int i = 0; i < ALPHABET_SIZE; i++)
		{
			newNode->children[i] = NULL;
		}
	}
	return newNode;
}

void insertLetter(trie_t *trie, std::string tempStr)
{
	//Inserts letter into the trie

	char *key;
	int size = tempStr.size() + 1;
	key = new char[size];
	strcpy_s(key, size, tempStr.c_str());
	int length = strlen(key);
	int level;
	trie_node *temp;
	temp = trie->root;
	for (level = 0; level < length; level++)
	{
		int index = CHAR_TO_INDEX(key[level]);
		if (!(temp->children[index])) //If the node does not have child at this index, insert
		{
			temp->children[index] = getNewNode();
		}
		temp = temp->children[index];
	}
	trie->count++;
	temp->value = trie->count; //Indicates the end  of word
}

bool searchTrie(trie_t *trie, std::string tempStr,bool prefixSearch )
{
	char *key;
	int size = tempStr.size() + 1;
	key = new char[size];
	strcpy_s(key, size, tempStr.c_str());
	int length = strlen(key);
	int level;
	trie_node *temp;
	temp = trie->root;
	for (level = 0; level < length; level++)
	{
		int index = CHAR_TO_INDEX(key[level]);
		if (!(temp->children[index]))
			return false;

		temp = temp->children[index];

	}
	delete[] key;
	if (prefixSearch) //For Prefix Search, need not check if the prefix is actually a word. Hence no need to check the node's value/marker
		return true;
	else
	{
		//For a word search, it needs to be a valid word. Hence, need to check the value/marker at that node
		if (temp->value > 0)
			return true; 
		
		return false;
	}
}

bool checkIfStringAlreadyPresent(std::string temp, std::vector<std::string> output)
{
	bool result = false;
	std::vector<std::string>::iterator it;
	for (it = output.begin(); it < output.end(); it++)
	{

		std::string str = *it;
		int length = temp.length();
		if (strncmp(temp.c_str(), str.c_str(), length) == 0)
		{
			result = true;
			break; 
		}
	}
	return result;
}



void findStringUsingTrie(struct Node **Letters, struct Node* node, std::vector<std::string> &output, int *count, struct trie_t *trie, std::string temp)
{
	char data = node->data;

	if (node->visited == 0)
	{
		temp.push_back(data);
		node->visited = 1;
	}
	else
	{
		node->visited = 0;
		return;
	}
	bool result = searchTrie(trie, temp, YES); //Prefix Search

	if (result == false)
	{
		node->visited = 0;//Check if a string with this prefix is present. If not, no need to continue
		return;
	}
	int length = temp.length();

	if (length >= 3) //Minimum world length is 3
	{
		//Another string with this prefix might be present 
		//but this prefix might not be a valid word in dictionary. 
		//Example: bre is the prefix for bread but not a valid word.
		if (searchTrie(trie, temp,NO))  
		{							

			//If yes, add it
			//increment count
			bool result = checkIfStringAlreadyPresent(temp, output); //The same word might be possibe at two different places in the board
			if (!result)
			{
				output.push_back(temp); 
				(*count)++;
			}

		}
	}
	
	//Consider the neighbours of each letter in the table
	
	if (node->left != NULL)
		findStringUsingTrie(Letters, node->left, output, count, trie, temp);

	if (node->Right != NULL)
		findStringUsingTrie(Letters, node->Right, output, count, trie, temp);

	if (node->Top != NULL)
		findStringUsingTrie(Letters, node->Top, output, count, trie, temp);

	if (node->Bottom != NULL)
		findStringUsingTrie(Letters, node->Bottom, output, count, trie, temp);

	if (node->LeftLower != NULL)
		findStringUsingTrie(Letters, node->LeftLower, output, count, trie, temp);

	if (node->LeftUpper != NULL)
		findStringUsingTrie(Letters, node->LeftUpper, output, count, trie, temp);

	if (node->RightLower != NULL)
		findStringUsingTrie(Letters, node->RightLower, output, count, trie, temp);

	if (node->RightUpper != NULL)
		findStringUsingTrie(Letters, node->RightUpper, output, count, trie, temp);

	node->visited = 0;
}

void boggle(struct Node **Letters, std::vector<std::string> &Output, int size, trie_t *trie, std::string temp,int *count)
{
	for (int i = 0; i < size*size; i++)
	{
		findStringUsingTrie(Letters, Letters[i],Output,  count,  trie, temp);
	}
}

void fillLetters(struct Node** Letters, int i, char data, int size)
{
	int  res, rowNum;
	Letters[i]->data = data;
	Letters[i]->left = (i % size) == 0 ? NULL : Letters[i - 1];
	Letters[i]->Right = ((i + 1) % size) == 0 ? NULL : Letters[i + 1];
	Letters[i]->Top = (i - size < 0) ? NULL : Letters[i - size];
	Letters[i]->Bottom = (i + size >= size * size) ? NULL : Letters[i + size];

	//Neighbours of the a particular in the table 
	
	//RightLower
	res = ((i + size + 1) / size);
	rowNum = i / size;
	if (res == rowNum + 1 && res < size)
		Letters[i]->RightLower = Letters[i + size + 1];
	else
		Letters[i]->RightLower = NULL;

	//RightUpper
	res = ((i - size + 1) % size);
	if (res != 0 && (i - size + 1) >= 0)
		Letters[i]->RightUpper = Letters[i - size + 1];
	else
		Letters[i]->RightUpper = NULL;

	//LeftLower
	res = (i + size - 1);
	rowNum = i / size;
	if (rowNum != (res / size) && (res) < (size * size))
		Letters[i]->LeftLower = Letters[i + size - 1];
	else
		Letters[i]->LeftLower = NULL;

	//LeftUpper
	res = (i - size - 1);
	rowNum = i / size;
	if (res >= 0 && (rowNum - res / size) == 1)
		Letters[i]->LeftUpper = Letters[i - size - 1];
	else
		Letters[i]->LeftUpper = NULL;

}

int main()
{

	printf("Enter the size of square matrix: ");
	scanf_s("%d", &size);
	
	struct Node** Letters = new Node*[size*size];

	for (int j = 0; j < size * size; j++)
	{
		Letters[j] = new Node;
		Letters[j]->visited = 0;
	}
	for (int j = 0; j < size * size; j++)
	{
		char data;
		printf("Enter data: ");
		std::cin >> data;
		
		//fill the letters into the table
		fillLetters(Letters, j, data, size);
	}

	trie_t trie;
	trie.root = getNewNode();
	trie.count = 0;

	char dictionary[][65] = { "bred", "yore", "abed", "byre", "orby","robed","broad","byroad","robe","bored",
		"derby","bade","aero", "read", "orbed", "verb", "aery", "bead", "bread", "very", "road","phne",
		"rncs","trench","chant","panel","vet","eye","path","let","ten","robbed","watter","trek","tawt","att"
		,"bud","cert","bore","dewax","drew","wed","duet","oread","ewts","kets","awe","awed","bum","stawed",
		"tawts","taw","swatter","rewax","duett","crews","watt","trets","ked","duet","stewbum","buke","redub","cred","recs","ewt"};
	clock_t tStart = clock();
	for (int j = 0; j < 65;j++)
		insertLetter(&trie, dictionary[j]);

	boggle(Letters, Output, size, &trie, temp,&count);
	PrintOutput(Output,count);
	printf("\nTime taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
	_getch();
	return 0;
}