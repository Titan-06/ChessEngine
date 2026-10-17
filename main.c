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
uint8_t board[128];
uint8_t lookup[240];
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
/* Weights for all pieces*/
enum weights{
    kingWt = 200,
    queenWt = 9,
    rookWt = 5,
    bishopWt = 3,
    knightWt = 3,
    pawnWt = 1,

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


/* Helper functions to manage history */

void addMoveHistory(struct gameState gamedata, uint8_t board[128]){
    struct historyUnit *newUnit = malloc(sizeof(struct historyUnit));

    newUnit->gamestate = gamedata;
    memcpy(newUnit->board,board,128);

    struct historyUnit *temp = HISTORY.latest;

    HISTORY.latest = newUnit;
    newUnit->previous = temp;

    HISTORY.moveHistorySize++;

}
void popMoveHistory(){
    struct historyUnit *temp = HISTORY.latest->previous;
    free(HISTORY.latest);
    HISTORY.latest = temp;
    HISTORY.moveHistorySize--;
}
void undo_last_move(){
    GAMESTATE = HISTORY.latest->gamestate;
    memcpy(board,HISTORY.latest->board,128);
    popMoveHistory();
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
    default:
        return 0;
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
}
void calculatae_eval_scores(){
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
/* Logic of the chess game */
int square_obstruct(int src, int dest){
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
        if(src & 0x88){
            return -1;
        }
        if (board[src] != 0)
        {
            return src;
        }
        src += walk;
    }
    return -1;
}
int pathClear(int src, int dest)
{
    int squareObstruct = square_obstruct(src,dest);
    if(squareObstruct == -1){
        return 1;
    }
    return 0;
}
// PARAMETERS: Square which you want to check, Attacked by who?
int check_attacked(int src_square, int enemy_player){
    // To check if knight is attcking 
    int horseySteps[] = {33,-33,31,-31,-14,14,-18,18};
    for(int i = 0; i < 8; i++){
        int desSquare = src_square + horseySteps[i];
        
        if((board[desSquare] & KNIGHT) && (board[desSquare] & enemy_player)){
            return desSquare;
        }
    }
    // To check if the other pieces are attacking in all 8 dir
    int dir[] = {-17,17,15,-15,1,-1,16,-16};
    for(int i = 0; i < 8 ; i++){
        int destSquare = src_square + (dir[i] * 8);
        int squareObstruct = square_obstruct(src_square, destSquare);
        int distance = squareObstruct - src_square;
        if(squareObstruct != -1 && (board[squareObstruct] & enemy_player)){
            int piece = board[squareObstruct] & 0b11111100;
            switch(i){
                case 0:
                case 1:
                case 2:
                case 3:
                // We are dividing by 15 because 16 and 17 qwill then have the same result
                
                    if(piece == BISHOP || piece == QUEEN ){
                        return squareObstruct;
                    }
                    if( piece == KING && (distance / 15 == 1 || distance / 15 == -1)){
                        return squareObstruct;
                    }
                    if(piece == PAWN){
                        if(enemy_player == 1 && distance/15 == -1){
                            return squareObstruct;
                        }else if(enemy_player == 2 && distance/15 == 1){
                            return squareObstruct;
                        }
                    }
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    if(piece == ROOK || piece == QUEEN){
                        return squareObstruct;
                    }
                    if(piece == KING && (distance == 16 || distance == -16 || distance == 1 || distance == -1) ) return squareObstruct;
                    break;
                    
            }
        }
    }
    return -1;
}
int legalMove(int srcSquare, int desSquare) // Return 1 if it is , Return 0 if its not
{

    int distance = desSquare - srcSquare;
    int piece = board[srcSquare] & 0b11111100;

    if (lookup[distance + 119] & piece)
    {
        if(piece == PAWN){
            if(distance == 16 || distance == -16){
                return board[desSquare] == 0;
            }
            return 0;
        }
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
        return 0;
    }
}

int specialMove(int srcSquare, int desSquare)
{
    int distance = desSquare - srcSquare;

    int piece = board[srcSquare] & 0b11111100;
    int player = board[srcSquare] & 0b00000011;
    int opponent = (player == 1) ? 2: 1;
    int srcRank = boardToRank(srcSquare);
    int start_rank = (player == WHITE)? 1: 6; 

    switch (piece)
    {
    case PAWN:
        if ((distance == 32 || distance == -32) && (srcRank == start_rank) && board[srcSquare + distance/2] == 0 && board[desSquare]==0)
        { // Double Step of a pawn
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
            else if (GAMESTATE.enPassant_square != -1 && ((srcSquare - 1) == GAMESTATE.enPassant_square || (srcSquare + 1) == GAMESTATE.enPassant_square) && (boardToFile(GAMESTATE.enPassant_square) == boardToFile(desSquare)))
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
        if (distance == 2 && pathClear(srcSquare, srcSquare + 3) && GAMESTATE.players[player-1].castling_rights[0] == 1)
        {
            // King interchange
            if(check_attacked(desSquare-1,opponent) != -1) return 0;
            board[desSquare] = board[srcSquare];
            board[srcSquare] = 0;

            // Rook Interchange
            board[desSquare - 1] = board[srcSquare + 3];
            board[srcSquare + 3] = 0;
            return 1;
        }
        else if (distance == -2 && pathClear(srcSquare, srcSquare - 4) && GAMESTATE.players[player-1].castling_rights[1] == 1)
        {
            // King interchange
            if(check_attacked(desSquare+1,opponent) != -1) return 0;
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

/* Checking Check/Checkmate */


/* MOVEEEEE */
void evaluate_castling_rights(int piece,int player, int srcSquare){
    if (piece == KING)
    {
        GAMESTATE.players[player - 1].castling_rights[0] = 0;
        GAMESTATE.players[player - 1].castling_rights[1] = 0;
    }
    else if (piece == ROOK)
    {
        int file = boardToFile(srcSquare);
        if (file == 0)
        {
            GAMESTATE.players[player - 1].castling_rights[1] = 0;
        }
        else if (file == 7)
        {
            GAMESTATE.players[player - 1].castling_rights[0] = 0;
        }
    }
}

int movePiece(int srcSquare, int desSquare)
{
    addMoveHistory(GAMESTATE,board);
    int distance = desSquare - srcSquare;
    int piece = board[srcSquare] & 0b11111100;
    int playerSrc = board[srcSquare] & 0b00000011;
    // To not let the player move other piece or capture its own
    if (!(board[srcSquare] & GAMESTATE.current_player) || board[desSquare] & GAMESTATE.current_player)
    {
        return 0;
    }

    if (playerSrc == WHITE && distance < 0 && board[srcSquare] & PAWN)
    {
        return 0;
    }
    else if (playerSrc == BLACK && distance > 0 && board[srcSquare] & PAWN)
    {
        return 0;
    }
    if ((desSquare & 0x88)) // Check if the dest Square is OUT OF BOUNDS
    {
        return 0;
    }

    if (legalMove(srcSquare, desSquare))
    {
        board[desSquare] = board[srcSquare];
        board[srcSquare] = 0;
    }
    else if (!specialMove(srcSquare, desSquare)) // SpecialMove function also move the piece for us
    {
        return 0;
    }

    if((GAMESTATE.enPassant_square != -1) && !(board[GAMESTATE.enPassant_square] & playerSrc)){
        GAMESTATE.enPassant_square = -1;
    }

    if(piece == KING){
        GAMESTATE.players[GAMESTATE.current_player - 1 ].kingSquare = desSquare;
    }
    int previous_player = GAMESTATE.current_player;
    GAMESTATE.current_player = (GAMESTATE.current_player == 2) ? 1 : 2;
    GAMESTATE.moves++;

    // Move was made, now check if that put the king under check or nah
    int state = check_attacked(GAMESTATE.players[previous_player - 1 ].kingSquare,GAMESTATE.current_player);
    if(state != -1){
        return 0;
    }
    evaluate_castling_rights(piece,previous_player,srcSquare);
    calculatae_eval_scores();
    return piece;
}

int has_legal_move(int player_num){
    for(int src = 0; src < 128; src++){
        if((src & 0x88) || !(board[src] & player_num)) continue;
        for(int dest= 0; dest < 128; dest++){
            if((dest & 0x88) || dest == src) continue;
            int legal = movePiece(src,dest);
            undo_last_move();
            if(legal) return 1;
        }
    }
    return 0;
}
int evaluate_checkmate_stalemate(){ // 0 for nothing, 1 for checkmate, 2 for stalemate


    int me = GAMESTATE.current_player;
    int opponent = (me == 1)? 2 : 1;
    if(has_legal_move(me)) return 0;
    int inCheck = check_attacked(GAMESTATE.players[me -1].kingSquare,opponent);
    if(inCheck != -1){
        return 1;
    }else{
        return 2;
    }

}
int pawn_promotion_condition(int src,int des){
    int cell = board[src];
    int piece = cell & 0b11111100;
    int player = cell & 0b00000011;
    int rankDest = boardToRank(des);
    if(piece == PAWN && ((rankDest == 0 && player == BLACK) || (rankDest == 7 && player == WHITE))){
        return 1;
    }
    return 0;
}
// THE DEBUG PERFT FUNCTION

int generate_legal_moves(int src_moves[],int des_moves[]){
    int moves = 0;
    for(int src = 0; src < 128; src++){
        if((src & 0x88)) continue;
        for(int dest= 0; dest < 128; dest++){
            if((dest & 0x88) || dest == src) continue;
            int legal = movePiece(src,dest);
            undo_last_move();
            if(legal){
                src_moves[moves] = src;
                des_moves[moves] = dest;
                moves++;
            }
        }
    }
    return moves;
}

unsigned long long int perft(int depth){
    int src_moves[256];
    int des_moves[256];
    int n_moves,i;
    unsigned long long int nodes = 0;

    if(depth == 0){
        return 1ULL;
    }

    n_moves = generate_legal_moves(src_moves,des_moves);
    for(i = 0; i < n_moves; i++){
        int pawn_promotiuon = pawn_promotion_condition(src_moves[i],des_moves[i]);
        movePiece(src_moves[i],des_moves[i]);
        if(pawn_promotiuon){
            int player = board[des_moves[i]] & 0b00000011;
            int pieces[] = {QUEEN,KNIGHT,ROOK,BISHOP};
            for(int j = 0 ; j < 4; j++){
                board[des_moves[i]] = (pieces[j] | player);
                nodes += perft(depth -1);
            }
        }else{
            nodes += perft(depth -1);
        }
        undo_last_move();
        
    }
    return nodes;
}
/* Nega maxxing*/
int evaluate_score(){
    int square_wt = 6;
    int piece_wt = 10;

    int player = (GAMESTATE.current_player == 1) ? 1 : -1;

    int eval = (square_wt * GAMESTATE.square_score + piece_wt * GAMESTATE.piece_score) * player;

    return eval;
}
int negamax( int depth ){
    if(depth == 0) return evaluate_score();
    int max = -999999;
    int score = -999999;
    int src_moves[256];
    int des_moves[256];
    int n_moves = generate_legal_moves(src_moves,des_moves);

    for(int i = 0; i < n_moves ; i++){
        if(!(board[src_moves[i]] & GAMESTATE.current_player)) continue;

        int pawn_promotion = pawn_promotion_condition(src_moves[i],des_moves[i]);
        movePiece(src_moves[i],des_moves[i]);
        if(pawn_promotion){
            int pieces[] = {QUEEN,KNIGHT,ROOK,BISHOP};
            for(int j = 0 ; j < 4; j++){
                board[des_moves[i]] = (pieces[j] | GAMESTATE.current_player);
                score = -negamax(depth - 1);
                if(score > max) max = score;
            }
        }else{
            score = -negamax(depth - 1);
            if(score > max) max = score;
        }
       
        undo_last_move();
        
    }
    return max;
}
// The Main Move Function 
void make_move(char *s){
    int srcSquare;
    int desSquare;
    int translate = translateNotation(s, &srcSquare, &desSquare);
    int pawn_promotion = pawn_promotion_condition(srcSquare,desSquare);

    if(translate == 0){
        printf("Couldn't Translate\n");
        return;
    }
    int piece = movePiece(srcSquare,desSquare);
    if(!piece){
        undo_last_move();
        return;
    }else{
        if(pawn_promotion && translate > 1){
            int player = board[desSquare] & 0b00000011;
            board[desSquare] = (translate | player);
        }
        popMoveHistory();
        return;
    }
        
    
}
void generate_ai_move(int depth){
    int src_moves[256];
    int des_moves[256];

    int max = -999999;
    int score = -999999;

    int max_src_move = -1;
    int max_des_move = -1;
    int n_moves = generate_legal_moves(src_moves,des_moves);
    int promotion_piece = 0; // 0 for none, any other for whatever piece it was promoted to
    
    for(int i = 0 ; i < n_moves; i++){
        if(!(board[src_moves[i]] & GAMESTATE.current_player)) continue;

        for(int i = 0; i < n_moves ; i++){
            if(!(board[src_moves[i]] & GAMESTATE.current_player)) continue;

            int pawn_promotion = pawn_promotion_condition(src_moves[i],des_moves[i]);
            movePiece(src_moves[i],des_moves[i]);
            if(pawn_promotion){
                int pieces[] = {QUEEN,KNIGHT,ROOK,BISHOP};
                for(int j = 0 ; j < 4; j++){
                    board[des_moves[i]] = (pieces[j] | GAMESTATE.current_player);
                    score = -negamax(depth - 1);
                    if(score > max){ 
                        promotion_piece = pieces[j];
                        max = score;
                        max_src_move = src_moves[i];
                        max_des_move = des_moves[i];
                    };
                }
            }else{
                score = -negamax(depth - 1);
                if(score > max){
                    promotion_piece = 0; 
                    max = score;
                    max_src_move = src_moves[i];
                    max_des_move = des_moves[i];
                };
            }
        
            undo_last_move();
            
        }
    }

    movePiece(max_src_move,max_des_move);
    if(promotion_piece){
        board[max_des_move] = (promotion_piece | GAMESTATE.current_player);
    }
    popMoveHistory();
}
/* Initializng the Game Board */
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
// Lookup function includes all the moves that a piece can do ( excluding the Special moves )
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
    char s[16];
    initGameState();
    initBoard();
    initLookup();

    int mode = 1; // 0 for normal game and 1 against AI
    int player_yours = 0; // 0 for white and 1 for black (When playing aganst AI)

    /* Make a Loop here to move both */
    if(mode){
        //Play Against AI
        while (1)
        {
            printf("PERFT FUNCTION(DEBUG) NODES : %llu\n",perft(2));
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
                generate_ai_move(3);
            }
            
        }
    }else{
        while (1)
        {
            printf("PERFT FUNCTION(DEBUG) NODES : %llu\n",perft(2));
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
            printf("%d MOVE:", GAMESTATE.current_player);
            scanf("%s", s);
            make_move(s);
        }
    }
    
    return 0;
}