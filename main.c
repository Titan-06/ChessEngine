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
uint8_t lookup[240];
/* Structs for the game*/
struct gameState
{
    uint8_t player; /* 1 for white and 2 for black */
    int whiteScore;
    int blackScore;
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

int pointForPiece(uint8_t piece)
{
    switch (piece)
    {
    case PAWN:
        return 1;
    case ROOK:
        return 5;
    case KNIGHT:
        return 3;
    case BISHOP:
        return 3;
    case QUEEN:
        return 9;
    case KING:
        return 999;
    }
}

/* Translate the notation */
uint8_t signToPiece(char sign)
{
    switch (sign)
    {
    case 'N':
        return KNIGHT;
    case 'K':
        return KING;
    case 'Q':
        return QUEEN;
    case 'B':
        return BISHOP;
    case 'R':
        return ROOK;
    }
}
int destSquare(int *piece, char *move)
{
    int len = strlen(move);
    char file;
    char rank;
    switch (len)
    {
    case 2:
        file = move[0] - 97;
        rank = move[1] - 49;
        *piece = PAWN;
        return rankFileToBoard(rank, file);
    case 3:
        *piece = signToPiece(move[0]);
        file = move[1] - 97;
        rank = move[2] - 49;
        return rankFileToBoard(rank, file);
    default:
        break;
    }
}

/* Logic of the chess game */
void movePiece(char *move)
{
    int i = 119;
    int piece;
    int dest = destSquare(&piece, move);
    printf("%d\n", dest);
    while (i >= 0)
    {
        int dist = dest - i;
        printf("%d\n", dist);
        if ((lookup[dist + 119] & piece))
        {
            board[dest] = board[i];
            board[i] = 0;
            break;
        }

        if (i % 8 == 0)
        {
            i -= 8;
        }
        i--;
    }
}
/* Initializng the Game Board */
void initGameState()
{
    GAMESTATE.player = 2;
    GAMESTATE.blackScore = 0;
    GAMESTATE.whiteScore = 0;
}
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
void initLookup()
{
    for (int i = 0; i < 240; i++)
    {
        lookup[i] = 0;
        if (i == 33 || i - 119 == -33 || i == 31 || i - 119 == -31)
        {
            lookup[i] |= KNIGHT;
        }
        if (i % 16 == 0)
        {
            lookup[i] |= ROOK;
            lookup[i] |= QUEEN;
        }
        if ((i - 1) % 16 == 0 || (i + 1) % 16 == 0)
        {
            lookup[i] |= BISHOP;
            lookup[i] |= QUEEN;
        }
        if (i == 16 || i - 119 == -16 || i == 17 || i == 15 || i - 119 == -17 || i - 119 == -15)
        {
            lookup[i] |= PAWN;
            lookup[i] |= KING;
        }
    }
}

int main()
{
    char *s;
    initBoard();
    initLookup();
    printBoard();
    printf("AI: NO MOVEEEE\n");
    printf("YOUR MOVE:");
    scanf("%s", s);
    movePiece(s);
    printBoard();
    printf("%s\n", s);
    return 0;
}