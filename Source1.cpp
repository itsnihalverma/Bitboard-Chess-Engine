
// Zobrist Hashing
namespace Zobrist
{
    // pseudo random number state
    unsigned int random_state = 1804289383;

    // generate 32-bit pseudo legal numbers
    unsigned int get_random_U32_number()
    {
        // get current state
        unsigned int number = random_state;

        // XOR shift algorithm
        number ^= number << 13;
        number ^= number >> 17;
        number ^= number << 5;

        // update random number state
        random_state = number;

        // return random number
        return number;
    }

    // generate 64-bit pseudo legal numbers
    U64 get_random_U64_number()
    {
        // define 4 random numbers
        U64 n1, n2, n3, n4;

        // init random numbers slicing 16 bits from MS1B side
        n1 = (U64)(get_random_U32_number()) & 0xFFFF;
        n2 = (U64)(get_random_U32_number()) & 0xFFFF;
        n3 = (U64)(get_random_U32_number()) & 0xFFFF;
        n4 = (U64)(get_random_U32_number()) & 0xFFFF;

        // return random number
        return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
    }


    // random piece keys [piece][square]
    U64 piece_keys[12][64];

    // random enpassant keys [square]
    U64 enpassant_keys[64];

    // random castling keys
    U64 castle_keys[16];

    // random side key
    U64 side_key;

    // init random hash keys
    void init_random_keys()
    {
        // update pseudo random number state
        random_state = 1804289383;

        // loop over piece codes
        for (int piece = P; piece <= k; piece++)
        {
            // loop over board squares
            for (int square = 0; square < 64; square++)
                // init random piece keys
                piece_keys[piece][square] = get_random_U64_number();
        }

        // loop over board squares
        for (int square = 0; square < 64; square++)
            // init random enpassant keys
            enpassant_keys[square] = get_random_U64_number();

        // loop over castling keys
        for (int index = 0; index < 16; index++)
            // init castling keys
            castle_keys[index] = get_random_U64_number();

        // init random side key
        side_key = get_random_U64_number();
    }

    // generate "almost" unique position ID aka hash key from scratch
    U64 generate_hash_key()
    {
        // final hash key
        U64 final_key = 0ULL;

        // temp piece bitboard copy
        U64 bitboard;

        // loop over piece bitboards
        for (int piece = P; piece <= k; piece++)
        {
            // init piece bitboard copy
            bitboard = bitboards[piece];

            // loop over the pieces within a bitboard
            while (bitboard)
            {
                // init square occupied by the piece
                int square = get_ls1b_index(bitboard);

                // hash piece
                final_key ^= piece_keys[piece][square];

                // pop LS1B
                pop_bit(bitboard, square);
            }
        }

        // if enpassant square is on board
        if (enpassant != no_sq)
            // hash enpassant
            final_key ^= enpassant_keys[enpassant];

        // hash castling rights
        final_key ^= castle_keys[castle];

        // hash the side only if black is to move
        if (side == black) final_key ^= side_key;

        // return generated hash key
        return final_key;
    }

}
/*
        not A file
  8  0 1 1 1 1 1 1 1
  7  0 1 1 1 1 1 1 1
  6  0 1 1 1 1 1 1 1
  5  0 1 1 1 1 1 1 1
  4  0 1 1 1 1 1 1 1
  3  0 1 1 1 1 1 1 1
  2  0 1 1 1 1 1 1 1
  1  0 1 1 1 1 1 1 1
     a b c d e f g h

       not H file

  8  1 1 1 1 1 1 1 0
  7  1 1 1 1 1 1 1 0
  6  1 1 1 1 1 1 1 0
  5  1 1 1 1 1 1 1 0
  4  1 1 1 1 1 1 1 0
  3  1 1 1 1 1 1 1 0
  2  1 1 1 1 1 1 1 0
  1  1 1 1 1 1 1 1 0
     a b c d e f g h

       not HG file

  8  1 1 1 1 1 1 0 0
  7  1 1 1 1 1 1 0 0
  6  1 1 1 1 1 1 0 0
  5  1 1 1 1 1 1 0 0
  4  1 1 1 1 1 1 0 0
  3  1 1 1 1 1 1 0 0
  2  1 1 1 1 1 1 0 0
  1  1 1 1 1 1 1 0 0
     a b c d e f g h

       not AB file

  8  0 0 1 1 1 1 1 1
  7  0 0 1 1 1 1 1 1
  6  0 0 1 1 1 1 1 1
  5  0 0 1 1 1 1 1 1
  4  0 0 1 1 1 1 1 1
  3  0 0 1 1 1 1 1 1
  2  0 0 1 1 1 1 1 1
  1  0 0 1 1 1 1 1 1
     a b c d e f g h
*/

// not A file constant
const U64 not_a_file = 18374403900871474942ULL;

// not H file constant
const U64 not_h_file = 9187201950435737471ULL;

// not HG file constant
const U64 not_hg_file = 4557430888798830399ULL;

// not AB file constant
const U64 not_ab_file = 18229723555195321596ULL;

// bishop relevant occupancy bit count for every square on board
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

// rook relevant occupancy bit count for every square on board
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

// rook magic numbers
U64 rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};

// bishop magic numbers
U64 bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};

namespace Attacks
{
    // Creating Attack tables for Different pieces
    U64 pawn_attacks[2][64];
    U64 knight_attacks[64];
    U64 king_attacks[64];
    U64 bishop_attacks[64][512];
    U64 rook_attacks[64][4096];
    U64 queen_attacks[64];

    U64 bishop_masks[64];
    U64 rook_masks[64];

    // set occupancies
    U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask)
    {
        // occupancy map
        U64 occupancy = 0ULL;

        // loop over the range of bits within attack mask
        for (int count = 0; count < bits_in_mask; count++)
        {
            // get LS1B index of attacks mask
            int square = get_ls1b_index(attack_mask);

            // pop LS1B in attack map
            pop_bit(attack_mask, square);

            // make sure occupancy is on board
            if (index & (1 << count))
                // populate occupancy map
                occupancy |= (1ULL << square);
        }

        // return occupancy map
        return occupancy;
    }

    // generate pawn attacks
    U64 mask_pawn_attacks(int side, int square)
    {
        // result attacks bitboard
        U64 attacks = 0ULL;

        // piece bitboard
        U64 bitboard = 0ULL;

        // set piece on board
        set_bit(bitboard, square);

        // white pawns
        if (!side)
        {
            // generate pawn attacks
            if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
            if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
        }

        // black pawns
        else
        {
            // generate pawn attacks
            if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
            if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
        }

        // return attack map
        return attacks;
    }

    // generate knight attacks
    U64 mask_knight_attacks(int square)
    {
        // result attacks bitboard
        U64 attacks = 0ULL;

        // piece bitboard
        U64 bitboard = 0ULL;

        // set piece on board
        set_bit(bitboard, square);

        // generate knight attacks
        if ((bitboard >> 17) & not_h_file) attacks |= (bitboard >> 17);
        if ((bitboard >> 15) & not_a_file) attacks |= (bitboard >> 15);
        if ((bitboard >> 10) & not_hg_file) attacks |= (bitboard >> 10);
        if ((bitboard >> 6) & not_ab_file) attacks |= (bitboard >> 6);
        if ((bitboard << 17) & not_a_file) attacks |= (bitboard << 17);
        if ((bitboard << 15) & not_h_file) attacks |= (bitboard << 15);
        if ((bitboard << 10) & not_ab_file) attacks |= (bitboard << 10);
        if ((bitboard << 6) & not_hg_file) attacks |= (bitboard << 6);

        // return attack map
        return attacks;
    }

    // generate king attacks
    U64 mask_king_attacks(int square)
    {
        // result attacks bitboard
        U64 attacks = 0ULL;

        // piece bitboard
        U64 bitboard = 0ULL;

        // set piece on board
        set_bit(bitboard, square);

        // generate king attacks
        if (bitboard >> 8) attacks |= (bitboard >> 8);
        if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
        if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
        if ((bitboard >> 1) & not_h_file) attacks |= (bitboard >> 1);
        if (bitboard << 8) attacks |= (bitboard << 8);
        if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
        if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
        if ((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);

        // return attack map
        return attacks;
    }


    // mask bishop attacks
    U64 mask_bishop_attacks(int square)
    {
        // result attacks bitboard
        U64 attacks = 0ULL;

        // init ranks & files
        int r, f;

        // init target rank & files
        int tr = square / 8;
        int tf = square % 8;

        // mask relevant bishop occupancy bits
        for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
        for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
        for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
        for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));

        // return attack map
        return attacks;
    }

    // mask rook attacks
    U64 mask_rook_attacks(int square)
    {
        // result attacks bitboard
        U64 attacks = 0ULL;

        // init ranks & files
        int r, f;

        // init target rank & files
        int tr = square / 8;
        int tf = square % 8;

        // mask relevant rook occupancy bits
        for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
        for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
        for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
        for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));

        // return attack map
        return attacks;
    }

    // generate bishop attacks on the fly
    U64 bishop_attacks_on_the_fly(int square, U64 block)
    {
        // result attacks bitboard
        U64 attacks = 0ULL;

        // init ranks & files
        int r, f;

        // init target rank & files
        int tr = square / 8;
        int tf = square % 8;

        // generate bishop atacks
        for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
        {
            attacks |= (1ULL << (r * 8 + f));
            if ((1ULL << (r * 8 + f)) & block) break;
        }

        for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
        {
            attacks |= (1ULL << (r * 8 + f));
            if ((1ULL << (r * 8 + f)) & block) break;
        }

        for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
        {
            attacks |= (1ULL << (r * 8 + f));
            if ((1ULL << (r * 8 + f)) & block) break;
        }

        for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
        {
            attacks |= (1ULL << (r * 8 + f));
            if ((1ULL << (r * 8 + f)) & block) break;
        }

        // return attack map
        return attacks;
    }

    // generate rook attacks on the fly
    U64 rook_attacks_on_the_fly(int square, U64 block)
    {
        // result attacks bitboard
        U64 attacks = 0ULL;

        // init ranks & files
        int r, f;

        // init target rank & files
        int tr = square / 8;
        int tf = square % 8;

        // generate rook attacks
        for (r = tr + 1; r <= 7; r++)
        {
            attacks |= (1ULL << (r * 8 + tf));
            if ((1ULL << (r * 8 + tf)) & block) break;
        }

        for (r = tr - 1; r >= 0; r--)
        {
            attacks |= (1ULL << (r * 8 + tf));
            if ((1ULL << (r * 8 + tf)) & block) break;
        }

        for (f = tf + 1; f <= 7; f++)
        {
            attacks |= (1ULL << (tr * 8 + f));
            if ((1ULL << (tr * 8 + f)) & block) break;
        }

        for (f = tf - 1; f >= 0; f--)
        {
            attacks |= (1ULL << (tr * 8 + f));
            if ((1ULL << (tr * 8 + f)) & block) break;
        }

        // return attack map
        return attacks;
    }

    // get bishop attacks
    U64 get_bishop_attacks(int square, U64 occupancy)
    {
        // get bishop attacks assuming current board occupancy
        occupancy &= bishop_masks[square];
        occupancy *= bishop_magic_numbers[square];
        occupancy >>= 64 - bishop_relevant_bits[square];

        // return bishop attacks
        return bishop_attacks[square][occupancy];
    }

    // get rook attacks
    U64 get_rook_attacks(int square, U64 occupancy)
    {
        // get rook attacks assuming current board occupancy
        occupancy &= rook_masks[square];
        occupancy *= rook_magic_numbers[square];
        occupancy >>= 64 - rook_relevant_bits[square];

        // return rook attacks
        return rook_attacks[square][occupancy];
    }

    // get queen attacks
    U64 get_queen_attacks(int square, U64 occupancy)
    {
        // init result attacks bitboard
        U64 queen_attacks = 0ULL;

        // init bishop occupancies
        U64 bishop_occupancy = occupancy;

        // init rook occupancies
        U64 rook_occupancy = occupancy;

        // get bishop attacks assuming current board occupancy
        bishop_occupancy &= bishop_masks[square];
        bishop_occupancy *= bishop_magic_numbers[square];
        bishop_occupancy >>= 64 - bishop_relevant_bits[square];

        // get bishop attacks
        queen_attacks = bishop_attacks[square][bishop_occupancy];

        // get bishop attacks assuming current board occupancy
        rook_occupancy &= rook_masks[square];
        rook_occupancy *= rook_magic_numbers[square];
        rook_occupancy >>= 64 - rook_relevant_bits[square];

        // get rook attacks
        queen_attacks |= rook_attacks[square][rook_occupancy];

        // return queen attacks
        return queen_attacks;
    }


}

// init leaper pieces attacks
void init_leapers_attacks()
{
    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
    {
        // init pawn attacks
        Attacks::pawn_attacks[white][square] = Attacks::mask_pawn_attacks(white, square);
        Attacks::pawn_attacks[black][square] = Attacks::mask_pawn_attacks(black, square);

        // init knight attacks
        Attacks::knight_attacks[square] = Attacks::mask_knight_attacks(square);

        //init king attacks
        Attacks::king_attacks[square] = Attacks::mask_king_attacks(square);
    }
}

// init slider piece's attack tables
void init_sliders_attacks(int bishop)
{
    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
    {
        // init bishop & rook masks
        Attacks::bishop_masks[square] = Attacks::mask_bishop_attacks(square);
        Attacks::rook_masks[square] = Attacks::mask_rook_attacks(square);

        // init current mask
        U64 attack_mask = bishop ? Attacks::bishop_masks[square] : Attacks::rook_masks[square];

        // init relevant occupancy bit count
        int relevant_bits_count = count_bits(attack_mask);

        // init occupancy indicies
        int occupancy_indicies = (1 << relevant_bits_count);

        // loop over occupancy indicies
        for (int index = 0; index < occupancy_indicies; index++)
        {
            // bishop
            if (bishop)
            {
                // init current occupancy variation
                U64 occupancy = Attacks::set_occupancy(index, relevant_bits_count, attack_mask);

                // init magic index
                int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);

                // init bishop attacks
                Attacks::bishop_attacks[square][magic_index] = Attacks::bishop_attacks_on_the_fly(square, occupancy);
            }

            // rook
            else
            {
                // init current occupancy variation
                U64 occupancy = Attacks::set_occupancy(index, relevant_bits_count, attack_mask);

                // init magic index
                int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);

                // init bishop attacks
                Attacks::rook_attacks[square][magic_index] = Attacks::rook_attacks_on_the_fly(square, occupancy);

            }
        }
    }
}


// is square current given attacked by the current given side
namespace MoveGeneration
{

    char promoted_pieces[] = { ' ','n','b','r','q',' ',' ','n','b','r','q',' ' };

    /*
          binary move bits                               hexidecimal constants

    0000 0000 0000 0000 0011 1111    source square       0x3f
    0000 0000 0000 1111 1100 0000    target square       0xfc0
    0000 0000 1111 0000 0000 0000    piece               0xf000
    0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
    0001 0000 0000 0000 0000 0000    capture flag        0x100000
    0010 0000 0000 0000 0000 0000    double push flag    0x200000
    0100 0000 0000 0000 0000 0000    enpassant flag      0x400000
    1000 0000 0000 0000 0000 0000    castling flag       0x800000
*/

// encode move
#define encode_move(source, target, piece, promoted, capture, Double, enpassant, castling) \
    (source) |          \
    (target << 6) |     \
    (piece << 12) |     \
    (promoted << 16) |  \
    (capture << 20) |   \
    (Double << 21) |    \
    (enpassant << 22) | \
    (castling << 23)    \

// extract move items
#define get_move_source(move) (move & 0x3f)
#define get_move_target(move) ((move & 0xfc0) >> 6)
#define get_move_piece(move) ((move & 0xf000) >> 12)
#define get_move_promoted(move) ((move & 0xf0000) >> 16)
#define get_move_capture(move) (move & 0x100000)
#define get_move_double(move) (move & 0x200000)
#define get_move_enpassant(move) (move & 0x400000)
#define get_move_castling(move) (move & 0x800000)


// preserve board state
#define copy_board()                                                      \
    U64 bitboards_copy[12], occupancies_copy[3];                          \
    int side_copy, enpassant_copy, castle_copy;                           \
    memcpy(bitboards_copy, bitboards, 96);                                \
    memcpy(occupancies_copy, occupancies, 24);                            \
    side_copy = side, enpassant_copy = enpassant, castle_copy = castle;   \
    U64 hash_key_copy = hash_key;

// restore board state
#define take_back()                                                       \
    memcpy(bitboards, bitboards_copy, 96);                                \
    memcpy(occupancies, occupancies_copy, 24);                            \
    side = side_copy, enpassant = enpassant_copy, castle = castle_copy;   \
    hash_key = hash_key_copy;

// move types
    enum { all_moves, only_captures };

    /*
                               castling   move     in      in
                                  right update     binary  decimal
     king & rooks didn't move:     1111 & 1111  =  1111    15
            white king  moved:     1111 & 1100  =  1100    12
      white king's rook moved:     1111 & 1110  =  1110    14
     white queen's rook moved:     1111 & 1101  =  1101    13

             black king moved:     1111 & 0011  =  1011    3
      black king's rook moved:     1111 & 1011  =  1011    11
     black queen's rook moved:     1111 & 0111  =  0111    7
    */

    // castling rights update constants
    const int castling_rights[64] = {
         7, 15, 15, 15,  3, 15, 15, 11,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        13, 15, 15, 15, 12, 15, 15, 14
    };

   
    // add move to the move list
    void add_move(std::vector<int>& move_list, int move)
    {
        move_list.push_back(move);

    }

    // print move (for UCI purposes)
    void print_move(int move)
    {
        std::cout << square_to_coordinates[get_move_source(move)] << \
            square_to_coordinates[get_move_target(move)] << \
            promoted_pieces[get_move_promoted(move)] << "\n";
    }


    // print move list
    void print_move_list(std::vector<int>& move_list)
    {
        std::cout << "move   piece   capture   double   enpass   castling\n\n";

        // loop over moves within a move list
        for (int move_count = 0; move_count < move_list.size(); move_count++)
        {
            // init move
            int move = move_list[move_count];


            // print move
            std::cout << square_to_coordinates[get_move_source(move)] << \
                square_to_coordinates[get_move_target(move)] << \
                promoted_pieces[get_move_promoted(move)] << "   " << \
                ascii_pieces[get_move_piece(move)] << "        " << \
                (get_move_capture(move) ? 1 : 0) << "        " << \
                (get_move_double(move) ? 1 : 0) << "        " << \
                (get_move_enpassant(move) ? 1 : 0) << "        " << \
                (get_move_castling(move) ? 1 : 0) << "\n";



        }
        // print total number of moves
        std::cout << "\n\n    Total number of moves: " << move_list.size() << "\n\n";
    }

    int is_square_attacked(int square, int side)
    {
        // attacked by white pawns
        if ((side == white) && (Attacks::pawn_attacks[black][square] & bitboards[P])) return 1;

        // attacked by black pawns
        if ((side == black) && (Attacks::pawn_attacks[white][square] & bitboards[p])) return 1;

        // attacked by knights
        if (Attacks::knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;

        // attacked by bishops
        if (Attacks::get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;

        // attacked by rooks
        if (Attacks::get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return 1;

        // attacked by bishops
        if (Attacks::get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return 1;

        // attacked by kings
        if (Attacks::king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;

        // by default return false
        return 0;
    }

    // print attacked squares
    void print_attacked_squares(int side)
    {
        std::cout << "\n";

        // loop over board ranks
        for (int rank = 0; rank < 8; rank++)
        {
            // loop over board files
            for (int file = 0; file < 8; file++)
            {
                // init square
                int square = rank * 8 + file;

                // print ranks
                if (!file)
                    std::cout << 8 - rank;

                // check whether current square is attacked or not
                std::cout << (is_square_attacked(square, side) ? 1 : 0);
            }

            // print new line every rank
            std::cout << "\n";
        }

        // print files
        std::cout << "\n     a b c d e f g h\n\n";
    }

    // generate all moves
    void generate_moves(std::vector<int>& move_list)
    {
        // define source & target squares
        int source_square, target_square;

        // define current piece's bitboard copy & it's attacks
        U64 bitboard, attacks;

        // loop over all the bitboards
        for (int piece = P; piece <= k; piece++)
        {
            // init piece bitboard copy
            bitboard = bitboards[piece];

            // generate white pawns & white king castling moves
            if (side == white)
            {
                // pick up white pawn bitboards index
                if (piece == P)
                {
                    // loop over white pawns within white pawn bitboard
                    while (bitboard)
                    {
                        // init source square
                        source_square = get_ls1b_index(bitboard);

                        // init target square
                        target_square = source_square - 8;

                        // generate quite pawn moves
                        if (!(target_square < a8) && !get_bit(occupancies[both], target_square))
                        {
                            // pawn promotion
                            if (source_square >= a7 && source_square <= h7)
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
                            }

                            else
                            {
                                // one square ahead pawn move
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                                // two squares ahead pawn move
                                if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
                                    add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
                            }
                        }

                        // init pawn attacks bitboard
                        attacks = Attacks::pawn_attacks[side][source_square] & occupancies[black];

                        // generate pawn captures
                        while (attacks)
                        {
                            // init target square
                            target_square = get_ls1b_index(attacks);

                            // pawn promotion
                            if (source_square >= a7 && source_square <= h7)
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
                            }

                            else
                                // one square ahead pawn move
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                            // pop ls1b of the pawn attacks
                            pop_bit(attacks, target_square);
                        }

                        // generate enpassant captures
                        if (enpassant != no_sq)
                        {
                            // lookup pawn attacks and bitwise AND with enpassant square (bit)
                            U64 enpassant_attacks = Attacks::pawn_attacks[side][source_square] & (1ULL << enpassant);

                            // make sure enpassant capture available
                            if (enpassant_attacks)
                            {
                                // init enpassant capture target square
                                int target_enpassant = get_ls1b_index(enpassant_attacks);
                                add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                            }
                        }

                        // pop ls1b from piece bitboard copy
                        pop_bit(bitboard, source_square);
                    }
                }

                if (piece == K)
                {
                    // king side castling is available
                    if (castle & wk)
                    {
                        // make sure square between king and king's rook are empty
                        if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
                        {
                            // make sure king and the f1 squares are not under attacks
                            if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                                add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                        }
                    }

                    // queen side castling is available
                    if (castle & wq)
                    {
                        // make sure square between king and queen's rook are empty
                        if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
                        {
                            // make sure king and the d1 squares are not under attacks
                            if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
                                add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }

            // generate black pawns & black king castling moves
            else
            {
                // pick up black pawn bitboards index
                if (piece == p)
                {
                    // loop over white pawns within white pawn bitboard
                    while (bitboard)
                    {
                        // init source square
                        source_square = get_ls1b_index(bitboard);

                        // init target square
                        target_square = source_square + 8;

                        // generate quite pawn moves
                        if (!(target_square > h1) && !get_bit(occupancies[both], target_square))
                        {
                            // pawn promotion
                            if (source_square >= a2 && source_square <= h2)
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
                            }

                            else
                            {
                                // one square ahead pawn move
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                // two squares ahead pawn move
                                if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
                                    add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
                            }
                        }

                        // init pawn attacks bitboard
                        attacks = Attacks::pawn_attacks[side][source_square] & occupancies[white];

                        // generate pawn captures
                        while (attacks)
                        {
                            // init target square
                            target_square = get_ls1b_index(attacks);

                            // pawn promotion
                            if (source_square >= a2 && source_square <= h2)
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
                            }

                            else
                                // one square ahead pawn move
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                            // pop ls1b of the pawn attacks
                            pop_bit(attacks, target_square);
                        }

                        // generate enpassant captures
                        if (enpassant != no_sq)
                        {
                            // lookup pawn attacks and bitwise AND with enpassant square (bit)
                            U64 enpassant_attacks = Attacks::pawn_attacks[side][source_square] & (1ULL << enpassant);

                            // make sure enpassant capture available
                            if (enpassant_attacks)
                            {
                                // init enpassant capture target square
                                int target_enpassant = get_ls1b_index(enpassant_attacks);
                                add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                            }
                        }

                        // pop ls1b from piece bitboard copy
                        pop_bit(bitboard, source_square);
                    }
                }

                if (piece == k)
                {
                    // king side castling is available
                    if (castle & bk)
                    {
                        // make sure square between king and king's rook are empty
                        if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
                        {
                            // make sure king and the f8 squares are not under attacks
                            if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                                add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
                        }
                    }

                    // queen side castling is available
                    if (castle & bq)
                    {
                        // make sure square between king and queen's rook are empty
                        if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
                        {
                            // make sure king and the d8 squares are not under attacks
                            if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                                add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }

            // genarate knight moves
            if ((side == white) ? piece == N : piece == n)
            {
                // loop over source squares of piece bitboard copy
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);

                    // init piece attacks in order to get set of target squares
                    attacks = Attacks::knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                    // loop over target squares available from generated attacks
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // quite move
                        if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                        else
                            // capture move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b in current attacks set
                        pop_bit(attacks, target_square);
                    }


                    // pop ls1b of the current piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }

            // generate bishop moves
            if ((side == white) ? piece == B : piece == b)
            {
                // loop over source squares of piece bitboard copy
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);

                    // init piece attacks in order to get set of target squares
                    attacks = Attacks::get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                    // loop over target squares available from generated attacks
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // quite move
                        if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                        else
                            // capture move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b in current attacks set
                        pop_bit(attacks, target_square);
                    }


                    // pop ls1b of the current piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }

            // generate rook moves
            if ((side == white) ? piece == R : piece == r)
            {
                // loop over source squares of piece bitboard copy
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);

                    // init piece attacks in order to get set of target squares
                    attacks = Attacks::get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                    // loop over target squares available from generated attacks
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // quite move
                        if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                        else
                            // capture move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b in current attacks set
                        pop_bit(attacks, target_square);
                    }


                    // pop ls1b of the current piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }

            // generate queen moves
            if ((side == white) ? piece == Q : piece == q)
            {
                // loop over source squares of piece bitboard copy
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);

                    // init piece attacks in order to get set of target squares
                    attacks = Attacks::get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                    // loop over target squares available from generated attacks
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // quite move
                        if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                        else
                            // capture move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b in current attacks set
                        pop_bit(attacks, target_square);
                    }


                    // pop ls1b of the current piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }

            // generate king moves
            if ((side == white) ? piece == K : piece == k)
            {
                // loop over source squares of piece bitboard copy
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);

                    // init piece attacks in order to get set of target squares
                    attacks = Attacks::king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                    // loop over target squares available from generated attacks
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // quite move
                        if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                        else
                            // capture move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b in current attacks set
                        pop_bit(attacks, target_square);
                    }

                    // pop ls1b of the current piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }
        }
    }

    // make move on chess board
    int make_move(int move, int move_flag)
    {
        // quite moves
        if (move_flag == all_moves)
        {
            // preserve board state
            copy_board();

            // parse move
            int source_square = get_move_source(move);
            int target_square = get_move_target(move);
            int piece = get_move_piece(move);
            int promoted_piece = get_move_promoted(move);
            int capture = get_move_capture(move);
            int double_push = get_move_double(move);
            int enpass = get_move_enpassant(move);
            int castling = get_move_castling(move);

            // move piece
            pop_bit(bitboards[piece], source_square);
            set_bit(bitboards[piece], target_square);

            // hash piece
            hash_key ^= Zobrist::piece_keys[piece][source_square]; // remove piece from source square in hash key
            hash_key ^= Zobrist::piece_keys[piece][target_square]; // set piece to the target square in hash key

            // handling capture moves
            if (capture)
            {
                // pick up bitboard piece index ranges depending on side
                int start_piece, end_piece;

                // white to move
                if (side == white)
                {
                    start_piece = p;
                    end_piece = k;
                }

                // black to move
                else
                {
                    start_piece = P;
                    end_piece = K;
                }

                // loop over bitboards opposite to the current side to move
                for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
                {
                    // if there's a piece on the target square
                    if (get_bit(bitboards[bb_piece], target_square))
                    {
                        // remove it from corresponding bitboard
                        pop_bit(bitboards[bb_piece], target_square);

                        // remove the piece from hash key
                        hash_key ^= Zobrist::piece_keys[bb_piece][target_square];

                        break;
                    }
                }
            }

            // handle pawn promotions
            if (promoted_piece)
            {
                // white to move
                if (side == white)
                {
                    // erase the pawn from the target square
                    pop_bit(bitboards[P], target_square);

                    // remove pawn from hash key
                    hash_key ^= Zobrist::piece_keys[P][target_square];
                }

                // black to move
                else
                {
                    // erase the pawn from the target square
                    pop_bit(bitboards[p], target_square);

                    // remove pawn from hash key
                    hash_key ^= Zobrist::piece_keys[p][target_square];
                }

                // set up promoted piece on chess board
                set_bit(bitboards[promoted_piece], target_square);

                // add promoted piece into the hash key
                hash_key ^= Zobrist::piece_keys[promoted_piece][target_square];
            }

            // handle enpassant captures
            if (enpass)
            {
                // erase the pawn depending on side to move
                (side == white) ? pop_bit(bitboards[p], target_square + 8) :
                    pop_bit(bitboards[P], target_square - 8);

                // white to move
                if (side == white)
                {
                    // remove captured pawn
                    pop_bit(bitboards[p], target_square + 8);

                    // remove pawn from hash key
                    hash_key ^= Zobrist::piece_keys[p][target_square + 8];
                }

                // black to move
                else
                {
                    // remove captured pawn
                    pop_bit(bitboards[P], target_square - 8);

                    // remove pawn from hash key
                    hash_key ^= Zobrist::piece_keys[P][target_square - 8];
                }
            }

            // hash enpassant if available (remove enpassant square from hash key )
            if (enpassant != no_sq) hash_key ^= Zobrist::enpassant_keys[enpassant];

            // reset enpassant square
            enpassant = no_sq;

            // handle double pawn push
            if (double_push)
            {
                // white to move
                if (side == white)
                {
                    // set enpassant square
                    enpassant = target_square + 8;

                    // hash enpassant
                    hash_key ^= Zobrist::enpassant_keys[target_square + 8];
                }

                // black to move
                else
                {
                    // set enpassant square
                    enpassant = target_square - 8;

                    // hash enpassant
                    hash_key ^= Zobrist::enpassant_keys[target_square - 8];
                }
            }

            // handle castling moves
            if (castling)
            {
                // switch target square
                switch (target_square)
                {
                    // white castles king side
                case (g1):
                    // move H rook
                    pop_bit(bitboards[R], h1);
                    set_bit(bitboards[R], f1);
                    break;

                    // white castles queen side
                case (c1):
                    // move A rook
                    pop_bit(bitboards[R], a1);
                    set_bit(bitboards[R], d1);
                    break;

                    // black castles king side
                case (g8):
                    // move H rook
                    pop_bit(bitboards[r], h8);
                    set_bit(bitboards[r], f8);
                    break;

                    // black castles queen side
                case (c8):
                    // move A rook
                    pop_bit(bitboards[r], a8);
                    set_bit(bitboards[r], d8);
                    break;
                }
            }

            // hash castling
            hash_key ^= Zobrist::castle_keys[castle];

            // update castling rights
            castle &= castling_rights[source_square];
            castle &= castling_rights[target_square];

            // hash castling
            hash_key ^= Zobrist::castle_keys[castle];

            // reset occupancies
            memset(occupancies, 0ULL, 24);

            // loop over white pieces bitboards
            for (int bb_piece = P; bb_piece <= K; bb_piece++)
                // update white occupancies
                occupancies[white] |= bitboards[bb_piece];

            // loop over black pieces bitboards
            for (int bb_piece = p; bb_piece <= k; bb_piece++)
                // update black occupancies
                occupancies[black] |= bitboards[bb_piece];

            // update both sides occupancies
            occupancies[both] |= occupancies[white];
            occupancies[both] |= occupancies[black];

            // change side
            side ^= 1;

            // hash side
            hash_key ^= Zobrist::side_key;

            // make sure that king has not been exposed into a check
            if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k]) : get_ls1b_index(bitboards[K]), side))
            {
                // take move back
                take_back();

                // return illegal move
                return 0;
            }

            //
            else
                // return legal move
                return 1;


        }

        // capture moves
        else
        {
            // make sure move is the capture
            if (get_move_capture(move))
                make_move(move, all_moves);

            // otherwise the move is not a capture
            else
                // don't make it
                return 0;
        }

    }
}