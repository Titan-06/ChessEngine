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




/* The FEN parser for testing purposes*/

void parse_fen(char *s){
    
    // Pre initializing some things...
    HISTORY.latest = NULL;
    HISTORY.moveHistorySize = 0;
    GAMESTATE.players[0].castling_rights[0] = 0;
    GAMESTATE.players[0].castling_rights[1] = 0;
    GAMESTATE.players[1].castling_rights[0] = 0;
    GAMESTATE.players[1].castling_rights[1] = 0;
    GAMESTATE.enPassant_square = -1;

    int i = 0;
    int sections = 1;
    int rank = 7; // One less to use our translator functions
    int file = 0;

    int gaps = 0;

    while(s[i] != '\0'){
        char character = s[i];
        if(character == ' '){
            sections++;
            i++;
            continue;
        }
        switch(sections){
            case 1:
            {
                int index = rankFileToBoard(rank,file);

                if(gaps > 0){
                    board[index] = 0;
                    gaps--;
                    file++;
                    continue;
                }
                if(character == '/'){
                    rank--;
                    file = 0;
                    i++;
                    continue;
                }

                

                gaps = 0;
                int player = 2;
                int piece = 0;
                
                if(character <= 90 && character >= 65){
                    player = 1;
                    character += 32;
                }else if(character <= 122 && character >= 97){
                    player = 2;

                }else{
                    gaps = character - 48;
                    i++;
                    continue;
                }
                
                piece = signToPiece(character);

                if(piece == KING){
                    GAMESTATE.players[player - 1].kingSquare = index;
                }

                board[index] = (player | piece);
                file++;
                break;
            }
            case 2:
                if(character == 'w'){
                    GAMESTATE.current_player = 1;
                }else{
                    GAMESTATE.current_player = 2;
                }
                break;
            case 3:
                switch (character)
                {
                case 'k':
                    GAMESTATE.players[1].castling_rights[0] = 1;
                    break;
                case 'q':
                    GAMESTATE.players[1].castling_rights[1] = 1;
                    break;
                case 'K':
                    GAMESTATE.players[0].castling_rights[0] = 1;
                    break;
                case 'Q':
                    GAMESTATE.players[0].castling_rights[1] = 1;
                    break;
                }
                break;
            case 4:
                if(s[i+1] - '6' == 0){
                    GAMESTATE.enPassant_square = rankFileToBoard(4,character - 'a');
                }else if(s[i+1] - '3' == 0){
                    GAMESTATE.enPassant_square = rankFileToBoard(3,character - 'a');
                }
                break;
        }
        i++;
    }
    calculatae_eval_scores();
}
// Testing different samples
void test_samples(){
    char *fen_samples[] = {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                           "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
                           "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
    int num_samples = 3;

    for(int i = 0; i < num_samples; i++){
        parse_fen(fen_samples[i]);
        printf("PERFT FUNCTION(DEBUG) NODES : %llu\n",perft(3));
        printBoard();
        generate_ai_move(3);
        printBoard();
        printf("NO: %d DONEEEE\n\n", i+1);
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

int main()
{
    srand(time(NULL));

    int mode = 0; // 0 for Testing(NPS and FEN) and 1 against AI
    int player_yours = 1; // 0 for white and 1 for black (When playing aganst AI)
    initLookup();

    
    if(mode){
        char s[16];
        initGameState();
        initBoard();
        //Play Against AI
        while (1)
        {
            printf("PERFT FUNCTION(DEBUG) NODES : %llu\n",perft(4));
            printf("Square Score: %d Piece Score : %d\n",GAMESTATE.square_score,GAMESTATE.piece_score);
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
                generate_ai_move(4);
            }
            
        }
    }else{
        test_samples();
    }
    
    return 0;
}