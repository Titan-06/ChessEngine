#include "defs.h"
#include "game.h"
#include "engine.h"



/* The FEN parser for testing purposes*/

void parse_fen(char *s){
    
    // Pre initializing some things...
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
                    gaps = character - '0';
                    for(int j = 0; j < gaps; j++){
                        board[rankFileToBoard(rank,file)] = 0;
                        file++;
                        continue;
                    }
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
    calculate_eval_scores();
}
// Testing different samples
void test_samples(){
    char *fen_samples[] = {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", //starting position 1
                        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", //Kiwipette 2
                        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", // Position 3 (Endgame-ish)
                        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", // Position 4
                        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", // Position 5 (The talkchess position)
                        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"}; // Position 6
    int num_samples = 6;

    for(int i = 0; i < num_samples; i++){
        printf("Starting %d position ~~~~",i+1);
        parse_fen(fen_samples[i]);
        generate_ai_move(-1000000,1000000,5,0);
        printf("NO: %d DONEEEE\n\n", i+1);

        memset(board, 0, sizeof(board)); // Clearing the board
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


// This UCI part is done by AI

/* Parse UCI 'position' command */
void parse_position(char *line)
{
    char *moves_ptr = strstr(line, "moves");

    if (strstr(line, "startpos"))
    {
        parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); //The starting position FEN
    }
    else if (strstr(line, "fen"))
    {
        char *fen_start = strstr(line, "fen") + 4;
        while (*fen_start == ' ') fen_start++;

        char fen_buffer[256];
        if (moves_ptr)
        {
            int len = moves_ptr - fen_start;
            if (len >= (int)sizeof(fen_buffer)) len = sizeof(fen_buffer) - 1;
            strncpy(fen_buffer, fen_start, len);
            fen_buffer[len] = '\0';
        }
        else
        {
            strncpy(fen_buffer, fen_start, sizeof(fen_buffer) - 1);
            fen_buffer[sizeof(fen_buffer) - 1] = '\0';
        }

        // Trim trailing newline or whitespace
        int l = strlen(fen_buffer);
        while (l > 0 && (fen_buffer[l - 1] == ' ' || fen_buffer[l - 1] == '\r' || fen_buffer[l - 1] == '\n'))
        {
            fen_buffer[--l] = '\0';
        }
        parse_fen(fen_buffer);
    }

    // Play move history
    if (moves_ptr)
    {
        moves_ptr += 5;
        char *token = strtok(moves_ptr, " \r\n\t");
        while (token)
        {
            make_move(token);
            token = strtok(NULL, " \r\n\t");
        }
    }
}

/* Synchronous UCI Loop */
void uci_loop()
{
    // Completely disable stdout buffering so GUI never hangs
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    char line[4096];
    while (fgets(line, sizeof(line), stdin))
    {
        // Strip trailing newline
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "uci") == 0)
        {
            printf("id name MyEngine\n");
            printf("id author Student\n");
            printf("uciok\n");
            fflush(stdout);
        }
        else if (strcmp(line, "isready") == 0)
        {
            printf("readyok\n");
            fflush(stdout);
        }
        else if (strcmp(line, "ucinewgame") == 0)
        {
            parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }
        else if (strncmp(line, "position", 8) == 0)
        {
            parse_position(line);
        }
        else if (strncmp(line, "go", 2) == 0)
        {
            int depth = 4; // Default depth if not specified
            char *depth_ptr = strstr(line, "depth");
            if (depth_ptr)
            {
                depth = atoi(depth_ptr + 6);
                if (depth <= 0) depth = 1;
            }
            generate_ai_move(-1000000,1000000,depth,1);
        }
        else if (strcmp(line, "quit") == 0)
        {
            break;
        }
    }
}


int main(int argc, char * argv[])
{
    initLookup();

    if(argc == 2 && !strcasecmp("test",argv[1])){
        test_samples();
        return 0;
    }

    uci_loop();
    return 0;
}