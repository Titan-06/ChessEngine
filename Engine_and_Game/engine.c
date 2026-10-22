#include "engine.h"

unsigned long long int nodes = 0;

/* Weights for all pieces*/
enum weights{
    kingWt = 999,
    queenWt = 90,
    rookWt = 50,
    bishopWt = 30,
    knightWt = 30,
    pawnWt = 10,

};
/* Weight for the squares in each piece */

// PAWN: Encourages pushing central pawns and marching toward promotion.
const int pawn_pst[64] = {
     0,  0,  0,  0,  0,  0,  0,  0, // Rank 1
     5, 10, 10,-20,-20, 10, 10,  5, // Rank 2
     5, -5,-10,  0,  0,-10, -5,  5, // Rank 3
     0,  0,  0, 20, 20,  0,  0,  0, // Rank 4
     5,  5, 10, 25, 25, 10,  5,  5, // Rank 5
    10, 10, 20, 30, 30, 20, 10, 10, // Rank 6
    50, 50, 50, 50, 50, 50, 50, 50, // Rank 7
     0,  0,  0,  0,  0,  0,  0,  0  // Rank 8 (Promoted)
};

// KNIGHT: Heavy penalties on the edge, massive bonus in the center.
const int knight_pst[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

// BISHOP: Wants to sit on long central diagonals.
const int bishop_pst[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

// ROOK: Minor bonus for central files, major bonus for reaching the 7th rank.
const int rook_pst[64] = {
      0,  0,  0,  5,  5,  0,  0,  0,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      5, 10, 10, 10, 10, 10, 10,  5, // 7th Rank infiltration
      0,  0,  0,  0,  0,  0,  0,  0
};

// QUEEN: Prefers not to develop too early, stays slightly centralized.
const int queen_pst[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

// KING (Middlegame): Extreme penalty for wandering to the center; bonuses for castling.
const int king_pst[64] = {
     20, 30, 10,  0,  0, 10, 30, 20, // High bonus at c1 and g1
     20, 20,  0,  0,  0,  0, 20, 20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30
};


// The Evaluation functions 
int piece_weight(int piece){
    switch (piece)
    {
    case KING:
        return kingWt;
    case QUEEN:
        return queenWt;
    case BISHOP:
        return bishopWt;
    case KNIGHT:
        return knightWt;
    case ROOK:
        return rookWt;
    case PAWN:
        return pawnWt;
    
    }
    return 0;
}

int square_weight(int piece, int square){
    switch (piece)
    {
    case KING:
        return king_pst[square];
    case QUEEN:
        return queen_pst[square];
    case BISHOP:
        return bishop_pst[square];
    case KNIGHT:
        return knight_pst[square];
    case ROOK:
        return rook_pst[square];
    case PAWN:
        return pawn_pst[square];
    
    }
    return 0;
}

void calculate_eval_scores(){
    GAMESTATE.piece_score = 0;
    GAMESTATE.square_score = 0;
    for(int i = 0; i < 120; i++){
        if(i & 0x88 || board[i] == 0) continue;
        int cell = board[i];
        int piece = cell & 0b11111100;
        int player = cell & 0b00000011;
        player = player == 2 ? -1 : 1;
        int index = boardToGame(i);

        // Calculate the piece Score
        GAMESTATE.piece_score += piece_weight(piece) * player;

        // Calculate the Square score
        if(player == 1){
            GAMESTATE.square_score += square_weight(piece,index);
        }else if(player == -1){
            GAMESTATE.square_score += square_weight(piece, 63 - index) * player;
        }
    }
}


/* The Evaluation Function , Materials are REALLY important */
int evaluate_score(){
    int square_wt = 5;
    int piece_wt = 100;

    int player = (GAMESTATE.current_player == 1) ? 1 : -1;

    int eval = (square_wt * GAMESTATE.square_score + piece_wt * GAMESTATE.piece_score) * player;

    return eval;
}

/* Most functions below here foloow the more or less the same pattern */

// Quisecence Search
int quisece(int alpha, int beta, int qdepth){ // Adding a qdepth to prevent move explosion
    nodes++;

    calculate_eval_scores();
    int static_eval = evaluate_score();

    int best_value = static_eval;
    if(best_value >= beta){
        return best_value;
    }
    // Delta pruning 
    int big_delta = 9500; // The weight of queen (Dont look further when already down a lot of points)
    if(static_eval < alpha - big_delta){
        return alpha;
    }

    if(best_value > alpha){
        alpha = best_value;
    }

    if(qdepth <= 0){
        return alpha;
    }

    int score = -999999;
    int src_moves[256];
    int des_moves[256];
    struct gameState temp_gamestate;
    u_int8_t temp_board[128];


    int n_moves = generate_legal_moves(src_moves,des_moves,GAMESTATE.current_player,1);
    for(int i = 0; i < n_moves; i++){

        int pawn_promotion = pawn_promotion_condition(src_moves[i],des_moves[i]);
        int player_this_move = GAMESTATE.current_player;

        save_last_move(&temp_gamestate,temp_board);
        movePiece(src_moves[i],des_moves[i]);

        if(pawn_promotion){
                board[des_moves[i]] = (QUEEN | player_this_move);
                score = -quisece(-beta,-alpha,qdepth-1);
                if(score >= beta){
                    undo_last_move(&temp_gamestate,temp_board);
                    return score;
                }
                if(score > best_value){
                    best_value = score;
                }
                if(score >= alpha){
                    alpha = score;
                }
            
        }else{
            score = -quisece(-beta,-alpha,qdepth-1);
            if(score >= beta){
                undo_last_move(&temp_gamestate,temp_board);
                return score;
            }
            if(score > best_value){
                best_value = score;
            }
            if(score >= alpha){
                alpha = score;
            }
        }
       
        undo_last_move(&temp_gamestate,temp_board);
    }
    return best_value;
}

// Negamax Framework -- Alpha beta pruning 
int alphaBeta(int alpha, int beta, int depth){
    nodes++;

    if(depth == 0){
        return quisece(alpha,beta,2);
    }
    
    int bestValue = -1000000;
    int score = -999999;
    int src_moves[256];
    int des_moves[256];


    struct gameState temp_gamestate;
    u_int8_t temp_board[128];

    int n_moves = generate_legal_moves(src_moves,des_moves,GAMESTATE.current_player,0);
    if(n_moves == 0){
        int player = GAMESTATE.current_player;
        int opponent = (player == 1) ? 2: 1;
        if(check_attacked(GAMESTATE.players[player -1].kingSquare,opponent) != -1){
            return -900000 - depth; // faster checkmates are preferrred
        }
        return 0; // Stalemate
    }
    for(int i = 0; i < n_moves; i++){
        int pawn_promotion = pawn_promotion_condition(src_moves[i],des_moves[i]);;
        int player_this_move = GAMESTATE.current_player;
        save_last_move(&temp_gamestate,temp_board);
        movePiece(src_moves[i],des_moves[i]);

        if(pawn_promotion){
            int pieces[] = {QUEEN,KNIGHT,ROOK,BISHOP};
            for(int j = 0 ; j < 4; j++){
                board[des_moves[i]] = (pieces[j] | player_this_move);
                score = -alphaBeta(-beta,-alpha,depth-1);
                if(score > bestValue){
                    bestValue = score;

                    if(score > alpha) alpha = score;

                }
                if(score >= beta){
                    undo_last_move(&temp_gamestate,temp_board);
                    return bestValue;
                }
            }
        }else{
            score = -alphaBeta(-beta,-alpha,depth - 1);
            if(score > bestValue){
                bestValue = score;

                if(score > alpha) alpha = score;

            }
            if(score >= beta){
                undo_last_move(&temp_gamestate,temp_board);
                return bestValue;
            }
        }
       
        undo_last_move(&temp_gamestate,temp_board);
    }
    return bestValue;

}
void generate_ai_move(int alpha,int beta,int depth, int playing_or_testing){ // 0 for testing , 1 for Playing
    int src_moves[256];
    int des_moves[256];

    int bestValue = -1000000;
    int score = -999999;

    int max_src_move = -1;
    int max_des_move = -1;
    int n_moves = generate_legal_moves(src_moves,des_moves,GAMESTATE.current_player,0);
    int promotion_piece = 0; // 0 for none, any other for whatever piece it was promoted to
    
    struct gameState temp_gamestate;
    u_int8_t temp_board[128];
    
    // For testing part
    clock_t start_time = clock();;
    nodes = 0;

    for(int i = 0; i < n_moves; i++){
        int pawn_promotion = pawn_promotion_condition(src_moves[i],des_moves[i]);;
        int player_this_move = GAMESTATE.current_player;
        save_last_move(&temp_gamestate,temp_board);
        movePiece(src_moves[i],des_moves[i]);

        if(pawn_promotion){
            int pieces[] = {QUEEN,KNIGHT,ROOK,BISHOP};
            for(int j = 0 ; j < 4; j++){
                board[des_moves[i]] = (pieces[j] | player_this_move);
                score = -alphaBeta(-beta,-alpha,depth-1);
                if(score > bestValue){
                    bestValue = score;
                    max_src_move = src_moves[i];
                    max_des_move = des_moves[i];
                    promotion_piece = pieces[j];
                    if(score > alpha) alpha = score;

                }
                if(score >= beta){
                    undo_last_move(&temp_gamestate,temp_board);
                    break;
                }
            }
        }else{
            score = -alphaBeta(-beta,-alpha,depth-1);
            if(score > bestValue){
                bestValue = score;
                max_src_move = src_moves[i];
                max_des_move = des_moves[i];
                promotion_piece = 0;
                if(score > alpha) alpha = score;

            }
            if(score >= beta){
                undo_last_move(&temp_gamestate,temp_board);
                break;
            }
        }
       
        undo_last_move(&temp_gamestate,temp_board);
    }
    clock_t end_time = clock();
        
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Move found: Nodes Searched: %llu Time spend:%.3f NPS: %.5f \n",nodes,time_spent,nodes/time_spent);

    movePiece(max_src_move,max_des_move);
    if(promotion_piece){
        int mover = board[max_des_move] & 0b00000011;
        board[max_des_move] = (promotion_piece | mover);
    }
    calculate_eval_scores();
}