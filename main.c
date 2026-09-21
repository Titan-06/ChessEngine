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
    int moves;
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
    case 'n':
        return KNIGHT;
    case 'k':
        return KING;
    case 'q':
        return QUEEN;
    case 'b':
        return BISHOP;
    case 'r':
        return ROOK;
    }
}
int translateNotation(char *move, int *src, int *dest) // Returns 1 normally, 0 if some error and piece value if promotion
{
    int len = strlen(move);
    char file;
    char rank;

    // Square of the piece
    file = move[0] - 97;
    rank = move[1] - 49;
    *src = rankFileToBoard(rank, file);

    // Square to move to
    file = move[2] - 97;
    rank = move[3] - 49;
    *dest = rankFileToBoard(rank, file);

    printf("Src: %d Des: %d Next Player: %d\n", *src, *dest, GAMESTATE.player);

    if (len == 4)
    {
        return 1;
    }
    else if (len == 5)
    {
        return signToPiece(move[4]);
    }
    else
    {
        return 0;
    }
}

/* Logic of the chess game */
int pathClear(int src, int dest)
{
    int distance = dest - src;
    // Get the direction
    int dir = (distance < 0) ? -1 : 1;
    // Get the magnitude of the steps
    // 15 and 17 for the bishop
    int walk;
    if (distance % 15 == 0)
    {
        walk = dir * 15;
    }
    else if (distance % 16 == 0)
    {
        walk = dir * 16;
    }
    else if (distance % 17 == 0)
    {
        walk = dir * 17;
    }
    else
    {
        walk = dir;
    }
    // Walk in that dir and mag , return 0 if anything is encounterred
    src += walk;
    while (src != dest)
    {
        if (board[src] != 0)
        {
            return 0;
        }
        src += walk;
    }
    return 1;
}

int legalMove(int srcSquare, int desSquare) // Return 1 if it is , Return 0 if its not
{
    if ((desSquare & 0x88))
    {
        printf("NOT inBOUNDS\n");
        return 0;
    }

    int distance = desSquare - srcSquare;
    int piece = board[srcSquare] & 0b11111100;
    int playerSrc = board[srcSquare] & 0b00000011;

    // To not let the player move other piece or capture its own
    if (!(board[srcSquare] & GAMESTATE.player) || board[desSquare] & GAMESTATE.player)
    {
        return 0;
    }

    if (playerSrc == WHITE && distance < 0 && board[srcSquare] & PAWN)
    {
        printf("%d\n", board[srcSquare]);
        return 0;
    }
    else if (playerSrc == BLACK && distance > 0 && board[srcSquare] & PAWN)
    {
        printf("%d\n", board[srcSquare]);
        return 0;
    }

    if (lookup[distance + 119] & piece)
    {
        if (piece == QUEEN || piece == ROOK || piece == BISHOP)
        {
            if (pathClear(srcSquare, desSquare))
            {
                return 1;
            }
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        printf("NOT LOOKEDUP\n");
        return 0;
    }
}

void movePiece(char *move)
{
    int srcSquare;
    int desSquare;
    if (!translateNotation(move, &srcSquare, &desSquare))
    {
        printf("Couldn't Translate\n");
        return;
    }
    if (!legalMove(srcSquare, desSquare))
    {
        printf("NOT LEGAL\n");
        return;
    }
    board[desSquare] = board[srcSquare];
    board[srcSquare] = 0;

    GAMESTATE.player = (GAMESTATE.player == 2) ? 1 : 2;
    GAMESTATE.moves++;
}
/* Initializng the Game Board */
void initGameState()
{
    GAMESTATE.player = 1;
    GAMESTATE.blackScore = 0;
    GAMESTATE.whiteScore = 0;
    GAMESTATE.moves = 0;
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
        int dir_des = i - 119;
        lookup[i] = 0;
        if (dir_des == 33 || dir_des == -33 || dir_des == 31 || dir_des == -31)
        {
            lookup[i] |= KNIGHT;
        }
        if ((dir_des % 16 == 0) || (dir_des / 8 == 0))
        {
            lookup[i] |= ROOK;
            lookup[i] |= QUEEN;
        }
        if ((dir_des) % 15 == 0 || (dir_des) % 17 == 0)
        {
            lookup[i] |= BISHOP;
            lookup[i] |= QUEEN;
        }
        if (dir_des == 16 || dir_des == -16 || dir_des == 17 || dir_des == 15 || dir_des == -17 || dir_des == -15 || dir_des == 1 || dir_des == -1)
        {
            lookup[i] |= KING;
        }
        if (dir_des == 16 || dir_des == -16)
        {
            lookup[i] |= PAWN;
        }
    }
}

int main()
{
    char *s;
    initGameState();
    initBoard();
    initLookup();

    /* Make a Loop here to move both */
    while (1)
    {
        printBoard();
        printf("%d MOVE:", GAMESTATE.player);
        scanf("%s", s);
        movePiece(s);
    }
    return 0;
}