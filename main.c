#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* These represnts each bit of a byte, The last 2  represents black and white
    and the first 6 represnts each pieces*/

#define KING 128
#define QUEEN 64
#define BISHOP 32
#define KNIGHT 16
#define ROOK 8
#define PAWN 4

#define BLACK 2
#define WHITE 1

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
    for (int i = 0; i < 8; i++)
    {
        printf("\x1b[7m%d\x1b[0m ", i + 1);
        for (int j = 0; j < 8; j++)
        {
            int piece = board[(16 * i) + j] & 0b11111100;
            int player = board[(16 * i) + j] & ~(piece);
            char toPrint;
            switch (piece)
            {
            case PAWN:
                toPrint = 'p';
                break;
            case ROOK:
                toPrint = 'r';
                break;
            case KNIGHT:
                toPrint = 'n';
                break;
            case BISHOP:
                toPrint = 'b';
                break;
            case QUEEN:
                toPrint = 'q';
                break;
            case KING:
                toPrint = 'k';
                break;
            default:
                toPrint = '_';
                printf("%c ", toPrint);
                continue;
            }
            if (player == WHITE)
            {
                toPrint -= 32;
            }
            printf("%c ", toPrint);
        }

        printf("\x1b[7m%d\x1b[0m\n", i + 1);
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

/* Initializng the Game Board */
void initBoard()
{
    int backpieces[8] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
    int i = 0;
    while (i <= 119)
    {
        int player = (i < 64) ? WHITE : BLACK;
        if (i / 16 == 0 || i / 16 == 7)
        {
            board[i] = backpieces[i % 16] | player;
        }
        else if (i / 16 == 1 || i / 16 == 6)
        {
            board[i] = PAWN | player;
        }
        else
        {
            board[i] = 0;
        }
        if ((i + 1) % 8 == 0)
        {
            i += 8;
        }
        i++;
    }
}

int main()
{
    initBoard();
    GAMESTATE.player = 2;
    printBoard();
    return 0;
}