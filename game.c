#include "game.h"

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
        if(check_attacked(desSquare,opponent) != -1) return 0;
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

int movePiece(int srcSquare, int desSquare) // Actually moves the piece on the board after checking if its valid or not
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
    calculatae_eval_scores();
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

// Genrate the all the legal moves at a particular notation
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

// If a player has any legal move 
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

// The Perft(DEBUG) function 
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

// The Main Move Function that translates the human LAN to move the piece
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