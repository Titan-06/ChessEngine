#include "defs.h"
#include "game.h"
#include "engine.h"



/* Initializng the Game Board to play against AI*/
void initGameState()
{
    GAMESTATE.current_player = 1;
    GAMESTATE.piece_score = 0;
    GAMESTATE.square_score = 0;

    GAMESTATE.players[0].castling_rights[0] = 1;
    GAMESTATE.players[0].castling_rights[1] = 1;
    GAMESTATE.players[0].score = 0;
    GAMESTATE.players[0].kingSquare = 4;

    GAMESTATE.players[1].castling_rights[0] = 1;
    GAMESTATE.players[1].castling_rights[1] = 1;
    GAMESTATE.players[1].kingSquare = 116;

    GAMESTATE.moves = 0;
    GAMESTATE.enPassant_square = -1;
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

/* Lookup function includes all the moves that a piece can do ( excluding the Special moves ) */
void initLookup()
{
    for (int i = 0; i < 240; i++)
    {
        int dir_des = i - 119;
        lookup[i] = 0;
        if (dir_des == 33 || dir_des == -33 || dir_des == 31 || dir_des == -31 || dir_des == -14 || dir_des == 14 || dir_des == -18 || dir_des == 18)
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

int main(int argv, char *argc[])
{
    int mode = 1;
    int player_yours = 0; // 0 for white and 1 for black (When playing aganst AI)
    initLookup();

    if(argv != 2) return 0;
    if(argc[1][0] != '0' && argc[1][0] != '1') return 0;

    player_yours = argc[1][0] - '0';

    if(mode){
        char s[16];
        initGameState();
        initBoard();
        //Play Against AI
        while (1)
        {
            int state = evaluate_checkmate_stalemate();
            if(state == 1){
                printf("CHECKMATE\n");
                return 0;
            }else if(state == 2){
                printf("stalemate\n");
                return 0;
            }
            printBoard();
            if(GAMESTATE.current_player == (player_yours + 1)){
                printf("%d MOVE:", GAMESTATE.current_player);
                scanf("%s", s);
                make_move(s);
            }else{
                generate_ai_move(-1000000,1000000,5,0);
            }
            
        }
    }
    
    return 0;
}