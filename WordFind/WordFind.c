#define _CRT_SECURE_NO_WARNINGS
#define UPPERCASE_RANGE (25)
#define UPPERCASE_OFFSET (65)
#define TO_LOWERCASE (32)
#define MAX_ELEMENT_COUNT (15)
#define MAX_WORD_LENGTH (50)
#define MAX_WORDS_PER (9) //9 because 0 holds the category. 8 actual words to place
#define WORDS_TO_FIND_START (1)

#define PNRM "\x1B[0m"
#define PRED "\x1B[31m"
#define PGREEN "\x1B[32m"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

enum Pattern
{
    VERTICAL = (char) 0,
    HORIZONTAL = (char) 1,
    DIAGONAL_LOHI = (char) 2,
    DIAGONAL_HILO = (char) 3,

    PATTERN_MAX = (char) 4
};

enum Orientation
{
    FORWARDS = (char) 0,
    BACKWARDS = (char) 1,

    ORIENT_MAX = (char) 2
};

typedef struct Word
{
    char word[MAX_WORD_LENGTH];
    char selected;
    char found;
    char row;
    char column;
    enum Pattern pattern;
    enum Orientation orientation;
    short length;
}Word;

typedef struct Letter
{
    char letter;
    char found;
    char occupied;
}Letter;

char GenerateULetter()
{
    float value = (float)rand() / RAND_MAX;
    int result = (int)round(value * UPPERCASE_RANGE) + UPPERCASE_OFFSET;
    return result;
}

struct Letter** GenerateWordFind(int size)
{
    
    struct Letter** wordFind = (struct Letter**)malloc(size * sizeof(struct Letter*));
    
    for (int i = 0; i < size; i++)
    {
        wordFind[i] = (struct Letter*)malloc(size * sizeof(struct Letter) * MAX_WORD_LENGTH);
    }

    //65 to 90

    //Fill with random stuff
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            wordFind[i][j].letter = GenerateULetter();
            wordFind[i][j].found = 0;
            wordFind[i][j].occupied = 0;
        }
    }

    return wordFind;

}

void PrintWordFind(struct Letter** board, int size)
{
    //Temp range

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (board[i][j].found == 1)
            {
                printf(" %s%c%s", PGREEN, board[i][j].letter, PNRM);
            }
            /*else if (board[i][j].occupied == 1)
            {
                printf(" %s%c%s", PRED, board[i][j].letter, PNRM);
            }*/
            else
            {
                printf(" %c", board[i][j].letter);
            }
            


            if (j == size - 1)
            {
                printf("\n");
            }
        }
    }
}

void PrintWordList(struct Word* wordList)
{
    printf("Words to find:\n");

    for (int i = WORDS_TO_FIND_START; i < MAX_WORDS_PER; i++)
    {
        if (wordList[i].found == 1)
        {
            printf("%s%s%s\n", PGREEN, wordList[i].word, PNRM);
        }
        else
        {
            printf("%s\n", wordList[i].word);
        }
       
    }
    printf("\n");
}

void ParseWordFile(FILE* stream, int size, struct Word* wordList)
{

    //Scan for categories
    char* buffer = malloc(sizeof(char) * MAX_WORD_LENGTH);
    int returnVal;

    const char delimiter = ':';

    printf("AVAILABLE CATEGORIES\n");
    while (!feof(stream))
    {
        returnVal = fscanf(stream, "%s", buffer);
        if (returnVal == 1 && strncmp(buffer, "CATEGORY", 8) == 0) //We have a category
        {
            printf("%s\n", strchr(buffer, delimiter) + sizeof(char));
        }

    }
    rewind(stream);

    printf("Which category do you want?\n");
    char inputBuffer[MAX_WORD_LENGTH];
    char* ptr = inputBuffer;
    char found = 1;
    int index = 0;

    
    
    //TAKE IN PLAYER INPUT TO SELECT A CATEGORY
    while (found)
    {
        //FGETS
        returnVal = fgets(&inputBuffer, MAX_WORD_LENGTH, stdin);
        
        while(!feof(stream))
        {
            returnVal = fscanf(stream, "%s", buffer);

            if (returnVal == 1 && strncmp(buffer, "CATEGORY", 8) == 0) //We have a category
            {
                strcpy(buffer, (strchr(buffer, delimiter) + sizeof(char)));
                if (returnVal == 1 && strncmp(buffer, inputBuffer, strlen(inputBuffer) - 1) == 0) //We have a matching category
                {
                    //Found a match
                    strcpy(wordList[index].word, buffer);
                    found = 0;
                    break;
                }
            }
            
        }

        if (found == 0) //Found s break
        {
            break;
        }
        else
        {
            printf("Category not found, please try again\n");
        }
        rewind(stream);

    }
    
    //Load words from that category randomly
    
    struct Word* tempHolder = malloc(sizeof(struct Word) * MAX_ELEMENT_COUNT);

    //Load words all words
    for (int i = 0; i < MAX_ELEMENT_COUNT; i++) 
    {
        returnVal = fscanf(stream, "%s", buffer);
        strcpy(tempHolder[i].word, buffer);
        tempHolder[i].selected = 0;
        
    }
    

    for (int i = 1; i < MAX_WORDS_PER; i++)
    {
        int target = (int)round(((float)rand() / RAND_MAX) * (MAX_ELEMENT_COUNT - 1));

        if (tempHolder[target].selected == 1) //Word already used, so iterate through list to find next available
        {
            while (1)
            {
                target++;
                if (target >= MAX_ELEMENT_COUNT)
                {
                    target = 0;
                }
                if (tempHolder[target].selected == 0)
                {
                    strcpy(wordList[i].word, tempHolder[target].word);
                    tempHolder[target].selected = 1;
                    break;
                }
            }
        }
        else
        {
            strcpy(wordList[i].word, tempHolder[target].word);
            tempHolder[target].selected = 1;
        }
    }

    //Set start Values
    for (int i = 1; i < MAX_WORDS_PER; i++)
    {
        
        wordList[i].found = 0;
        wordList[i].selected = 0;
        wordList[i].length = (short)strlen(wordList[i].word);

    }

    free(buffer);
    free(tempHolder);
    
}


void PlaceWord(struct Word* word, struct Letter** board, int* stepX, int* stepY)
{
    printf("Placing %s at row %d column %d\n", word->word, word->row + 1, word->column + 1);

    for (int x = word->row, y = word->column, index = 0; index < word->length; x += *stepX, y += *stepY, index++)
    {

        board[x][y].letter = word->word[index] > UPPERCASE_OFFSET + UPPERCASE_RANGE ? word->word[index] - TO_LOWERCASE : word->word[index];
        board[x][y].occupied = 1;
    }
}


int strncmpNoCase(char* str1, char* str2, size_t maxCount)
{
    int result = 0;

    for (size_t i = 0; i < maxCount; i++)
    {
        char temp1 = str1[i];
        char temp2 = str2[i];

        if (temp1 <= UPPERCASE_OFFSET + UPPERCASE_RANGE)
        {
            temp1 += TO_LOWERCASE;
        }
        if (temp2 <= UPPERCASE_OFFSET + UPPERCASE_RANGE)
        {
            temp2 += TO_LOWERCASE;
        }

        if (temp1 != temp2)
        {
            return 1;
        }
    }
    return 0;
}


int OrientationSign(enum Orientation o)
{
    if (o == FORWARDS)
    {
        return 1;
    }

    return -1;
}

void GetStepValues(enum Pattern* p, enum Orientation* o, int* stepX, int* stepY)
{
    switch (*p)
    {
    case HORIZONTAL:
    {
        *stepX = 1 * OrientationSign(*o);
        *stepY = 0;
        break;
    }
    case VERTICAL:
    {
        *stepX = 0;
        *stepY = 1 * OrientationSign(*o);
        break;
    }
    case DIAGONAL_LOHI:
    {
        *stepX = 1 * OrientationSign(*o);
        *stepY = -1 * OrientationSign(*o);
        break;
    }
    case DIAGONAL_HILO:
    {
        *stepX = 1 * OrientationSign(*o);
        *stepY = 1 * OrientationSign(*o);
        break;
    }
    default:
    {
        *stepX = 0;
        *stepY = 0;
        printf("WE SHOULDNT BE HERE FOR PATTERN!\n");
    }
    }
}

void GeneratePlacementVars(char* posX, char* posY, int* stepX, int* stepY, enum Pattern* p, enum Orientation* o, int boardSize, int wordSize)
{

    *posX = (char)round(((float)rand() / RAND_MAX) * (boardSize - wordSize));
    *posY = (char)round(((float)rand() / RAND_MAX) * (boardSize - wordSize));

    *p = (char)round(((float)rand() / RAND_MAX) * (PATTERN_MAX - 1));
    *o = (char)round(((float)rand() / RAND_MAX) * (ORIENT_MAX - 1));

    //printf("Pattern: %d\n", *p);
    //printf("Orient: %d\n", *o);
   
    
    if (*o == FORWARDS) //We need to worry about max size
    {
        *posX = (char)round(((float)rand() / RAND_MAX) * (boardSize - wordSize));
        *posY = (char)round(((float)rand() / RAND_MAX) * (boardSize - wordSize));
    }
    else //We need to worry about below 0
    {
        *posX = (char)round((((float)rand() / RAND_MAX) * (boardSize - wordSize)) + wordSize);
        *posY = (char)round((((float)rand() / RAND_MAX) * (boardSize - wordSize)) + wordSize);
    }

    GetStepValues(p, o, stepX, stepY);
    
}


char GenerateOverlapWord(struct Letter** board, struct Word* word, int* stepX, int* stepY, int size, int intersectX, int intersectY)
{
    //FOR TESTING
    //printf("\nTESTING\n");
    //PrintWordFind(board, size);

    //Check to see if our word contains the same letter
    //printf("%s vs %c\n", word->word, board[intersectX][intersectY].letter);

    int signX = *stepX;
    int signY = *stepY;

    char* intersectCharPtr = strchr(word->word, board[intersectX][intersectY].letter);

    if (intersectCharPtr == NULL) //To check if lowercase
    {
        intersectCharPtr = strchr(word->word, board[intersectX][intersectY].letter + TO_LOWERCASE);
    }

    if (intersectCharPtr != NULL)
    {
        //Check to see if we are not out of range for lo and hi X/Y
        int partLength = intersectCharPtr - word->word; //Letter of intersection in the word - the start of the word (gives us the length)

        if (intersectX + (partLength * signX * -1) >= 0 && intersectX + (partLength * signX * -1) < size &&
            intersectY + (partLength * signY * -1) >= 0 && intersectY + (partLength * signY * -1) < size) //front word part
        {
            partLength = word->length - partLength - 1; //-1 to exclude the intersected part
            if (intersectX + (partLength * signX) >= 0 && intersectX + (partLength * signX) < size &&
                intersectY + (partLength * signY) >= 0 && intersectY + (partLength * signY) < size) //latter word part 
            {
                //We can fit it! Set the new location of the word
                word->row = (char)(intersectX + ((intersectCharPtr - word->word) * signX * -1));
                word->column = (char)(intersectY + ((intersectCharPtr - word->word) * signY * -1));

            }
            else
            {
                return 0;
            }
            
        }
        else
        {
            return 0;
        }
        

    }
    else
    {
        return 0;
    }
    
    int occupiedCount = 0;

    // Check to see if there isnt a second occupied space in the way
    for (int x = word->row, y = word->column, index = 0; index < word->length; x += *stepX, y += *stepY, index++)
    {
        if (board[x][y].occupied == 1) //Allows for another crossover if they are the same letter
        {

            //if (board[x][y].letter != word->word[index]) <- Can work, but can also create small edgecases that would need more checking
            occupiedCount++;          
        }
    }
    if (occupiedCount > 1)
    {
        return 0;
    }
    return 1;

}

void InsertWords(struct Word* wordList, struct Letter** board, int size)
{
    for (int i = WORDS_TO_FIND_START; i < MAX_WORDS_PER; i++)
    {
        struct Word* wordToPlace = &wordList[i];
        
        int stepX = 0;
        int stepY = 0;

        int searching = 1;

        while (searching)
        {
            GeneratePlacementVars(&wordToPlace->row, &wordToPlace->column, &stepX, &stepY, &wordToPlace->pattern, &wordToPlace->orientation, size, wordToPlace->length);
            //printf("Placing %s with a %d Pattern with %d Orientation\n", wordToPlace->word, wordToPlace->pattern, wordToPlace->orientation);
            //Check if place is not occupied
            for (int x = wordToPlace->row, y = wordToPlace->column, index = 0;
                (x < size && x >= 0 ) && (y < size && y >= 0)&& index < wordToPlace->length;
                x += stepX, y += stepY, index++)
            {
                if (board[x][y].occupied == 1)
                {
                    if (GenerateOverlapWord(board, wordToPlace, &stepX, &stepY, size, x, y) == 1)
                    {
                        searching = 0;
                        PlaceWord(wordToPlace, board, &stepX, &stepY);
                        break;
                    }
                    //Overlap Cannot be generated
                    break;
                }
                else if (index == wordToPlace->length - 1) //We are at the end with 0 occupies
                {
                    searching = 0;
                    PlaceWord(wordToPlace, board, &stepX, &stepY);
                }
            }
        }


    }
}

struct Word* CompareToWordList(char* userInput, struct Word* wordList)
{
    for (int i = WORDS_TO_FIND_START; i < MAX_WORDS_PER; i++)
    {
        if (strncmpNoCase(userInput, wordList[i].word, strlen(wordList[i].word)) == 0)
        {
            if (wordList[i].found == 1)
            {
                printf("Word already found.\n");
                return NULL;
            }

            return &wordList[i];
        }
    }
    printf("Word does not exist in search list.\n");
    return NULL;
}


char CheckBoard(struct Letter** board, struct Word* word, int row, int column, int size)
{
    //Account for offset. Player Row 1 = System Row 0

    if (row > size || column > size)
    {
        printf("Word not found.\n");
        return 0;
    }

    if (word->row == row && word->column == column)
    {
        //User is correct!
        word->found = 1;
        
        int stepX;
        int stepY;

        GetStepValues(&word->pattern, &word->orientation, &stepX, &stepY);

        for (int x = row, y = column, i = 0; i < word->length; x+= stepX, y+= stepY, i++)
        {
            board[x][y].found = 1;
        }
        return 1;
    }
    printf("Word not found.\n");
    return 0;
}

void RevealWord(struct Letter** board, struct Word* wordList, int size)
{
    //Search for unfound word
    int index = ((float)rand() / RAND_MAX) * (MAX_WORDS_PER - WORDS_TO_FIND_START);
    struct Word* word;

    while (1)
    {
        if (wordList[index].found == 0)
        {
            word = &wordList[index];
            break;
        }

        index++;
        if (index >= MAX_WORDS_PER)
        {
            index = WORDS_TO_FIND_START;
        }
        
    }
    CheckBoard(board, word, word->row, word->column, size);

}

char CheckIfDone(struct Word* wordList)
{
    for (int i = WORDS_TO_FIND_START; i < MAX_WORDS_PER; i++)
    {
        if (wordList[i].found == 0)
        {
            return 0;
        }
    }

    return 1;
}

int main()
{
    int input;
    char inputBuffer[100];
    int returnVal;

    //Values for player to know when we found them all
    int wordCount = 0;
    int found = 0;

    srand(time(NULL));

    while (1)
    {
        printf("How large would you like your word find to be?\n20 = 20x20\n30 = 30x30\n");

        returnVal = fgets(&inputBuffer, 100, stdin);
        input = atoi(inputBuffer);
        system("cls");

        if (input == 30 || input == 20)
        {
            break;
        }
        else
        {
            printf("Please enter a valid size: 20 or 30.\n");
        }

    }

    //Get words from file
    FILE* wordFile = fopen("words.txt", "r");

    struct Letter** wordFind = GenerateWordFind(input);

    struct Word wordList[MAX_WORDS_PER];
    ParseWordFile(wordFile, input, wordList);

    system("cls");

    InsertWords(wordList, wordFind, input);

    while (1)
    {
        PrintWordList(wordList);
        PrintWordFind(wordFind, input);

        if (CheckIfDone(wordList))
        {
            printf("You found all the words!\n");
            break;
        }

        printf("\nEnter a word that you see. Type \"help\" to reveal a word and its location.\n");
        
        //FILL WITH INPUT GET
        //fgets(&inputBuffer, 100, stdin);
        returnVal = fgets(&inputBuffer, 100, stdin);
        struct Word* word = CompareToWordList(inputBuffer, wordList);

        if (strncmpNoCase(inputBuffer, "help\n", 6) == 0)
        {
            printf("Player used help command. Revealing random word\n");
            RevealWord(wordFind, wordList, input);
        }
        else if (word != NULL)
        {
            int row;
            int column;

            printf("Enter Row: ");
            returnVal = fgets(&inputBuffer, 100, stdin);
            row = atoi(inputBuffer);

            printf("Enter Column: ");
            returnVal = fgets(&inputBuffer, 100, stdin);
            column = atoi(inputBuffer);

            char correct = CheckBoard(wordFind, word, row - 1, column - 1, input);
            
            
        } 
        //IF - word typed is not found or exists in list
        //TRUE -> ASK FOR ROW AND THEN COLUMN

        //system("cls");
        //break;
        printf("\n");

        
    }
   
    //WordList does not need to be freed since it has a fixed size and malloc not used

    //Free wordFind
    for (int i = 0; i < input; i++)
    {
        free(wordFind[i]);
    }

    free(wordFind);

    printf("Enter anything to close window...");
    returnVal = fgets(&inputBuffer, 100, stdin);

}


