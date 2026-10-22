#include "game.h"

/* Helper functions to manage move history */

void save_last_move(struct gameState *prev_state, u_int8_t prev_board[128]){
    *prev_state = GAMESTATE;
    memcpy(prev_board,board,128);
}
void undo_last_move(struct gameState *prev_state, u_int8_t prev_board[128]){
    GAMESTATE = *prev_state;
    memcpy(board,prev_board,128);
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
    case 'p':
        return PAWN;
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

int square_obstruct(int src, int dest){ //Is the path obstructed by some piece, if then return that square
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

int pathClear(int src, int dest) // Wrapper function of SquareObstruct if a path is clear or not
{
    int squareObstruct = square_obstruct(src,dest);
    if(squareObstruct == -1){
        return 1;
    }
    return 0;
}


// PARAMETERS: Square which you want to check, Attacked by who?
int check_attacked(int src_square, int enemy_player){ //Checks if a square is attacked by the given player, if so returns the attacking square
    // To check if knight is attcking 
    int horseySteps[] = {33,-33,31,-31,-14,14,-18,18};
    for(int i = 0; i < 8; i++){
        int desSquare = src_square + horseySteps[i];
        
        if((in_bounds(desSquare))&&(board[desSquare] & KNIGHT) && (board[desSquare] & enemy_player)){
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


/* Functions for moving pieces legally */


int legalMove(int srcSquare, int desSquare) // Checks whether a piece can move taht distance
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

int specialMove(int srcSquare, int desSquare) // Check if the move is a special move
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
    case KING: // Castle
        if(check_attacked(desSquare,opponent) != -1 || check_attacked(srcSquare,opponent) != -1) return 0;
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

void evaluate_castling_rights(int piece,int player, int srcSquare){ // Just a check if a player still has catling rights or not after move 
    
    //Remove the opponent castling rights if the player captured their rook
    int opponent = (player == 1) ? 2 : 1;
    int oppenent_rookSquare = (opponent - 1) * 112;

    if(board[oppenent_rookSquare + 7] & player){ // Short Castle
        GAMESTATE.players[opponent - 1].castling_rights[0] = 0;
    }
    if(board[oppenent_rookSquare] & player){ // Long Castle
        GAMESTATE.players[opponent - 1].castling_rights[1] = 0;
    }

    if (piece == KING)
    {
        GAMESTATE.players[player - 1].castling_rights[0] = 0;
        GAMESTATE.players[player - 1].castling_rights[1] = 0;
    }
    else if (piece == ROOK)
    {
        int player_rookSquare = (player - 1) * 112;
        if (player_rookSquare == srcSquare)
        {
            GAMESTATE.players[player - 1].castling_rights[1] = 0;
        }
        else if (player_rookSquare + 7 == srcSquare)
        {
            GAMESTATE.players[player - 1].castling_rights[0] = 0;
        }
    }
}

int movePiece(int srcSquare, int desSquare) // Actually moves the piece on the board after checking if its valid or not
{
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

    if(!(piece == PAWN && (distance == 32 || distance == -32))){
        GAMESTATE.enPassant_square = -1;   // only a double step leaves an en passant square
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
    return piece;
}





// Checking....
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

// See if pawn can promote wrt to the src and des square of the pawn
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

// Genrate the all the legal moves at a particular notation for a player , 0 means both player
int generate_legal_moves(int src_moves[],int des_moves[],int player){
    int moves = 0;

    struct gameState temp_gamestate;
    u_int8_t temp_board[128];

    for(int src = 0; src < 128; src++){
        if((src & 0x88) || !board[src]) continue;
        if(player != 0 && !(board[src] & player)) continue;
        for(int dest= 0; dest < 128; dest++){
            if((dest & 0x88) || dest == src || !lookup[dest - src + 119]) continue;

            save_last_move(&temp_gamestate,temp_board);
            int legal = movePiece(src,dest);
            undo_last_move(&temp_gamestate,temp_board);

            if(legal){
                src_moves[moves] = src;
                des_moves[moves] = dest;
                moves++;
            }
        }
    }
    return moves;
}

// If a player has any legal move 
int has_legal_move(int player_num){

    struct gameState temp_gamestate;
    u_int8_t temp_board[128];

    for(int src = 0; src < 128; src++){
        if((src & 0x88) || !(board[src] & player_num)) continue;
        for(int dest= 0; dest < 128; dest++){
            if((dest & 0x88) || dest == src) continue;

            save_last_move(&temp_gamestate,temp_board);
            int legal = movePiece(src,dest);
            undo_last_move(&temp_gamestate,temp_board);
            if(legal) return 1;
        }
    }
    return 0;
}

// The Main Move Function that translates the human LAN to move the piece
void make_move(char *s){
    int srcSquare;
    int desSquare;
    int translate = translateNotation(s, &srcSquare, &desSquare);
    int pawn_promotion = pawn_promotion_condition(srcSquare,desSquare);

    struct gameState temp_gamestate;
    u_int8_t temp_board[128];

    if(translate == 0){
        printf("Couldn't Translate\n");
        return;
    }

    save_last_move(&temp_gamestate,temp_board);
    int piece = movePiece(srcSquare,desSquare);
    if(!piece){
        undo_last_move(&temp_gamestate,temp_board);
        calculate_eval_scores();
        return;
    }else{
        if(pawn_promotion && translate > 1){
            int player = board[desSquare] & 0b00000011;
            board[desSquare] = (translate | player);
        }
        calculate_eval_scores();
        return;
    }
    
        
    
}