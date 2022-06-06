#pragma once

#include <functional>
#include <chrono>
#include <unordered_map>
#include <iostream>
#include <array>
#include <numeric>
#include "chess_rules/thc.h"

#define ALPHA_BETA
#define MOVE_ORDERING

//#define GENERATE_MOVES_WITH_ADDITIONAL_DATA

#define TIME_OUT 3'000

//#define DEBUG_STATS


// TODO: dziala jak gowno
//#define HASH_TABLE


#ifdef DEBUG_STATS
#define BeginTiming(idx) auto start##idx = std::chrono::high_resolution_clock::now();
#define EndTiming(idx) auto end##idx = std::chrono::high_resolution_clock::now(); \
    accumulatedTimings[idx] += std::chrono::duration_cast<std::chrono::microseconds>(end##idx - start##idx).count();
#else
#define BeginTiming(idx)
#define EndTiming(idx)
#endif

class MinMax {
public:
    constexpr static size_t knownPositionsSizeLimit = 50'000'000;

    struct PairHash {
        template<class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &pair) const {
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }
    };

    struct HashEntry {
        uint64_t hash;
        int remainingDepth;
        float evaluation;
    };

    using KnownPositionTableType = std::vector<HashEntry>;
    KnownPositionTableType knownPositionTableType;

    using KnownPositionsType = std::unordered_map<std::pair<uint64_t, uint32_t>, std::pair<int, float>, PairHash>;
    // int is remaining depth after getting to this position (it is possible to get to known position, but with better depth)


    static constexpr int moveSequenceSizeLimit = 10;

    struct MoveSequence {
        //first move in array is the last in sequence(moves are reversed)
        std::array<thc::Move, moveSequenceSizeLimit> moves;
        int size = 0;

        void add(thc::Move move) {
            if (size == moves.size()) {
                for (int i = 1; i < moves.size(); i++) {
                    moves[i - 1] = moves[i];
                }
                moves[moves.size() - 1] = move;
            } else {
                moves[size++] = move;

            }
        }
    };


    MinMax(int maxDepth_, std::function<float(thc::ChessRules &board)> evaluationFunction_) : evaluationFunction{
            evaluationFunction_}, maxDepth{maxDepth_} {
        knownPositions.reserve(knownPositionsSizeLimit);
        knownPositionTableType.resize(knownPositionsSizeLimit, HashEntry{0, 0, 0.f});
        reset();
#ifdef DEBUG_STATS
        resetDebugStats();
#endif
    }

    void reset() {
        knownPositions.clear();
        bestMoveSequence.size = 0;
    }

    MoveSequence getMoveSequence() const {
        return bestMoveSequence;
    }


#ifdef DEBUG_STATS

    void printDebugStats() const {
        std::cout << "Timings:\n";
        for (auto[i, name] : timingsIndicesToNames) {
            std::cout << "\t" << name << ": " << double(accumulatedTimings[i]) / 1000.0
                      << " ms\n";
        }
        std::cout << "\nEvaluation function invoke counter: " << evaluationFunctionInvokeCounter << "\n";
        std::cout << "\nHash skips counter: " << hashSkipsCounter << "\n";
    }

    void resetDebugStats() {
        hashSkipsCounter = 0;
        evaluationFunctionInvokeCounter = 0;
        for (auto &v : accumulatedTimings) {
            v = 0;
        }
    }

#endif


    std::pair<thc::Move, float> run(thc::ChessRules board) {
        constexpr int depthIncrement = 1;
        float eval = 0.f;
        bestMoveSequence.size = 0;

        std::cout << "Depth: ";
        minMaxStart = std::chrono::high_resolution_clock::now();
#ifdef TIME_OUT
        try {
#endif
        MoveSequence moveSequence;
        for (int depth = 2; depth <= maxDepth; depth += depthIncrement) {
            currentMaxDepth = depth;
            std::cout << depth << " | ";
            eval = minMax(board, 0, moveSequence);
            bestMoveSequence = std::move(moveSequence);
            if (eval == 1000.f || eval == -1000.f) {
                break;
            }
        }
#ifdef TIME_OUT
        } catch (TimeOut &) {
            std::cout << "TIME IS OUT: finished depth: " << currentMaxDepth - depthIncrement << std::endl;
        }
#endif
        auto minMaxEnd = std::chrono::high_resolution_clock::now();
        auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(minMaxEnd - minMaxStart);
        std::cout << "Time: " << ms_int.count() << "ms\n";

        return {bestMoveSequence.moves[bestMoveSequence.size - 1], eval};
    }


private:
#ifdef TIME_OUT
    struct TimeOut {
    };
#endif

    float minMax(thc::ChessRules board, int depth, MoveSequence &moveSequence,
                 float alpha = std::numeric_limits<float>::lowest(),
                 float beta = std::numeric_limits<float>::max()) {
#ifdef HASH_TABLE
        BeginTiming(5)
        uint32_t metaData = uint32_t(board.enpassant_target);
        metaData += uint32_t(board.WhiteToPlay()) << 10;
        metaData += uint32_t(board.wking_allowed()) << 11;
        metaData += uint32_t(board.wqueen_allowed()) << 12;
        metaData += uint32_t(board.bking_allowed()) << 13;
        metaData += uint32_t(board.bqueen_allowed()) << 14;

        std::pair<uint64_t, uint32_t> hash = {board.Hash64Calculate(), metaData};
        if (knownPositions.contains(hash)) {
            if (knownPositions[hash].first >= currentMaxDepth - depth + 1) {
                return knownPositions[hash].second;
            }
        }

//        uint64_t hash = board.Hash64Calculate() ^ metaData;
//        uint64_t hashPosition = hash % knownPositionsSizeLimit;
//        auto hashEntry = knownPositionTableType[hashPosition];
//        if (hashPosition == hashEntry.hash % knownPositionsSizeLimit) {
//            //cell is not empty
//            if (hash == hashEntry.hash && hashEntry.remainingDepth >= currentMaxDepth - depth + 1) {
//                hashSkipsCounter++;
//                return hashEntry.evaluation;
//            }
//        }

        EndTiming(5)

        auto insertBoardToHashTable = [&](std::pair<uint64_t, uint32_t> hash, float val) {
            BeginTiming(5)
            if (knownPositions.contains(hash)) {
                if (knownPositions[hash].first < currentMaxDepth - (depth + 1)) {
                    knownPositions.at(hash) = {currentMaxDepth - (depth + 1), val};
                }
            } else if (knownPositions.size() < knownPositionsSizeLimit) {
                knownPositions[hash] = {currentMaxDepth - (depth + 1), val};
            }
            EndTiming(5)
        };


//        auto insertBoardToHashTable = [&](uint64_t hash, float val) {
//            BeginTiming(5)
//            uint64_t hashPosition = hash % knownPositionsSizeLimit;
//            auto hashEntry = knownPositionTableType[hashPosition];
//            if (hashPosition == hashEntry.hash % knownPositionsSizeLimit) {
//                //cell is not empty
//                if (hash == hashEntry.hash && hashEntry.remainingDepth < currentMaxDepth - (depth + 1)) {
//                    knownPositionTableType[hashPosition] = HashEntry{hash, currentMaxDepth - (depth + 1), val};
//                }
//            } else {
//                knownPositionTableType[hashPosition] = HashEntry{hash, currentMaxDepth - (depth + 1), val};
//            }
//            EndTiming(5)
//        };

#endif


        thc::TERMINAL evalPos;
        BeginTiming(4)
        board.Evaluate(evalPos);
        EndTiming(4)
        if (evalPos != thc::TERMINAL::NOT_TERMINAL || depth == currentMaxDepth) {
            if (evalPos == thc::TERMINAL::TERMINAL_BCHECKMATE) {    //black is mated
                return 1000.f;
            } else if (evalPos == thc::TERMINAL::TERMINAL_WCHECKMATE) { //white is mated
                return -1000.f;
            } else if (evalPos == thc::TERMINAL::TERMINAL_BSTALEMATE ||
                       evalPos == thc::TERMINAL::TERMINAL_WSTALEMATE) {    //stalemate
                return 0.f;
            } else {
                BeginTiming(0)
                float eval = evaluationFunction(board);
#ifdef DEBUG_STATS
                evaluationFunctionInvokeCounter++;
#endif
                EndTiming(0)
                return eval;
            }
        }

#ifdef TIME_OUT
        auto currTime = std::chrono::high_resolution_clock::now();
        auto algTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - minMaxStart);
        if (algTime.count() > TIME_OUT) {
            throw TimeOut{};
        }
#endif

        thc::MOVELIST moveList;
#ifdef GENERATE_MOVES_WITH_ADDITIONAL_DATA
        static bool check[MAXMOVES];
        static bool mate[MAXMOVES];
        static bool stalemate[MAXMOVES];
#endif
#ifdef MOVE_ORDERING
        static std::array<float, MAXMOVES> movesOrderingWeights;
        static std::array<int, MAXMOVES> permutation;
        std::iota(permutation.begin(), permutation.end(), 0);
        auto calculateMoveWeight = [&](int moveIdx) -> float {
            float val = 0.f;
#ifdef GENERATE_MOVES_WITH_ADDITIONAL_DATA
            if (mate[moveIdx]) {
                return 1000.f;
            }
            if (stalemate[moveIdx]) {
                return -1000.f;
            }
            if (check[moveIdx]) {
                val += 3.f;
            }
#endif
            thc::Move move = moveList.moves[moveIdx];
            if (depth < bestMoveSequence.size) {
                if (move == bestMoveSequence.moves[bestMoveSequence.size - 1 - depth]) {
                    val += 5.f;
                }
            }
            if (move.capture) {
                auto getPieceValue = [&](thc::Square s) {
                    switch (board.squares[s]) {
                        case 'P':
                            return 1;
                        case 'R':
                            return 5;
                        case 'N':
                            return 3;
                        case 'B':
                            return 3;
                        case 'Q':
                            return 9;
                        case 'p':
                            return 1;
                        case 'r':
                            return 5;
                        case 'n':
                            return 3;
                        case 'b':
                            return 3;
                        case 'q':
                            return 9;
                    }
                    return 0;
                };
                val += std::max(0, std::abs(getPieceValue(move.dst)) -
                                   std::abs(getPieceValue(move.src)));
            }
            if (move.special == thc::SPECIAL_PROMOTION_QUEEN) {
                val += 8;
            }
            return val;
        };
#endif

        BeginTiming(1)
#ifdef GENERATE_MOVES_WITH_ADDITIONAL_DATA
        board.GenLegalMoveList(&moveList, check, mate, stalemate);
#else
        board.GenLegalMoveList(&moveList);
#endif
        EndTiming(1)
#ifdef MOVE_ORDERING
        BeginTiming(6)
        //calculate moves weights
        for (int i = 0; i < moveList.count; i++) {
            auto &move = moveList.moves[i];
            movesOrderingWeights[i] = calculateMoveWeight(i);
        }
        //sort moves based on weights
        std::sort(permutation.begin(), std::next(permutation.begin(), moveList.count), [&](int idx1, int idx2) {
            return movesOrderingWeights[idx1] > movesOrderingWeights[idx2];
        });
        //apply permutation
        for (int i = 0; i < moveList.count; i++) {
            thc::Move t = moveList.moves[i];
            auto current = i;
            while (i != permutation[current]) {
                auto next = permutation[current];
                moveList.moves[current] = moveList.moves[next];
                permutation[current] = current;
                current = next;
            }
            moveList.moves[current] = t;
            permutation[current] = current;
        }
        EndTiming(6)
#endif

        for (int i = 0; i < moveList.count; i++) {
            auto &move = moveList.moves[i];


            BeginTiming(3)
            board.PushMove(move);
            EndTiming(3)

            MoveSequence tempMoveSequence;
            float new_val = minMax(board, depth + 1, tempMoveSequence, alpha, beta);


#ifdef HASH_TABLE
            uint64_t new_hash = board.Hash64Calculate();
            metaData = uint32_t(board.enpassant_target);
            metaData += uint32_t(board.WhiteToPlay()) << 10;
            metaData += uint32_t(board.wking_allowed()) << 11;
            metaData += uint32_t(board.wqueen_allowed()) << 12;
            metaData += uint32_t(board.bking_allowed()) << 13;
            metaData += uint32_t(board.bqueen_allowed()) << 14;
//            insertBoardToHashTable(new_hash ^ metaData, new_val);
            insertBoardToHashTable({new_hash, metaData}, new_val);

#endif
            board.PopMove(move);


            if (board.WhiteToPlay()) {
                if (alpha < new_val) {
                    alpha = new_val;
                    moveSequence = std::move(tempMoveSequence);
                    moveSequence.add(move);
                }
            } else {
                if (beta > new_val) {
                    beta = new_val;
                    moveSequence = std::move(tempMoveSequence);
                    moveSequence.add(move);
                }
            }

#ifdef ALPHA_BETA
            if (alpha >= beta) {
                break;
            }
#endif
        }
        return board.WhiteToPlay() ? alpha : beta;
    }


    KnownPositionsType knownPositions;
    MoveSequence bestMoveSequence;

    std::function<float(thc::ChessRules &board)> evaluationFunction;
    int maxDepth;
    int currentMaxDepth;

    std::chrono::time_point<std::chrono::steady_clock> minMaxStart;

#ifdef DEBUG_STATS
    uint64_t evaluationFunctionInvokeCounter = 0;
    uint64_t hashSkipsCounter = 0;

    std::array<uint64_t, 200> accumulatedTimings;
    std::unordered_map<uint32_t, std::string> timingsIndicesToNames{
#ifdef HASH_TABLE
            {5, "Hash table calculations"},
#endif
#ifdef MOVE_ORDERING
            {6, "Calculate moves weights and sort"},
#endif
            {0, "Evaluation function"},
            {1, "Generate legal move list"},
            {2, "TODO TU NIC NIE MA"},
            {3, "Board play move"},
            {4, "Check if game has ended"}
    };
#endif
};
