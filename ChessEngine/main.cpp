#include <iostream>
#include <cstring>
#include <optional>

#include "ChessBoardWeights.h"
#include "MinMax.h"

//pawn endgame:  8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1

#if defined(__linux) || defined(__linux__) || defined(linux)
# define LINUX
#elif defined(__APPLE__)
# define MACOS
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(_WIN64)
# define WINDOWS
#endif

float ifAroundKing(int kingsPosition, int currentPosition, bool white) {
    if (kingsPosition - 1 == currentPosition || kingsPosition + 1 == currentPosition ||
        kingsPosition + 8 == currentPosition || kingsPosition - 8 == currentPosition) {
        if (white) {
            return 0.5;
        } else {
            return -0.5;
        }
    } else {
        return 0.0;
    }
}

float evaluate(thc::ChessRules &board) {
    static constexpr float boardPositionWeightDivisor = 100.f;
    float val = 0.f;
    for (int i = 0; i < 64; i++) {
        switch (board.squares[i]) {
            case 'P':
                val += 1;
                val += pawnsOnBoardPositions[i] / boardPositionWeightDivisor;
                break;
            case 'R':
                val += 5;
                val += rookOnBoardPositions[i] / boardPositionWeightDivisor;
                break;
            case 'N':
                val += 3;
                val += knightsOnBoardPositions[i] / boardPositionWeightDivisor;
                break;
            case 'B':
                val += 3;
                val += bishopsOnBoardPositions[i] / boardPositionWeightDivisor;
                break;
            case 'Q':
                val += 9;
                val += queenOnBoardPositions[i] / boardPositionWeightDivisor;
                break;
            case 'K':
                val += kingOnBoardPositions[i] / boardPositionWeightDivisor;
                break;

                // black
            case 'p':
                val -= 1;
                val -= pawnsOnBoardPositions[63 - i] / boardPositionWeightDivisor;
                break;
            case 'r':
                val -= 5;
                val -= rookOnBoardPositions[63 - i] / boardPositionWeightDivisor;
                break;
            case 'n':
                val -= 3;
                val -= knightsOnBoardPositions[63 - i] / boardPositionWeightDivisor;
                break;
            case 'b':
                val -= 3;
                val -= bishopsOnBoardPositions[63 - i] / boardPositionWeightDivisor;
                break;
            case 'q':
                val -= 9;
                val -= queenOnBoardPositions[63 - i] / boardPositionWeightDivisor;
                break;
            case 'k':
                val -= kingOnBoardPositions[63 - i] / boardPositionWeightDivisor;
                break;
        }

//        val += ifAroundKing(board.bking_square, i, true);
//        val -= ifAroundKing(board.wking_square, i, false);
    }
    return val;
}

extern "C" {
#ifdef WINDOWS
__declspec(dllexport) float run(const char *fenInput, char moveOutput[6]) {
#else
    float run(const char *fenInput, char moveOutput[6]) {
#endif
    MinMax minMax(30, evaluate);
    thc::ChessRules board;
    board.Forsyth(fenInput);
    auto[move, eval] = minMax.run(board);
    strcpy(moveOutput, move.TerseOut().c_str());
    return eval;
}
}


void display_position(thc::ChessRules &cr) {
    std::string fen = cr.ForsythPublish();
    std::string s = cr.ToDebugStr();
    printf("FEN (Forsyth Edwards Notation) = %s\n", fen.c_str());
    printf("Position = %s\n", s.c_str());
}

void test1();

void test2();

void test3();

void test4();

void test5();

int main() {
    test1();
}

void test1() {
    std::vector<std::string> testPositions = {
            "1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - 1 1",
            "3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - - 1 1",
            "2q1rr1k/3bbnnp/p2p1pp1/2pPp3/PpP1P1P1/1P2BNNP/2BQ1PRK/7R b - - 1 1",
            "rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq - 1 1",
            "r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - - 1 1",
            "2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - - 1 1",
            "1nk1r1r1/pp2n1pp/4p3/q2pPp1N/b1pP1P2/B1P2R2/2P1B1PP/R2Q2K1 w - - 1 1",
            "4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - - 1 1",
            "2kr1bnr/pbpq4/2n1pp2/3p3p/3P1P1B/2N2N1Q/PPP3PP/2KR1B1R w - - 1 1",
            "3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - - 1 1",
            "2r1nrk1/p2q1ppp/bp1p4/n1pPp3/P1P1P3/2PBB1N1/4QPPP/R4RK1 w - - 1 1",
            "r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - - 1 1",
            "r2q1rk1/4bppp/p2p4/2pP4/3pP3/3Q4/PP1B1PPP/R3R1K1 w - - 1 1",
            "rnb2r1k/pp2p2p/2pp2p1/q2P1p2/8/1Pb2NP1/PB2PPBP/R2Q1RK1 w - - 1 1",
            "2r3k1/1p2q1pp/2b1pr2/p1pp4/6Q1/1P1PP1R1/P1PN2PP/5RK1 w - - 1 1",
            "r1bqkb1r/4npp1/p1p4p/1p1pP1B1/8/1B6/PPPN1PPP/R2Q1RK1 w kq - 1 1",
            "r2q1rk1/1ppnbppp/p2p1nb1/3Pp3/2P1P1P1/2N2N1P/PPB1QP2/R1B2RK1 b - - 1 1",
            "r1bq1rk1/pp2ppbp/2np2p1/2n5/P3PP2/N1P2N2/1PB3PP/R1B1QRK1 b - - 1 1",
            "3rr3/2pq2pk/p2p1pnp/8/2QBPP2/1P6/P5PP/4RRK1 b - - 1 1",
            "r4k2/pb2bp1r/1p1qp2p/3pNp2/3P1P2/2N3P1/PPP1Q2P/2KRR3 w - - 1 1",
            "3rn2k/ppb2rpp/2ppqp2/5N2/2P1P3/1P5Q/PB3PPP/3RR1K1 w - - 1 1",
            "2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 b - - 1 1",
            "r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq - 1 1",
            "r2qnrnk/p2b2b1/1p1p2pp/2pPpp2/1PP1P3/PRNBB3/3QNPPP/5RK1 w - - 1 1"
    };

    MinMax minMax(6, evaluate);
    thc::ChessRules board;

    uint32_t testTime = 0;

    for (const auto &fen: testPositions) {
        board.Forsyth(fen.c_str());
        minMax.reset();

        std::cout << fen << "\n";
        auto start = std::chrono::high_resolution_clock::now();
        auto[move, eval] = minMax.run(board);
        auto end = std::chrono::high_resolution_clock::now();
        testTime += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Move: " << move.TerseOut() << " | Eval: " << eval << "\n";
        std::cout << "\n\n";
    }

    std::cout << "\n\nTEST TIME: " << testTime << " ms\n\n";
#ifdef DEBUG_STATS
    minMax.printDebugStats();
#endif
}

void test2() {
    MinMax minMax(5, evaluate);
    thc::ChessRules board;
    board.Forsyth("2q1rr1k/3bbnnp/p2p1pp1/2pPp3/PpP1P1P1/1P2BNNP/2BQ1PRK/7R b - - 1 1");
    uint32_t testTime = 0;
    constexpr int numberOfMovesToPlay = 20;
    std::array<std::string, numberOfMovesToPlay> movesToPlay{
            "d7g4",
            "h3g4",
            "c8g4",
            "f3e1",
            "h8g8",
            "g3e2",
            "g4d7",
            "e3h6",
            "g6g5",
            "h6g7",
            "g8g7",
            "a4a5",
            "f8h8",
            "c2b1",
            "e8d8",
            "e2g3",
            "g7g8",
            "g3f5",
            "e7f8",
            "g2g4"
    };

    for (int i = 0; i < numberOfMovesToPlay; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        auto[move, eval] = minMax.run(board);

        auto end = std::chrono::high_resolution_clock::now();
        testTime += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Move: " << move.TerseOut() << " | Eval: " << eval << "\n";

        std::cout << "Play: " << movesToPlay[i] << "\n";
        thc::Move moveToPlay;
        if (!moveToPlay.NaturalIn(&board, movesToPlay[i].c_str())) {
            std::cout << "\nCOS POSZLO NIE TAK\n";
            break;
        }
        board.PlayMove(moveToPlay);
    }
    std::cout << "\n\nTEST TIME: " << testTime << " ms\n\n";
#ifdef DEBUG_STATS
    minMax.printDebugStats();
#endif
}

void test3() {
    thc::ChessRules board;
    thc::Move move;
    move.NaturalIn(&board, "e2e4");
    board.PlayMove(move);
    move.NaturalIn(&board, "b8a6");
    board.PlayMove(move);
    move.NaturalIn(&board, "e4e5");
    board.PlayMove(move);
    move.NaturalIn(&board, "a8b8");
    board.PlayMove(move);
    move.NaturalIn(&board, "e5e6");
    board.PlayMove(move);
    display_position(board);

    MinMax minMax(2, evaluate);

    auto[minMaxBestMove, eval] = minMax.run(board);
    std::cout << "Move: " << minMaxBestMove.TerseOut() << " | Eval: " << eval << "\n";
    auto evalAfterPlayingMoves = evaluate(board);
    std::cout << "eval: " << eval << " eval after playing moves: " << evalAfterPlayingMoves << "\n";
    display_position(board);

    std::cout << "\n";
}

void test4() {
    MinMax minMax(25, evaluate);
    thc::ChessRules board;

    uint32_t testTime = 0;

    std::string fen = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1";

    board.Forsyth(fen.c_str());
    minMax.reset();

    std::cout << fen << "\n";
    auto start = std::chrono::high_resolution_clock::now();
    auto[move, eval] = minMax.run(board);
    auto end = std::chrono::high_resolution_clock::now();
    testTime += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Move: " << move.TerseOut() << " | Eval: " << eval << "\n";
    std::cout << "\n\n";


    std::cout << "\n\nTEST TIME: " << testTime << " ms\n\n";
#ifdef DEBUG_STATS
    minMax.printDebugStats();
#endif
}

void test5() {
    MinMax minMax(6, evaluate);
    thc::ChessRules board;

    uint32_t testTime = 0;

    std::string fen = "r2q1rk1/4bppp/p2p4/2pP4/3pP3/3Q4/PP1B1PPP/R3R1K1 w - - 1 1";

    board.Forsyth(fen.c_str());
    minMax.reset();

    std::cout << fen << "\n";
    auto start = std::chrono::high_resolution_clock::now();
    auto[move, eval] = minMax.run(board);
    auto end = std::chrono::high_resolution_clock::now();
    testTime += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Move: " << move.TerseOut() << " | Eval: " << eval << "\n";
    std::cout << "\n\n";


    std::cout << "\n\nTEST TIME: " << testTime << " ms\n\n";
#ifdef DEBUG_STATS
    minMax.printDebugStats();
#endif
}