#ifndef GAME_H
#define GAME_H

#include "defs.h"
#include "engine.h"


/* Helper functions to manage history */
void save_last_move(struct gameState *prev_state, int prev_board[128]);
void undo_last_move(struct gameState *prev_state, int prev_board[128]);

/* Printing the board in the terminal */
void printBoard();

/* Checking if a square is atatcked by someone */
int check_attacked(int src_square, int enemy_player);

/* To move a piece to certain squae */
int movePiece(int srcSquare, int desSquare);
void make_move(char *s); // Wrapper function that translates the notation to actually do that move

/* To check if its checkmate/stalemate */
int evaluate_checkmate_stalemate();

/* To see if the condition for a pawn promotion has been met */
int pawn_promotion_condition(int src,int des);

/* Genrating legal moves */
int generate_legal_moves(int src_moves[],int des_moves[]);

/* Perft(DEBUG) function */
unsigned long long int perft(int depth);

int has_legal_move(int player_num);

/* Helper functions to translate the board */
int in_bounds(uint8_t des);
int rankFileToBoard(uint8_t rank, uint8_t file);
int boardToFile(uint8_t boardIndex);
int boardToRank(uint8_t boardIndex);
int gameToBoard(uint8_t gameIndex);
int boardToGame(uint8_t boardIndex);
uint8_t signToPiece(char sign);

#endif