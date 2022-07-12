#include "Header1.h"
#include "Source1.cpp"
#include "Chess_board.cpp"

// FEN dedug positions
#define empty_board "8/8/8/8/8/8/8/8 w - - "

#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "


namespace Time
{
    // exit from engine flag
    int quit = 0;

    // UCI "movestogo" command moves counter
    int movestogo = 30;

    // UCI "movetime" command time counter
    int movetime = -1;

    // UCI "time" command holder (ms)
    int time = -1;

    // UCI "inc" command's time increment holder
    int inc = 0;

    // UCI "starttime" command time holder
    int starttime = 0;

    // UCI "stoptime" command time holder
    int stoptime = 0;

    // variable to flag time control availability
    int timeset = 0;

    // variable to flag when the time is up
    int stopped = 0;

    // get time in milliseconds
    int get_time_ms()
    {
        auto millisec_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        return millisec_since_epoch;
    }


    int input_waiting()
    {
#ifndef WIN32
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);
        tv.tv_sec = 0; tv.tv_usec = 0;
        select(16, &readfds, 0, 0, &tv);

        return (FD_ISSET(fileno(stdin), &readfds));
#else
        static int init = 0, pipe;
        static HANDLE inh;
        DWORD dw;

        if (!init)
        {
            init = 1;
            inh = GetStdHandle(STD_INPUT_HANDLE);
            pipe = !GetConsoleMode(inh, &dw);
            if (!pipe)
            {
                SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
                FlushConsoleInputBuffer(inh);
            }
        }

        if (pipe)
        {
            if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
            return dw;
        }

        else
        {
            GetNumberOfConsoleInputEvents(inh, &dw);
            return dw <= 1 ? 0 : dw;
        }

#endif
    }

    // read GUI/user input
    void read_input()
    {
        // bytes to read holder
        int bytes;

        // GUI/user input
        char input[256] = "", * endc;

        // "listen" to STDIN
        if (input_waiting())
        {
            // tell engine to stop calculating
            stopped = 1;

            // loop to read bytes from STDIN
            do
            {
                // read bytes from STDIN
                bytes = _read(_fileno(stdin), input, 256);
            }

            // until bytes available
            while (bytes < 0);

            // searches for the first occurrence of '\n'
            endc = strchr(input, '\n');

            // if found new line set value at pointer to 0
            if (endc) *endc = 0;

            // if input is available
            if (strlen(input) > 0)
            {
                // match UCI "quit" command
                if (!strncmp(input, "quit", 4))
                {
                    // tell engine to terminate exacution    
                    quit = 1;
                }

                // // match UCI "stop" command
                else if (!strncmp(input, "stop", 4)) {
                    // tell engine to terminate exacution
                    quit = 1;
                }
            }
        }
    }

    // a bridge function to interact between search and GUI input
    static void communicate() {
        // if time is up break here
        if (timeset == 1 && get_time_ms() > stoptime) {
            // tell engine to stop calculating
            stopped = 1;
        }

        // read GUI input
        read_input();
    }



}


namespace Perft {

    long nodes;

    // perft driver
    void perft_driver(int depth)
    {
        // reccursion escape condition
        if (depth == 0)
        {
            // increment nodes count (count reached positions)
            nodes++;
            return;
        }

        // create move list instance
        std::vector<int> move_list;

        // generate moves
        MoveGeneration::generate_moves(move_list);

        // loop over generated moves
        for (int move_count = 0; move_count < move_list.size(); move_count++)
        {
            // preserve board state
            copy_board();

            // make move
            if (!make_move(move_list[move_count], MoveGeneration::all_moves))
                // skip to the next move
                continue;

            // call perft driver recursively
            perft_driver(depth - 1);

            // take back
            take_back();
        }
    }

    // perft test
    void perft_test(int depth)
    {
        std::cout<<"\n     Performance test\n\n";

        // create move list instance
        std::vector<int> move_list;

        // generate moves
        MoveGeneration::generate_moves(move_list);


         // init start time
        long start = Time::get_time_ms();

        // loop over generated moves
        for (int move_count = 0; move_count < move_list.size(); move_count++)
        {
            // preserve board state
            copy_board();

            // make move
            if (!MoveGeneration::make_move(move_list[move_count], MoveGeneration::all_moves))
                // skip to the next move
                continue;

            // cummulative nodes
            long cummulative_nodes = nodes;

            // call perft driver recursively
            perft_driver(depth - 1);

            // old nodes
            long old_nodes = nodes - cummulative_nodes;

            // take back
            take_back();

            // print move
            std::cout<<"     move: "<< square_to_coordinates[get_move_source(move_list[move_count])]<<
                square_to_coordinates[get_move_target(move_list[move_count])]<<
                (get_move_promoted(move_list[move_count]) ? MoveGeneration::promoted_pieces[get_move_promoted(move_list[move_count])] : ' ')<<
                "      nodes:  "<<old_nodes<<"\n";
        }

        // print results
        std::cout << "\n    Depth: " << depth << "\n";
        std::cout << "    Nodes: " << nodes << "\n\n";
        std::cout << "    Time: " << Time::get_time_ms() - start  << "\n";
        
    }
}


namespace Evaluation {

    // material scrore

/*
    ? =   100   = ?
    ? =   300   = ? * 3
    ? =   350   = ? * 3 + ? * 0.5
    ? =   500   = ? * 5
    ? =   1000  = ? * 10
    ? =   10000 = ? * 100

*/

// material score [game phase][piece]
    const int material_score[2][12] =
    {
        // opening material score
        82, 337, 365, 477, 1025, 12000, -82, -337, -365, -477, -1025, -12000,

        // endgame material score
        94, 281, 297, 512,  936, 12000, -94, -281, -297, -512,  -936, -12000
    };

    // game phase scores
    const int opening_phase_score = 6192;
    const int endgame_phase_score = 518;

    // game phases
    enum { opening, endgame, middlegame };

    // piece types
    enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

    // positional piece scores [game phase][piece][square]
    const int positional_score[2][6][64] =

        // opening positional piece scores //
    {
        //pawn
        0,   0,   0,   0,   0,   0,  0,   0,
        98, 134,  61,  95,  68, 126, 34, -11,
        -6,   7,  26,  31,  65,  56, 25, -20,
        -14,  13,   6,  21,  23,  12, 17, -23,
        -27,  -2,  -5,  12,  17,   6, 10, -25,
        -26,  -4,  -4, -10,   3,   3, 33, -12,
        -35,  -1, -20, -23, -15,  24, 38, -22,
        0,   0,   0,   0,   0,   0,  0,   0,

        // knight
        -167, -89, -34, -49,  61, -97, -15, -107,
        -73, -41,  72,  36,  23,  62,   7,  -17,
        -47,  60,  37,  65,  84, 129,  73,   44,
        -9,  17,  19,  53,  37,  69,  18,   22,
        -13,   4,  16,  13,  28,  19,  21,   -8,
        -23,  -9,  12,  10,  19,  17,  25,  -16,
        -29, -53, -12,  -3,  -1,  18, -14,  -19,
        -105, -21, -58, -33, -17, -28, -19,  -23,

        // bishop
        -29,   4, -82, -37, -25, -42,   7,  -8,
        -26,  16, -18, -13,  30,  59,  18, -47,
        -16,  37,  43,  40,  35,  50,  37,  -2,
        -4,   5,  19,  50,  37,  37,   7,  -2,
        -6,  13,  13,  26,  34,  12,  10,   4,
        0,  15,  15,  15,  14,  27,  18,  10,
        4,  15,  16,   0,   7,  21,  33,   1,
        -33,  -3, -14, -21, -13, -12, -39, -21,

        // rook
        32,  42,  32,  51, 63,  9,  31,  43,
        27,  32,  58,  62, 80, 67,  26,  44,
        -5,  19,  26,  36, 17, 45,  61,  16,
        -24, -11,   7,  26, 24, 35,  -8, -20,
        -36, -26, -12,  -1,  9, -7,   6, -23,
        -45, -25, -16, -17,  3,  0,  -5, -33,
        -44, -16, -20,  -9, -1, 11,  -6, -71,
        -19, -13,   1,  17, 16,  7, -37, -26,

        // queen
        -28,   0,  29,  12,  59,  44,  43,  45,
        -24, -39,  -5,   1, -16,  57,  28,  54,
        -13, -17,   7,   8,  29,  56,  47,  57,
        -27, -27, -16, -16,  -1,  17,  -2,   1,
        -9, -26,  -9, -10,  -2,  -4,   3,  -3,
        -14,   2, -11,  -2,  -5,   2,  14,   5,
        -35,  -8,  11,   2,   8,  15,  -3,   1,
        -1, -18,  -9,  10, -15, -25, -31, -50,

        // king
        -65,  23,  16, -15, -56, -34,   2,  13,
        29,  -1, -20,  -7,  -8,  -4, -38, -29,
        -9,  24,   2, -16, -20,   6,  22, -22,
        -17, -20, -12, -27, -30, -25, -14, -36,
        -49,  -1, -27, -39, -46, -44, -33, -51,
        -14, -14, -22, -46, -44, -30, -15, -27,
        1,   7,  -8, -64, -43, -16,   9,   8,
        -15,  36,  12, -54,   8, -28,  24,  14,


        // Endgame positional piece scores //

        //pawn
        0,   0,   0,   0,   0,   0,   0,   0,
        178, 173, 158, 134, 147, 132, 165, 187,
        94, 100,  85,  67,  56,  53,  82,  84,
        32,  24,  13,   5,  -2,   4,  17,  17,
        13,   9,  -3,  -7,  -7,  -8,   3,  -1,
        4,   7,  -6,   1,   0,  -5,  -1,  -8,
        13,   8,   8,  10,  13,   0,   2,  -7,
        0,   0,   0,   0,   0,   0,   0,   0,

        // knight
        -58, -38, -13, -28, -31, -27, -63, -99,
        -25,  -8, -25,  -2,  -9, -25, -24, -52,
        -24, -20,  10,   9,  -1,  -9, -19, -41,
        -17,   3,  22,  22,  22,  11,   8, -18,
        -18,  -6,  16,  25,  16,  17,   4, -18,
        -23,  -3,  -1,  15,  10,  -3, -20, -22,
        -42, -20, -10,  -5,  -2, -20, -23, -44,
        -29, -51, -23, -15, -22, -18, -50, -64,

        // bishop
        -14, -21, -11,  -8, -7,  -9, -17, -24,
        -8,  -4,   7, -12, -3, -13,  -4, -14,
        2,  -8,   0,  -1, -2,   6,   0,   4,
        -3,   9,  12,   9, 14,  10,   3,   2,
        -6,   3,  13,  19,  7,  10,  -3,  -9,
        -12,  -3,   8,  10, 13,   3,  -7, -15,
        -14, -18,  -7,  -1,  4,  -9, -15, -27,
        -23,  -9, -23,  -5, -9, -16,  -5, -17,

        // rook
        13, 10, 18, 15, 12,  12,   8,   5,
        11, 13, 13, 11, -3,   3,   8,   3,
        7,  7,  7,  5,  4,  -3,  -5,  -3,
        4,  3, 13,  1,  2,   1,  -1,   2,
        3,  5,  8,  4, -5,  -6,  -8, -11,
        -4,  0, -5, -1, -7, -12,  -8, -16,
        -6, -6,  0,  2, -9,  -9, -11,  -3,
        -9,  2,  3, -1, -5, -13,   4, -20,

        // queen
        -9,  22,  22,  27,  27,  19,  10,  20,
        -17,  20,  32,  41,  58,  25,  30,   0,
        -20,   6,   9,  49,  47,  35,  19,   9,
        3,  22,  24,  45,  57,  40,  57,  36,
        -18,  28,  19,  47,  31,  34,  39,  23,
        -16, -27,  15,   6,   9,  17,  10,   5,
        -22, -23, -30, -16, -16, -23, -36, -32,
        -33, -28, -22, -43,  -5, -32, -20, -41,

        // king
        -74, -35, -18, -18, -11,  15,   4, -17,
        -12,  17,  14,  17,  17,  38,  23,  11,
        10,  17,  23,  15,  20,  45,  44,  13,
        -8,  22,  24,  27,  26,  33,  26,   3,
        -18,  -4,  21,  24,  27,  23,   9, -11,
        -19,  -3,  11,  21,  23,  16,   7,  -9,
        -27, -11,   4,  13,  14,   4,  -5, -17,
        -53, -34, -21, -11, -28, -14, -24, -43
    };


    // mirror positional score tables for opposite side
    const int mirror_score[128] =
    {
        a1, b1, c1, d1, e1, f1, g1, h1,
        a2, b2, c2, d2, e2, f2, g2, h2,
        a3, b3, c3, d3, e3, f3, g3, h3,
        a4, b4, c4, d4, e4, f4, g4, h4,
        a5, b5, c5, d5, e5, f5, g5, h5,
        a6, b6, c6, d6, e6, f6, g6, h6,
        a7, b7, c7, d7, e7, f7, g7, h7,
        a8, b8, c8, d8, e8, f8, g8, h8
    };

    /*
          Rank mask            File mask           Isolated mask        Passed pawn mask
        for square a6        for square f2         for square g2          for square c4
    8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 1 0 0    8  0 0 0 0 0 1 0 1     8  0 1 1 1 0 0 0 0
    7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 1 0 0    7  0 0 0 0 0 1 0 1     7  0 1 1 1 0 0 0 0
    6  1 1 1 1 1 1 1 1    6  0 0 0 0 0 1 0 0    6  0 0 0 0 0 1 0 1     6  0 1 1 1 0 0 0 0
    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 1 0 0    5  0 0 0 0 0 1 0 1     5  0 1 1 1 0 0 0 0
    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 1 0 0    4  0 0 0 0 0 1 0 1     4  0 0 0 0 0 0 0 0
    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 1 0 0    3  0 0 0 0 0 1 0 1     3  0 0 0 0 0 0 0 0
    2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 1 0 0    2  0 0 0 0 0 1 0 1     2  0 0 0 0 0 0 0 0
    1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 1 0 0    1  0 0 0 0 0 1 0 1     1  0 0 0 0 0 0 0 0
       a b c d e f g h       a b c d e f g h       a b c d e f g h        a b c d e f g h
*/

// file masks [square]
    U64 file_masks[64];

    // rank masks [square]
    U64 rank_masks[64];

    // isolated pawn masks [square]
    U64 isolated_masks[64];

    // white passed pawn masks [square]
    U64 white_passed_masks[64];

    // black passed pawn masks [square]
    U64 black_passed_masks[64];

    // extract rank from a square [square]
    const int get_rank[64] =
    {
        7, 7, 7, 7, 7, 7, 7, 7,
        6, 6, 6, 6, 6, 6, 6, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    // double pawns penalty
    const int double_pawn_penalty = -10;

    // isolated pawn penalty
    const int isolated_pawn_penalty = -10;

    // passed pawn bonus
    const int passed_pawn_bonus[8] = { 0, 10, 30, 50, 75, 100, 150, 200 };

    // semi open file score
    const int semi_open_file_score = 10;

    // open file score
    const int open_file_score = 15;

    // king's shield bonus
    const int king_shield_bonus = 5;

    // set file or rank mask
    U64 set_file_rank_mask(int file_number, int rank_number)
    {
        // file or rank mask
        U64 mask = 0ULL;

        // loop over ranks
        for (int rank = 0; rank < 8; rank++)
        {
            // loop over files
            for (int file = 0; file < 8; file++)
            {
                // init square
                int square = rank * 8 + file;

                if (file_number != -1)
                {
                    // on file match
                    if (file == file_number)
                        // set bit on mask
                        mask |= set_bit(mask, square);
                }

                else if (rank_number != -1)
                {
                    // on rank match
                    if (rank == rank_number)
                        // set bit on mask
                        mask |= set_bit(mask, square);
                }
            }
        }

        // return mask
        return mask;
    }

    // init evaluation masks
    void init_evaluation_masks()
    {
        /******** Init file masks ********/

        // loop over ranks
        for (int rank = 0; rank < 8; rank++)
        {
            // loop over files
            for (int file = 0; file < 8; file++)
            {
                // init square
                int square = rank * 8 + file;

                // init file mask for a current square
                file_masks[square] |= set_file_rank_mask(file, -1);
            }
        }

        /******** Init rank masks ********/

        // loop over ranks
        for (int rank = 0; rank < 8; rank++)
        {
            // loop over files
            for (int file = 0; file < 8; file++)
            {
                // init square
                int square = rank * 8 + file;

                // init rank mask for a current square
                rank_masks[square] |= set_file_rank_mask(-1, rank);
            }
        }

        /******** Init isolated masks ********/

        // loop over ranks
        for (int rank = 0; rank < 8; rank++)
        {
            // loop over files
            for (int file = 0; file < 8; file++)
            {
                // init square
                int square = rank * 8 + file;

                // init isolated pawns masks for a current square
                isolated_masks[square] |= set_file_rank_mask(file - 1, -1);
                isolated_masks[square] |= set_file_rank_mask(file + 1, -1);
            }
        }

        /******** White passed masks ********/

        // loop over ranks
        for (int rank = 0; rank < 8; rank++)
        {
            // loop over files
            for (int file = 0; file < 8; file++)
            {
                // init square
                int square = rank * 8 + file;

                // init white passed pawns mask for a current square
                white_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
                white_passed_masks[square] |= set_file_rank_mask(file, -1);
                white_passed_masks[square] |= set_file_rank_mask(file + 1, -1);

                // loop over redudant ranks
                for (int i = 0; i < (8 - rank); i++)
                    // reset redudant bits 
                    white_passed_masks[square] &= ~rank_masks[(7 - i) * 8 + file];
            }
        }

        /******** Black passed masks ********/

        // loop over ranks
        for (int rank = 0; rank < 8; rank++)
        {
            // loop over files
            for (int file = 0; file < 8; file++)
            {
                // init square
                int square = rank * 8 + file;

                // init black passed pawns mask for a current square
                black_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
                black_passed_masks[square] |= set_file_rank_mask(file, -1);
                black_passed_masks[square] |= set_file_rank_mask(file + 1, -1);

                // loop over redudant ranks
                for (int i = 0; i < rank + 1; i++)
                    // reset redudant bits 
                    black_passed_masks[square] &= ~rank_masks[i * 8 + file];
            }
        }
    }

    // get game phase score
    static inline int get_game_phase_score()
    {
        /*
            The game phase score of the game is derived from the pieces
            (not counting pawns and kings) that are still on the board.
            The full material starting position game phase score is:

            4 * knight material score in the opening +
            4 * bishop material score in the opening +
            4 * rook material score in the opening +
            2 * queen material score in the opening
        */

        // white & black game phase scores
        int white_piece_scores = 0, black_piece_scores = 0;

        // loop over white pieces
        for (int piece = N; piece <= Q; piece++)
            white_piece_scores += count_bits(bitboards[piece]) * material_score[opening][piece];

        // loop over white pieces
        for (int piece = n; piece <= q; piece++)
            black_piece_scores += count_bits(bitboards[piece]) * -material_score[opening][piece];

        // return game phase score
        return white_piece_scores + black_piece_scores;
    }

    // position evaluation
    int evaluate()
    {

        // get game phase score
        int game_phase_score = get_game_phase_score();
        printf("game phase score: %d\n", game_phase_score);

        // static evaluation score
        int score = 0;

        // current pieces bitboard copy
        U64 bitboard;

        // init piece & square
        int piece, square;

        // penalties
        int double_pawns = 0;

        // loop over piece bitboards
        for (int bb_piece = P; bb_piece <= k; bb_piece++)
        {
            // init piece bitboard copy
            bitboard = bitboards[bb_piece];

            // loop over pieces within a bitboard
            while (bitboard)
            {
                // init piece
                piece = bb_piece;

                // init square
                square = get_ls1b_index(bitboard);

                // score material weights
                score += material_score[piece];

                // score positional piece scores
                switch (piece)
                {
                    // evaluate white pieces
                case P: 
                    // positional score
                    score += pawn_score[square];

                    // double pawn penalty
                    double_pawns = count_bits(bitboards[P] & file_masks[square]);

                    // on double pawns (tripple, etc)
                    if (double_pawns > 1)
                        score += double_pawns * double_pawn_penalty;

                    // on isolated pawn
                    if ((bitboards[P] & isolated_masks[square]) == 0)
                        // give an isolated pawn penalty
                        score += isolated_pawn_penalty;

                    // on passed pawn
                    if ((white_passed_masks[square] & bitboards[p]) == 0)
                        // give passed pawn bonus
                        score += passed_pawn_bonus[get_rank[square]];

                    break;

                case N:
                    // positional score
                    score += knight_score[square];
                    break;

                case B: 
                    // positional scores
                    score += bishop_score[square];

                    // mobility
                    score += count_bits(Attacks::get_bishop_attacks(square, occupancies[both]));

                    break;

                case R: 
                    // positional score
                    score += rook_score[square];

                    // semi open file
                    if ((bitboards[P] & file_masks[square]) == 0)
                        // add semi open file bonus
                        score += semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file bonus
                        score += open_file_score;

                    break;

                    // evaluate white queens
                case Q:
                    // mobility
                    score += count_bits(Attacks::get_queen_attacks(square, occupancies[both]));
                    break;

                case K: 
                    // posirional score
                    score += king_score[square];

                    // semi open file
                    if ((bitboards[P] & file_masks[square]) == 0)
                        // add semi open file penalty
                        score -= semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file penalty
                        score -= open_file_score;

                    // king safety bonus
                    score += count_bits(Attacks::king_attacks[square] & occupancies[white]) * king_shield_bonus;
                    break;


                    // evaluate black pawns
                case p:
                    // positional score
                    score -= pawn_score[mirror_score[square]];

                    // double pawn penalty
                    double_pawns = count_bits(bitboards[p] & file_masks[square]);

                    // on double pawns (tripple, etc)
                    if (double_pawns > 1)
                        score -= double_pawns * double_pawn_penalty;

                    // on isolated pawnd
                    if ((bitboards[p] & isolated_masks[square]) == 0)
                        // give an isolated pawn penalty
                        score -= isolated_pawn_penalty;

                    // on passed pawn
                    if ((black_passed_masks[square] & bitboards[P]) == 0)
                        // give passed pawn bonus
                        score -= passed_pawn_bonus[get_rank[mirror_score[square]]];

                    break;

                    // evaluate black knights
                case n:
                    // positional score
                    score -= knight_score[mirror_score[square]];
                    break;

                    // evaluate black bishops
                case b:
                    // positional score
                    score -= bishop_score[mirror_score[square]];

                    // mobility
                    score -= count_bits(Attacks::get_bishop_attacks(square, occupancies[both]));
                    break;

                    // evaluate black rooks
                case r:
                    // positional score
                    score -= rook_score[mirror_score[square]];

                    // semi open file
                    if ((bitboards[p] & file_masks[square]) == 0)
                        // add semi open file bonus
                        score -= semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file bonus
                        score -= open_file_score;

                    break;

                    // evaluate black queens
                case q:
                    // mobility
                    score -= count_bits(Attacks::get_queen_attacks(square, occupancies[both]));
                    break;

                    // evaluate black king
                case k:
                    // positional score
                    score -= king_score[mirror_score[square]];

                    // semi open file
                    if ((bitboards[p] & file_masks[square]) == 0)
                        // add semi open file penalty
                        score += semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file penalty
                        score += open_file_score;

                    // king safety bonus
                    score -= count_bits(Attacks::king_attacks[square] & occupancies[black]) * king_shield_bonus;
                    break;
                }
                

                // pop ls1b
                pop_bit(bitboard, square);
            }
        }

        // return final evaluation based on side
        return (side == white) ? score : -score;
    }

}

namespace Search {

    /* These are the score bounds for the range of the mating scores
  [-infinity, -mate_value ... -mate_score, ... score ... mate_score ... mate_value, infinity]
   */

#define infinity 50000
#define mate_value 49000
#define mate_score 48000
   // hash table size
#define hash_size 0x400000

// no hash entry found constant
#define no_hash_entry 100000

// transposition table hash flags
#define hash_flag_exact 0
#define hash_flag_alpha 1
#define hash_flag_beta 2

// transposition table data structure
    typedef struct {
        U64 hash_key;   // "almost" unique chess position identifier
        int depth;      // current search depth
        int flag;       // flag the type of node (fail-low/fail-high/PV) 
        int score;      // score (alpha/beta/PV)
    } transpositionTable;               // transposition table (TT aka hash table)

// define TT instance
    transpositionTable hash_table[hash_size];

    // clear TT (hash table)
    void clear_hash_table()
    {
        // loop over TT elements
        for (int index = 0; index < hash_size; index++)
        {
            // reset TT inner fields
            hash_table[index].hash_key = 0;
            hash_table[index].depth = 0;
            hash_table[index].flag = 0;
            hash_table[index].score = 0;
        }
    }

    // read hash entry data
    static inline int read_hash_entry(int alpha, int beta, int depth)
    {
        // create a TT instance pointer to particular hash entry storing
        // the scoring data for the current board position if available
        transpositionTable* hash_entry = &hash_table[hash_key % hash_size];

        // make sure we're dealing with the exact position we need
        if (hash_entry->hash_key == hash_key)
        {
            // make sure that we match the exact depth our search is now at
            if (hash_entry->depth >= depth)
            {
                // extract stored score from TT entry
                int score = hash_entry->score;

                // retrieve score independent from the actual path
                // from root node (position) to current node (position)
                if (score < -mate_score) score += ply;
                if (score > mate_score) score -= ply;

                // match the exact (PV node) score 
                if (hash_entry->flag == hash_flag_exact)
                    // return exact (PV node) score
                    return score;



                // match alpha (fail-low node) score
                if ((hash_entry->flag == hash_flag_alpha) &&
                    (hash_entry->score <= alpha))
                    // return alpha (fail-low node) score
                    return alpha;

                // match beta (fail-high node) score
                if ((hash_entry->flag == hash_flag_beta) &&
                    (hash_entry->score >= beta))
                    // return beta (fail-high node) score
                    return beta;
            }
        }

        // if hash entry doesn't exist
        return no_hash_entry;
    }

    // write hash entry data
    static inline void write_hash_entry(int score, int depth, int hash_flag)
    {
        // create a TT instance pointer to particular hash entry storing
        // the scoring data for the current board position if available
        transpositionTable* hash_entry = &hash_table[hash_key % hash_size];

        // store score independent from the actual path
        // from root node (position) to current node (position)
        if (score < -mate_score) score -= ply;
        if (score > mate_score) score += ply;

        // write hash entry data 
        hash_entry->hash_key = hash_key;
        hash_entry->score = score;
        hash_entry->flag = hash_flag;
        hash_entry->depth = depth;
    }


    // most valuable victim & less valuable attacker

/*

    (Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
        Pawn   105    205    305    405    505    605
      Knight   104    204    304    404    504    604
      Bishop   103    203    303    403    503    603
        Rook   102    202    302    402    502    602
       Queen   101    201    301    401    501    601
        King   100    200    300    400    500    600
*/

// MVV LVA [attacker][victim]

    static int mvv_lva[12][12] = {
        105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
        104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
        103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
        102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
        101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
        100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

        105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
        104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
        103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
        102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
        101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
        100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
    };

    // max ply that we can reach within a search
    #define MAX_PLY 64

    // killer moves [id][ply]
    int killer_moves[2][MAX_PLY];

    // history moves [piece][square]
    int history_moves[12][64];

    /*
          ================================
                Triangular PV table
          --------------------------------
            PV line: e2e4 e7e5 g1f3 b8c6
          ================================
               0    1    2    3    4    5

          0    m1   m2   m3   m4   m5   m6

          1    0    m2   m3   m4   m5   m6

          2    0    0    m3   m4   m5   m6

          3    0    0    0    m4   m5   m6

          4    0    0    0    0    m5   m6

          5    0    0    0    0    0    m6
    */

    // PV length
    int pv_length[MAX_PLY];

    // PV table
    int pv_table[MAX_PLY][MAX_PLY];

    // follow PV & score PV move
    bool follow_pv, score_pv;


    // enable PV move scoring
    void enable_pv_scoring(std::vector<int>& move_list)
    {
        // disable following PV
        follow_pv = false;

        // loop over the moves within a move list
        for (int count = 0; count < move_list.size(); count++)
        {
            // make sure we hit PV move
            if (pv_table[0][ply] == move_list[count])
            {
                // enable move scoring
                score_pv = true;

                // enable following PV
                follow_pv = true;
            }
        }
    }

    // score moves
    int score_move(int move)
    {
        // if PV move scoring is allowed
        if (score_pv)
        {
            // make sure we are dealing with PV move
            if (pv_table[0][ply] == move)
            {
                // disable score PV flag
                score_pv = 0;

                // give PV move the highest score to search it first
                return 20000;
            }
        }

        // score capture move
        if (get_move_capture(move))
        {
            // init target piece
            int target_piece = P;

            // pick up bitboard piece index ranges depending on side
            int start_piece, end_piece;

            // pick up side to move
            if (side == white) { start_piece = p; end_piece = k; }
            else { start_piece = P; end_piece = K; }

            // loop over bitboards opposite to the current side to move
            for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
            {
                // if there's a piece on the target square
                if (get_bit(bitboards[bb_piece], get_move_target(move)))
                {
                    // remove it from corresponding bitboard
                    target_piece = bb_piece;
                    break;
                }
            }

            // score move by MVV LVA lookup [source piece][target piece]
            return mvv_lva[get_move_piece(move)][target_piece];
        }

        // score quiet move
        else
        {
            // score 1st killer move
            if (killer_moves[0][ply] == move)
                return 9000;

            // score 2nd killer move
            else if (killer_moves[1][ply] == move)
                return 8000;

            // score history move
            else
                return history_moves[get_move_piece(move)][get_move_target(move)];

        }

        return 0;
    }

    // sort moves in descending order
    void sort_moves(std::vector<int>& move_list)
    {
       
        // move scores
        std::vector<int> move_scores;

        // score all the moves within a move list
        for (int count = 0; count < move_list.size(); count++)
            // score move
            move_scores.push_back(score_move(move_list[count]));

        // loop over current move within a move list
        for (int current_move = 0; current_move < move_list.size(); current_move++)
        {
            // loop over next move within a move list
            for (int next_move = current_move + 1; next_move < move_list.size(); next_move++)
            {
                // compare current and next move scores
                if (move_scores[current_move] < move_scores[next_move])
                {
                    // swap scores
                    int temp_score = move_scores[current_move];
                    move_scores[current_move] = move_scores[next_move];
                    move_scores[next_move] = temp_score;

                    // swap moves
                    int temp_move = move_list[current_move];
                    move_list[current_move] = move_list[next_move];
                    move_list[next_move] = temp_move;
                }
            }
        }
    }

    // print move scores
    void print_move_scores(std::vector<int>& move_list)
    {
        printf("     Move scores:\n\n");

        // loop over moves within a move list
        for (int count = 0; count < move_list.size(); count++)
        {
            printf("     move: ");
            MoveGeneration::print_move(move_list[count]);
            printf(" score: %d\n", score_move(move_list[count]));
        }
    }

    // position repetition detection
    static int is_repetition()
    {
        // loop over repetition indicies range
        for (int index = 0; index < repetition_index; index++)
            // if we found the hash key same with a current
            if (repetition_table[index] == hash_key)
                // we found a repetition
                return 1;

        // if no repetition found
        return 0;
    }

    // quiescence search
    static int quiescence(int alpha, int beta)
    {
        // every 2047 nodes
        if ((Perft::nodes & 2047) == 0)
            // "listen" to the GUI/user input
            Time::communicate();

        // increment nodes count
        Perft::nodes++;

        // we are too deep, hence there's an overflow of arrays relying on max ply constant
        if (ply > MAX_PLY - 1)
            // evaluate position
            return Evaluation::evaluate();

        // evaluate position
        int evaluation = Evaluation::evaluate();

        // fail-hard beta cutoff
        if (evaluation >= beta)
        {
            // node (position) fails high
            return beta;
        }

        // found a better move
        if (evaluation > alpha)
        {
            // PV node (position)
            alpha = evaluation;
        }

        // create move list instance
        std::vector<int> move_list;

        // generate moves
        MoveGeneration::generate_moves(move_list);

        // sort moves
        Search::sort_moves(move_list);

        // loop over moves within a movelist
        for (int count = 0; count < move_list.size(); count++)
        {
            // preserve board state
            copy_board();

            // increment ply
            ply++;

            // increment repetition index & store hash key
            repetition_index++;
            repetition_table[repetition_index] = hash_key;

            // make sure to make only legal moves
            if (MoveGeneration::make_move(move_list[count], MoveGeneration::only_captures) == 0)
            {
                // decrement ply
                ply--;

                // decrement repetition index
                repetition_index--;

                // skip to next move
                continue;
            }

            // score current move
            int score = -quiescence(-beta, -alpha);

            // decrement ply
            ply--;

            // take move back
            take_back();

            // reutrn 0 if time is up
            if (Time::stopped == 1) return 0;

            // fail-hard beta cutoff
            if (score >= beta)
            {
                // node (position) fails high
                return beta;
            }

            // found a better move
            if (score > alpha)
            {
                // PV node (position)
                alpha = score;

            }
        }

        // node (position) fails low
        return alpha;
    }

   

    const int full_depth_moves = 4;
    const int reduction_limit = 3;

    // negamax alpha beta search
    static inline int negamax(int alpha, int beta, int depth)
    {
        // variable to store current move's score (from the static evaluation perspective)
        int score;

        // define hash flag
        int hash_flag = hash_flag_alpha;

        // if position repetition occurs
        if (ply && is_repetition())
            // return draw score
            return 0;

        // a hack by Pedro Castro to figure out whether the current node is PV node or not 
        int pv_node = beta - alpha > 1;

        // read hash entry
        if (ply && (score = read_hash_entry(alpha, beta, depth)) != no_hash_entry && !pv_node)
            // if the move has already been searched (hence has a value)
            // we just return the score for this move without searching it
            return score;

        // every 2047 nodes
        if ((Perft::nodes & 2047) == 0)
            // "listen" to the GUI/user input
            Time::communicate();

        // init PV length
        pv_length[ply] = ply;

        // recursion escapre condition
        if (depth == 0)
            return quiescence(alpha, beta);
            

        // we are too deep, hence there's an overflow of arrays relying on max ply constant
        if (ply > MAX_PLY - 1)
            // evaluate position
            return Evaluation::evaluate();

        // increment nodes count
        Perft::nodes++;

        // is king in check
        int in_check = MoveGeneration::is_square_attacked((side == white) ? get_ls1b_index(bitboards[K]) :
            get_ls1b_index(bitboards[k]),
            side ^ 1);

        // increase search depth if the king has been exposed into a check
        if (in_check)
            depth++;

        // legal moves counter
        int legal_moves = 0;

        // null move pruning
        if (depth >= 3 && in_check == 0 && ply)
        {
            // preserve board state
            copy_board();

            // increment ply
            ply++;

            // increment repetition index & store hash key
            repetition_index++;
            repetition_table[repetition_index] = hash_key;

            // switch the side, literally giving opponent an extra move to make
            side ^= 1;

            // reset enpassant capture square
            enpassant = no_sq;

            // search moves with reduced depth to find beta cutoffs
               //depth - 1 - R where R is a reduction limit 
            int score = -negamax(-beta, -beta + 1, depth - 1 - 2);

            // decrement ply
            ply--;

            // decrement repetition index
            repetition_index--;

            // restore board state
            take_back();

            if (Time::stopped)
                return 0;

            // fail-hard beta cutoff
            if (score >= beta)
                // node (move) fails high
                return beta;
        }

        // create move list instance
        std::vector<int> move_list;

        // generate moves
        MoveGeneration::generate_moves(move_list);

        // if we are now following PV line
        if (follow_pv)
            // enable PV move scoring
            enable_pv_scoring(move_list);

        // sort moves
        sort_moves(move_list);

        // number of moves searched in a move list
        int moves_searched = 0;

        // loop over moves within a movelist
        for (int count = 0; count < move_list.size(); count++)
        {
            // preserve board state
            copy_board();

            // increment ply
            ply++;

            // increment repetition index & store hash key
            repetition_index++;
            repetition_table[repetition_index] = hash_key;

            // make sure to make only legal moves
            if (MoveGeneration::make_move(move_list[count], MoveGeneration::all_moves) == 0)
            {
                ply--;

                // decrement repetition index
                repetition_index--;

                continue;
            }

            // increment legal moves
            legal_moves++;

            // variable to store current move's score (from the static evaluation perspective)
            //int score;

            // full depth search
            if (moves_searched == 0)
                // do normal alpha beta search
                score = -negamax(-beta, -alpha, depth - 1);

            // late move reduction (LMR)
            else
            {
                // condition to consider LMR
                if (
                    moves_searched >= full_depth_moves &&
                    depth >= reduction_limit &&
                    in_check == 0 &&
                    get_move_capture(move_list[count]) == 0 &&
                    get_move_promoted(move_list[count]) == 0
                    )
                    // search current move with reduced depth:
                    score = -negamax(-alpha - 1, -alpha, depth - 2);

                // hack to ensure that full-depth search is done
                else score = alpha + 1;

                // principle variation search PVS
                if (score > alpha)
                {
                    /* Once you've found a move with a score that is between alpha and beta,
                       the rest of the moves are searched with the goal of proving that they are all bad.
                       It's possible to do this a bit faster than a search that worries that one
                       of the remaining moves might be good. */
                    score = -negamax(-alpha - 1, -alpha, depth - 1);

                    /* If the algorithm finds out that it was wrong, and that one of the
                       subsequent moves was better than the first PV move, it has to search again,
                       in the normal alpha-beta manner.  This happens sometimes, and it's a waste of time,
                       but generally not often enough to counteract the savings gained from doing the
                       "bad move proof" search referred to earlier. */
                    if ((score > alpha) && (score < beta))
                        /* re-search the move that has failed to be proved to be bad
                           with normal alpha beta score bounds*/
                        score = -negamax(-beta, -alpha, depth - 1);
                }
            }

            // decrement ply
            ply--;

            // decrement repetition index
            repetition_index--;

            // take move back
            take_back();

            if (Time::stopped)
                return 0;
            // increment the counter of moves searched so far
            moves_searched++;

            // found a better move
            if (score > alpha)
            {
                // switch hash flag from storing score for fail-low node
                // to the one storing score for PV node
                hash_flag = hash_flag_exact;

                // on quiet moves
                if (get_move_capture(move_list[count]) == 0)
                    // store history moves
                    history_moves[get_move_piece(move_list[count])][get_move_target(move_list[count])] += depth;

                // PV node (position)
                alpha = score;

                // write PV move
                pv_table[ply][ply] = move_list[count];

                // loop over the next ply
                for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
                    // copy move from deeper ply into a current ply's line
                    pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];

                // adjust PV length
                pv_length[ply] = pv_length[ply + 1];

                // fail-hard beta cutoff
                if (score >= beta)
                {
                    // store hash entry with the score equal to beta
                    write_hash_entry(beta, depth, hash_flag_beta);

                    // on quiet moves
                    if (get_move_capture(move_list[count]) == 0)
                    {
                        // store killer moves
                        killer_moves[1][ply] = killer_moves[0][ply];
                        killer_moves[0][ply] = move_list[count];
                    }

                    // node (position) fails high
                    return beta;
                }
            }
        }

        // we don't have any legal moves to make in the current postion
        if (legal_moves == 0)
        {
            // king is in check
            if (in_check)
                // return mating score (assuming closest distance to mating position)
                return -mate_value + ply;

            // king is not in check
            else
                // return stalemate score
                return 0;
        }

        // store hash entry with the score equal to alpha
        write_hash_entry(alpha, depth, hash_flag);

        // node (move) fails low
        return alpha;
    }
    
    // search position for the best move
    void search_position(int depth)
    {
        // find best move within a given position
        int score = 0;

        // reset nodes counter
        Perft::nodes = 0;

        Time::stopped = 0;

        // reset follow PV flags
        follow_pv = false;
        score_pv = false;

        // clear helper data structures for search
        memset(killer_moves, 0, sizeof(killer_moves));
        memset(history_moves, 0, sizeof(history_moves));
        memset(pv_table, 0, sizeof(pv_table));
        memset(pv_length, 0, sizeof(pv_length));

        // define initial alpha beta bounds
        int alpha = -infinity;
        int beta = infinity;

        // iterative deepening
        for (int current_depth = 1; current_depth <= depth; current_depth++)
        {
            if (Time::stopped)
                break;

            follow_pv = true;

            // find best move within a given position
            score = negamax(alpha, beta, current_depth);

            // we fell outside the window, so try again with a full-width window (and the same depth)
            if ((score <= alpha) || (score >= beta)) {
                alpha = -infinity;
                beta = infinity;
                continue;
            }

            // set up the window for the next iteration
            alpha = score - 50;
            beta = score + 50;

            if (score > -mate_value && score < -mate_score)
                printf("info score mate %d depth %d nodes %ld time %d pv ", -(score + mate_value) / 2 - 1, current_depth, Perft::nodes, Time::get_time_ms() - Time::starttime);

            else if (score > mate_score && score < mate_value)
                printf("info score mate %d depth %d nodes %ld time %d pv ", (mate_value - score) / 2 + 1, current_depth, Perft::nodes, Time::get_time_ms() - Time::starttime);

            else
                printf("info score cp %d depth %d nodes %ld time %d pv ", score, current_depth, Perft::nodes, Time::get_time_ms() - Time::starttime);

            

            // loop over the moves within a PV line
            for (int count = 0; count < pv_length[0]; count++)
            {
                // print PV move
                MoveGeneration::print_move(pv_table[0][count]);
                printf(" ");
            }

            // print new line
            printf("\n");
        }

        // best move placeholder
        printf("bestmove ");
        MoveGeneration::print_move(pv_table[0][0]);
        printf("\n");


    }

}

namespace UCI
{
    // parse user/GUI move string input (e.g. "e7e8q")
    int parse_move(char* move_string)
    {
        // create move list instance
        std::vector<int> move_list;

        // generate moves
        MoveGeneration::generate_moves(move_list);

        // parse source square
        int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;

        // parse target square
        int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

        // loop over the moves within a move list
        for (int move_count = 0; move_count < move_list.size(); move_count++)
        {
            // init move
            int move = move_list[move_count];

            // make sure source & target squares are available within the generated move
            if (source_square == get_move_source(move) && target_square == get_move_target(move))
            {
                // init promoted piece
                int promoted_piece = get_move_promoted(move);

                // promoted piece is available
                if (promoted_piece)
                {
                    // promoted to queen
                    if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                        // return legal move
                        return move;

                    // promoted to rook
                    else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                        // return legal move
                        return move;

                    // promoted to bishop
                    else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                        // return legal move
                        return move;

                    // promoted to knight
                    else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                        // return legal move
                        return move;

                    // continue the loop on possible wrong promotions (e.g. "e7e8f")
                    continue;
                }

                // return legal move
                return move;
            }
        }

        // return illegal move
        return 0;
    }

    /*
        Example UCI commands to init position on chess board

        // init start position
        position startpos

        // init start position and make the moves on chess board
        position startpos moves e2e4 e7e5

        // init position from FEN string
        position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1

        // init position from fen string and make moves on chess board
        position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e8g8
    */

    // parse UCI "position" command
    void parse_position(char* command)
    {
        // shift pointer to the right where next token begins
        command += 9;

        // init pointer to the current character in the command string
        char* current_char = command;

        // parse UCI "startpos" command
        if (strncmp(command, "startpos", 8) == 0)
            // init chess board with start position
            parse_fen(start_position);

        // parse UCI "fen" command 
        else
        {
            // make sure "fen" command is available within command string
            current_char = strstr(command, "fen");

            // if no "fen" command is available within command string
            if (current_char == NULL)
                // init chess board with start position
                parse_fen(start_position);

            // found "fen" substring
            else
            {
                // shift pointer to the right where next token begins
                current_char += 4;

                // init chess board with position from FEN string
                parse_fen(current_char);
            }
        }

        // parse moves after position
        current_char = strstr(command, "moves");

        // moves available
        if (current_char != NULL)
        {
            // shift pointer to the right where next token begins
            current_char += 6;

            // loop over moves within a move string
            while (*current_char)
            {
                // parse next move
                int move = parse_move(current_char);

                // if no more moves
                if (move == 0)
                    // break out of the loop
                    break;

                // make move on the chess board
                MoveGeneration::make_move(move, MoveGeneration::all_moves);

                // move current character mointer to the end of current move
                while (*current_char && *current_char != ' ') current_char++;

                // go to the next move
                current_char++;
            }
        }

        // print board
        print_board();
    }

    // parse UCI command "go"
    void parse_go(char* command)
    {
        // init parameters
        int depth = -1;

        // init argument
        char* argument = NULL;

        // infinite search
        if ((argument = strstr(command, "infinite"))) {}

        // match UCI "binc" command
        if ((argument = strstr(command, "binc")) && side == black)
            // parse black time increment
            Time::inc = atoi(argument + 5);

        // match UCI "winc" command
        if ((argument = strstr(command, "winc")) && side == white)
            // parse white time increment
            Time::inc = atoi(argument + 5);

        // match UCI "wtime" command
        if ((argument = strstr(command, "wtime")) && side == white)
            // parse white time limit
            Time::time = atoi(argument + 6);

        // match UCI "btime" command
        if ((argument = strstr(command, "btime")) && side == black)
            // parse black time limit
            Time::time = atoi(argument + 6);

        // match UCI "movestogo" command
        if ((argument = strstr(command, "movestogo")))
            // parse number of moves to go
            Time::movestogo = atoi(argument + 10);

        // match UCI "movetime" command
        if ((argument = strstr(command, "movetime")))
            // parse amount of time allowed to spend to make a move
            Time::movetime = atoi(argument + 9);

        // match UCI "depth" command
        if ((argument = strstr(command, "depth")))
            // parse search depth
            depth = atoi(argument + 6);

        // if move time is not available
        if (Time::movetime != -1)
        {
            // set time equal to move time
            Time::time = Time:: movetime;

            // set moves to go to 1
            Time::movestogo = 1;
        }

        // init start time
        Time::starttime = Time::get_time_ms();

        // init search depth
        depth = depth;

        // if time control is available
        if (Time::time != -1)
        {
            // flag we're playing with time control
            Time::timeset = 1;

            // set up timing
            Time::time /= Time::movestogo;

            // "illegal" (empty) move bug fix
            if (Time::time > 1500) 
                Time::time -= 50;
            Time::stoptime = Time::starttime + Time::time +Time:: inc;
        }

        // if depth is not available
        if (depth == -1)
            // set depth to 64 plies (takes ages to complete...)
            depth = 64;

        // print debug info
        printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
            Time::time, Time::starttime, Time::stoptime, depth, Time::timeset);

        // search position
        Search::search_position(depth);
    }

    // main UCI loop
    void uci_loop()
    {
        // reset STDIN & STDOUT buffers
        //setbuf(stdin, NULL);
        //setbuf(stdout, NULL);

        // define user / GUI input buffer
        char input[2000];

        // print engine info
        printf("id name BBC\n");
        printf("id name Lencho\n");
        printf("uciok\n");

        // main loop
        while (1)
        {
            // reset user /GUI input
            memset(input, 0, sizeof(input));

            // make sure output reaches the GUI
            fflush(stdout);

            // get user / GUI input
            if (!fgets(input, 2000, stdin))
                // continue the loop
                continue;

            // make sure input is available
            if (input[0] == '\n')
                // continue the loop
                continue;

            // parse UCI "isready" command
            if (strncmp(input, "isready", 7) == 0)
            {
                printf("readyok\n");
                continue;
            }

            // parse UCI "position" command
            else if (strncmp(input, "position", 8) == 0)
                // call parse position function
                parse_position(input);

            // parse UCI "ucinewgame" command
            else if (strncmp(input, "ucinewgame", 10) == 0)
                // call parse position function
                parse_position((char*)start_position);

            // parse UCI "go" command
            else if (strncmp(input, "go", 2) == 0)
                // call parse go function
                parse_go(input);

            // parse UCI "quit" command
            else if (strncmp(input, "quit", 4) == 0)
                // quit from the chess engine program execution
                break;

            // parse UCI "uci" command
            else if (strncmp(input, "uci", 3) == 0)
            {
                // print engine info
                printf("id name BBC\n");
                printf("id name Lencho\n");
                printf("uciok\n");
            }
        }
    }
}


void init_all()
{
    init_leapers_attacks();
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
    Zobrist::init_random_keys();
    Evaluation::init_evaluation_masks();
    Search::clear_hash_table();
}



int main()
{

    init_all;

    parse_fen(tricky_position);

    print_board();
    Search::search_position(4);
    // connect to the GUI
    UCI::uci_loop();

    return 0;
}