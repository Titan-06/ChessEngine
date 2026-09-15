#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* In this program, board means 128 size array, game means the 8x8 chess board*/
uint8_t board[128];
/* Structs for the game*/
struct gameState
{
    uint8_t player; /* 1 for white and 2 for black */
};
struct gameState GAMESTATE;
/* To print the board*/
void printBoard()
{
    printf("\x1b[7m  A B C D E F G H  \x1b[0m\n");
    char s[80];
    if (GAMESTATE.player == 1)
    {
        for (int i = 1; i <= 8; i++)
        {
            snprintf(s, 80, "\x1b[7m%d\x1b[0m _ _ _ _ _ _ _ _ \x1b[7m%d\x1b[0m\n", i, i);
            printf("%s", s);
        }
    }
    else
    {
        for (int i = 8; i >= 1; i--)
        {
            snprintf(s, 80, "\x1b[7m%d\x1b[0m _ _ _ _ _ _ _ _ \x1b[7m%d\x1b[0m\n", i, i);
            printf("%s", s);
        }
    }
    printf("\x1b[7m  A B C D E F G H  \x1b[0m\n");
}
/* Helper functions for translating the board(0x88) */
int in_bounds(uint8_t des)
{
    if (des & 0x88)
    {
        return 0;
    }
    return 1;
}

int rankFileToBoard(uint8_t rank, uint8_t file)
{
    return (16 * rank) + file;
}

int boardToRank(uint8_t boardIndex)
{
    return boardIndex & 7;
}

int boardToFile(uint8_t boardIndex)
{
    return boardIndex >> 4;
}

int gameToBoard(uint8_t gameIndex)
{
    return gameIndex + (gameIndex & ~7);
}

int boardToGame(uint8_t boardIndex)
{
    return (boardIndex + (boardIndex & 7)) >> 1;
}

int main()
{
    GAMESTATE.player = 2;
    printBoard();
    return 0;
}