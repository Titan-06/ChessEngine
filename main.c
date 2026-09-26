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
struct player
{
    int score;
    int castling_rights[2]; /* 0 for short and 1 for long */
};
struct gameState
{
    uint8_t current_player;   /* 1 for white and 2 for black */
    struct player players[2]; /* 0 for white and 1 for black*/
    int moves;
    int enPassant_square; /* NEgative if not possible, otherwise location of some square*/
};
struct gameState GAMESTATE;

/* Structs to make the history */
struct historyUnit{
    struct gameState gamestate;
    uint8_t board[128];
    struct historyUnit *previous;
};

struct history{
    int moveHistorySize;
    struct historyUnit *latest;
};

struct history HISTORY;

/* Helper functions to manage history */

void addMoveHistory(struct gameState gamedata, uint8_t board[128]){
    struct historyUnit *newUnit = malloc(sizeof(struct historyUnit));

    newUnit->gamestate = gamedata;
    memcpy(newUnit,board,128 * sizeof(int));

    struct historyUnit *temp = HISTORY.latest;

    HISTORY.latest = newUnit;
    newUnit->previous = HISTORY.latest;

    HISTORY.moveHistorySize++;

}
void popMoveHistory(){
    struct historyUnit *temp = HISTORY.latest->previous;
    free(HISTORY.latest);
    HISTORY.latest = temp;
    HISTORY.moveHistorySize--;
}

/* To print the board*/
void printBoard()
{
    printf("\x1b[7m  A B C D E F G H  \x1b[0m\n");
    for (int i = 7; i >= 0; i--)
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

int boardToFile(uint8_t boardIndex)
{
    return boardIndex & 7;
}

int boardToRank(uint8_t boardIndex)
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

    printf("Src: %d Des: %d Player: %d\n", *src, *dest, GAMESTATE.current_player);

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

    int distance = desSquare - srcSquare;
    int piece = board[srcSquare] & 0b11111100;

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

int specialMove(int srcSquare, int desSquare)
{
    int distance = desSquare - srcSquare;
    int piece = board[srcSquare] & 0b11111100;

    int srcRank = boardToRank(srcSquare);
    printf("SPECIAL STEP %d %d \n", piece, srcRank);

    switch (piece)
    {
    case PAWN:
        if ((distance == 32 || distance == -32) && (srcRank == 1 || srcRank == 6))
        { // Double Step of a pawn
            printf("DOUBLE STEP\n");
            board[desSquare] = board[srcSquare];
            board[srcSquare] = 0;
            GAMESTATE.enPassant_square = desSquare;
            return 1;
        }
        else if (distance == -15 || distance == 15 || distance == 17 || distance == -17)
        {
            if (board[desSquare])
            { // Diagonal Capture
                board[desSquare] = board[srcSquare];
                board[srcSquare] = 0;
                return 1;
            }
            else if (GAMESTATE.enPassant_square >= 0 && (board[srcSquare - 1] == GAMESTATE.enPassant_square || board[srcSquare + 1] == GAMESTATE.enPassant_square))
            { // En Passant
                board[GAMESTATE.enPassant_square] = 0;
                board[desSquare] = board[srcSquare];
                board[srcSquare] = 0;

                GAMESTATE.enPassant_square = -1;
                return 1;
            }
        }
        break;
    case KING:
        if (distance == 2 && pathClear(srcSquare, srcSquare + 3))
        {
            // King interchange
            board[desSquare] = board[srcSquare];
            board[srcSquare] = 0;

            // Rook Interchange
            board[desSquare - 1] = board[srcSquare + 3];
            board[srcSquare + 3] = 0;
            return 1;
        }
        else if (distance == -2 && pathClear(srcSquare, srcSquare - 4))
        {
            // King interchange
            board[desSquare] = board[srcSquare];
            board[srcSquare] = 0;

            // Rook Interchange
            board[desSquare + 1] = board[srcSquare - 4];
            board[srcSquare - 4] = 0;
            return 1;
        }
        break;
    }
    return 0;
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

    int distance = desSquare - srcSquare;
    int piece = board[srcSquare] & 0b11111100;
    int playerSrc = board[srcSquare] & 0b00000011;
    // To not let the player move other piece or capture its own
    if (!(board[srcSquare] & GAMESTATE.current_player) || board[desSquare] & GAMESTATE.current_player)
    {
        printf("LMAO YOURN OWN SHI\n");
        return;
    }

    if (playerSrc == WHITE && distance < 0 && board[srcSquare] & PAWN)
    {
        printf("%d\n", board[srcSquare]);
        return;
    }
    else if (playerSrc == BLACK && distance > 0 && board[srcSquare] & PAWN)
    {
        printf("%d\n", board[srcSquare]);
        return;
    }
    if ((desSquare & 0x88)) // Check if the dest Square is OUT OF BOUNDS
    {
        printf("NOT inBOUNDS\n");
        return;
    }

    if (legalMove(srcSquare, desSquare))
    {
        board[desSquare] = board[srcSquare];
        board[srcSquare] = 0;
    }
    else if (!specialMove(srcSquare, desSquare))
    {
        printf("NOT SPECIAL MOVE EITHER\n");
    }

    /* Remove castling rights */
    if (piece == KING)
    {
        GAMESTATE.players[GAMESTATE.current_player - 1].castling_rights[0] = 0;
        GAMESTATE.players[GAMESTATE.current_player - 1].castling_rights[1] = 0;
    }
    else if (piece == ROOK)
    {
        int file = boardToFile(srcSquare);
        if (file == 0)
        {
            GAMESTATE.players[GAMESTATE.current_player - 1].castling_rights[1] = 0;
        }
        else if (file == 7)
        {
            GAMESTATE.players[GAMESTATE.current_player - 1].castling_rights[0] = 0;
        }
    }
    GAMESTATE.current_player = (GAMESTATE.current_player == 2) ? 1 : 2;
    GAMESTATE.moves++;
}
/* Initializng the Game Board */
void initGameState()
{
    GAMESTATE.current_player = 1;

    GAMESTATE.players[0].castling_rights[0] = 1;
    GAMESTATE.players[0].castling_rights[1] = 1;
    GAMESTATE.players[0].score = 0;

    GAMESTATE.players[1].castling_rights[0] = 1;
    GAMESTATE.players[1].castling_rights[1] = 1;
    GAMESTATE.players[1].score = 0;

    GAMESTATE.moves = 0;
    GAMESTATE.enPassant_square = -1;

    HISTORY.latest = NULL;
    HISTORY.moveHistorySize = 0;
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
// Lookup function includes all the moves that a piece can do ( excluding the Special moves )
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
    char s[16];
    initGameState();
    initBoard();
    initLookup();

    /* Make a Loop here to move both */
    while (1)
    {
        printBoard();
        printf("%d MOVE:", GAMESTATE.current_player);
        scanf("%s", s);
        movePiece(s);
    }
    return 0;
}