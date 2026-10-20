#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


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
extern uint8_t board[128];
extern uint8_t lookup[240];

/* Structs for the game*/
struct player
{
    int score;
    int castling_rights[2]; /* 0 for short and 1 for long */
    int kingSquare;
};
struct gameState
{
    uint8_t current_player;   /* 1 for white and 2 for black */
    struct player players[2]; /* 0 for white and 1 for black*/
    int moves;
    int enPassant_square; /* Negative if not possible, otherwise location of some square*/
    int piece_score;
    int square_score;
};

extern struct gameState GAMESTATE;


#endif