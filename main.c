#include <stdio.h>
#include <stdint.h>

/* In this program, board means 128 size array, game means the 8x8 chess board*/
uint8_t board[128];

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
    printf("%d\n", in_bounds(0x07));
    return 0;
}