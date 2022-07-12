#pragma once

// system Headerfiles
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <windows.h>
#include <fstream>

// Fen for starting position
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "

// define bitboard data type
#define U64 unsigned long long

// board squares
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

static char ascii_pieces[] = "PNBRQKpnbrqk";

// sides to move (colors)
enum { white, black, both };

// bishop and rook
enum { rook, bishop };

// encode pieces
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// Castling rights binary encoding

/*
    bin  dec

   0001    1  white king can castle to the king side
   0010    2  white king can castle to the queen side
   0100    4  black king can castle to the king side
   1000    8  black king can castle to the queen side
   examples
   1111       both sides an castle both directions
   1001       black king => queen side
              white king => king side
*/
enum { wk = 1, wq = 2, bk = 4, bq = 8 };

// convert squares to coordinates
const std::string square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};


// "almost" unique position identifier aka hash key or position key
U64 hash_key;

// positions repetition table
U64 repetition_table[1000];  // 1000 is a number of plies (500 moves) in the entire game

// repetition index
int repetition_index;

// half move counter
int ply;

#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : 0)


// piece bitboards
U64 bitboards[12];

// occupancy bitboards
U64 occupancies[3];

// side to move
int side;

// enpassant square
int enpassant = no_sq;

// castling rights
int castle;

// count bits within a bitboard (Brian Kernighan's way)
int count_bits(U64 bitboard)
{
    // bit counter
    int count = 0;

    // consecutively reset least significant 1st bit
    while (bitboard)
    {
        // increment count
        count++;

        // reset least significant 1st bit
        bitboard &= bitboard - 1;
    }

    // return bit count
    return count;
}

// get least significant 1st bit index
int get_ls1b_index(U64 bitboard)
{
    // make sure bitboard is not 0
    if (bitboard)
    {
        // count trailing bits before LS1B
        return count_bits((bitboard & ~(bitboard - 1)) - 1);
    }

    //otherwise
    else
        // return illegal index
        return -1;
}
