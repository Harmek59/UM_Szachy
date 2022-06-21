#pragma once

#include <functional>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <array>
#include <numeric>
#include "chess_rules/thc.h"

#define MOVE_ORDERING

//#define GENERATE_MOVES_WITH_ADDITIONAL_DATA

#define TIME_OUT 3'000

//#define DEBUG_STATS
#define HASH_TABLE

#define ANTY_3_FOLD_REPETITION

class MinMax {
public:
    constexpr static size_t knownPositionsSizeLimit = 50'000'000;

    struct PairHash {
        template<class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &pair) const {
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }
    };


    using HashType = std::pair<uint64_t, uint32_t>;
    enum class HashFlag {
        Exact, Alpha, Beta
    };
    struct HashEntry {
        int remainingDepth;
        float evaluation;
        HashFlag hashFlag;
    };

    using KnownPositionsType = std::unordered_map<HashType, HashEntry, PairHash>;
    // int is remaining depth after getting to this position (it is possible to get to known position, but with better depth)


    MinMax(int maxDepth_, std::function<float(thc::ChessRules &board)> evaluationFunction_) : evaluationFunction{
            evaluationFunction_}, maxDepth{maxDepth_} {
        knownPositions.reserve(knownPositionsSizeLimit);
#ifdef ANTY_3_FOLD_REPETITION
        gameHistory.reserve(100);
#endif

        reset();
#ifdef DEBUG_STATS
        resetDebugStats();
#endif
    }

    void reset() {
        knownPositions.clear();
#ifdef ANTY_3_FOLD_REPETITION
        gameHistory.clear();
#endif
    }


#ifdef DEBUG_STATS

    void printDebugStats() const {
        std::cout << "\nEvaluation function invoke counter: " << evaluationFunctionInvokeCounter << "\n";
        std::cout << "\nHash skips counter: " << hashSkipsCounter << "\n";
        std::cout << "\nHash alpha beta cut offs: " << alphaBetaCutOffs << "\n";
    }

    void resetDebugStats() {
        hashSkipsCounter = 0;
        alphaBetaCutOffs = 0;
        evaluationFunctionInvokeCounter = 0;
    }

#endif


    std::pair<thc::Move, float> run(thc::ChessRules board) {
        constexpr int depthIncrement = 2;
        float eval = 0.f;

        std::cout << "Depth: ";
        minMaxStart = std::chrono::high_resolution_clock::now();
#ifdef TIME_OUT
        try {
#endif


        HashType hash = calculateHash(board);

        for (int depth = 2; depth <= maxDepth; depth += depthIncrement) {
            currentMaxDepth = depth;
            std::cout << depth << " | ";
            eval = minMax(board, hash, 0);
            bestMove = currBestMove;
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

#ifdef ANTY_3_FOLD_REPETITION
        board.PushMove(bestMove);
        auto histHash = calculateHash(board);
        gameHistory.insert(histHash);
        board.PopMove(bestMove);
#endif

        return {bestMove, eval};
    }


private:
#ifdef TIME_OUT
    struct TimeOut {
    };
#endif

    float minMax(thc::ChessRules &board, HashType boardHash, int depth,
                 float alpha = std::numeric_limits<float>::lowest(),
                 float beta = std::numeric_limits<float>::max()) {
#ifdef HASH_TABLE
        auto insertBoardToHashTable = [this](HashType hash, float val, HashFlag hashFlag, int depth) {
            if (knownPositions.contains(hash)) {
                if (knownPositions[hash].remainingDepth < currentMaxDepth - depth) {
                    knownPositions.at(hash) = {currentMaxDepth - depth, val, hashFlag};
                }
            } else if (knownPositions.size() < knownPositionsSizeLimit) {
                knownPositions[hash] = {currentMaxDepth - depth, val, hashFlag};
            }
        };
        HashFlag hashFlag = HashFlag::Exact;
        if (board.WhiteToPlay()) {
            hashFlag = HashFlag::Beta;
        } else {
            hashFlag = HashFlag::Alpha;
        }
#endif

#ifdef ANTY_3_FOLD_REPETITION
        if (gameHistory.contains(boardHash)) {
            return 0.f;
        }
#endif

        thc::TERMINAL evalPos;
        board.Evaluate(evalPos);
        if (evalPos != thc::TERMINAL::NOT_TERMINAL || depth == currentMaxDepth) {
            if (evalPos == thc::TERMINAL::TERMINAL_BCHECKMATE) {    //black is mated
                return 1000.f;
            } else if (evalPos == thc::TERMINAL::TERMINAL_WCHECKMATE) { //white is mated
                return -1000.f;
            } else if (evalPos == thc::TERMINAL::TERMINAL_BSTALEMATE ||
                       evalPos == thc::TERMINAL::TERMINAL_WSTALEMATE) {    //stalemate
                return 0.f;
            } else {
                float eval = evaluationFunction(board);
#ifdef DEBUG_STATS
                evaluationFunctionInvokeCounter++;
#endif
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
        std::array<HashType, MAXMOVES> nextBoardHashes;
#ifdef GENERATE_MOVES_WITH_ADDITIONAL_DATA
        static bool check[MAXMOVES];
        static bool mate[MAXMOVES];
        static bool stalemate[MAXMOVES];
#endif
#ifdef MOVE_ORDERING
        static std::array<float, MAXMOVES> movesOrderingWeights;
        static std::array<int, MAXMOVES> permutation;
        std::iota(permutation.begin(), permutation.end(), 0);
        auto calculateMoveWeight = [&](int moveIdx, thc::ChessRules board) -> float {
            float val = 0.f;
#ifdef GENERATE_MOVES_WITH_ADDITIONAL_DATA
            if (mate[moveIdx]) {
                return 1000.f;
            }
            if (stalemate[moveIdx]) {
                return 1000.f;
            }
            if (check[moveIdx]) {
                val += 3.f;
            }
#endif
            thc::Move move = moveList.moves[moveIdx];
#ifdef HASH_TABLE
//            auto hashEntry = knownPositions.find(nextBoardHashes[moveIdx]);
//            if (hashEntry != knownPositions.end()) {
//                if (hashEntry->second.remainingDepth >= currentMaxDepth - (depth + 1)) {
//                    if (hashEntry->second.hashFlag == HashFlag::Exact) {
//                        return 100.f;
//                    } else {
//                        return 5.f;
//                    }
//                } else {
//                    if (board.WhiteToPlay()) {
//                        val += (hashEntry->second.hashFlag == HashFlag::Exact ? 1.f : 0.5f) *
//                               hashEntry->second.evaluation;
//                    } else {
//                        val -= (hashEntry->second.hashFlag == HashFlag::Exact ? 1.f : 0.5f) *
//                               hashEntry->second.evaluation;
//                    }
//                }
//            }
#endif
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


            thc::Square kingSquare;
            if (board.WhiteToPlay()) {
                kingSquare = board.bking_square;
            } else {
                kingSquare = board.wking_square;
            }
            constexpr static std::array<int, 8> squareOffsets = {-9, -8, -7, -1, 1, 7, 8, 9};
            for (auto offset: squareOffsets) {
                if (move.dst == kingSquare + offset) {
                    val += 0.1f;
                }
                if (move.src == kingSquare + offset) {
                    val -= 0.1f;
                }
            }

            static constexpr float boardPositionWeightDivisor = 300.f;
            switch (board.squares[move.src]) {
                case 'P':
                    val += (pawnsOnBoardPositions[move.dst] - pawnsOnBoardPositions[move.src]) /
                           boardPositionWeightDivisor;
                    break;
                case 'R':
                    val += (rookOnBoardPositions[move.dst] - rookOnBoardPositions[move.src]) /
                           boardPositionWeightDivisor;
                    break;
                case 'N':
                    val += (knightsOnBoardPositions[move.dst] - knightsOnBoardPositions[move.src]) /
                           boardPositionWeightDivisor;
                    break;
                case 'B':
                    val += (bishopsOnBoardPositions[move.dst] - bishopsOnBoardPositions[move.src]) /
                           boardPositionWeightDivisor;
                    break;
                case 'Q':
                    val += (queenOnBoardPositions[move.dst] - queenOnBoardPositions[move.src]) /
                           boardPositionWeightDivisor;
                    break;
                case 'K':
                    val += (kingOnBoardPositions[move.dst] - kingOnBoardPositions[move.src]) /
                           boardPositionWeightDivisor;
                    break;

                    // black
                case 'p':
                    val += (pawnsOnBoardPositions[63 - move.dst] - pawnsOnBoardPositions[63 - move.src]) /
                           boardPositionWeightDivisor;
                    break;
                case 'r':
                    val += (rookOnBoardPositions[63 - move.dst] - rookOnBoardPositions[63 - move.src]) /
                           boardPositionWeightDivisor;
                    break;
                case 'n':
                    val += (knightsOnBoardPositions[63 - move.dst] - knightsOnBoardPositions[63 - move.src]) /
                           boardPositionWeightDivisor;
                    break;
                case 'b':
                    val += (bishopsOnBoardPositions[63 - move.dst] - bishopsOnBoardPositions[63 - move.src]) /
                           boardPositionWeightDivisor;
                    break;
                case 'q':
                    val += (queenOnBoardPositions[63 - move.dst] - queenOnBoardPositions[63 - move.src]) /
                           boardPositionWeightDivisor;
                    break;
                case 'k':
                    val += (kingOnBoardPositions[63 - move.dst] - kingOnBoardPositions[63 - move.src]) /
                           boardPositionWeightDivisor;
                    break;
            }


            return val;
        };
#endif

#ifdef GENERATE_MOVES_WITH_ADDITIONAL_DATA
        board.GenLegalMoveList(&moveList, check, mate, stalemate);
#else
        board.GenLegalMoveList(&moveList);
#endif
        // calculate board hashes
        for (int i = 0; i < moveList.count; i++) {
            auto &move = moveList.moves[i];
            board.PushMove(move);

            // TODO
            auto newHash = updateHash(board, boardHash, move);
            nextBoardHashes[i] = newHash;
            board.PopMove(move);
        }
#ifdef MOVE_ORDERING
        //calculate moves weights
        for (int i = 0; i < moveList.count; i++) {
            auto &move = moveList.moves[i];
            movesOrderingWeights[i] = calculateMoveWeight(i, board);
        }
        //sort moves based on weights
        std::sort(permutation.begin(), std::next(permutation.begin(), moveList.count), [&](int idx1, int idx2) {
            return movesOrderingWeights[idx1] > movesOrderingWeights[idx2];
        });
        //apply permutation
        for (int i = 0; i < moveList.count; i++) {
            thc::Move t = moveList.moves[i];
            auto hash = nextBoardHashes[i];
            auto current = i;
            while (i != permutation[current]) {
                auto next = permutation[current];
                moveList.moves[current] = moveList.moves[next];
                nextBoardHashes[current] = nextBoardHashes[next];
                permutation[current] = current;
                current = next;
            }
            moveList.moves[current] = t;
            nextBoardHashes[current] = hash;
            permutation[current] = current;
        }
#endif

        for (int i = 0; i < moveList.count; i++) {
            auto &move = moveList.moves[i];


            board.PushMove(move);
#ifdef HASH_TABLE
            float new_val;
            auto hashEntry = knownPositions.find(nextBoardHashes[i]);
            if (hashEntry != knownPositions.end() &&
                hashEntry->second.remainingDepth >= currentMaxDepth - (depth + 1) &&
                hashEntry->second.hashFlag == HashFlag::Exact) {
#ifdef DEBUG_STATS
                hashSkipsCounter++;
#endif
                new_val = hashEntry->second.evaluation;
            } else if (hashEntry != knownPositions.end() &&
                       hashEntry->second.remainingDepth >= currentMaxDepth - (depth + 1) &&
                       hashEntry->second.hashFlag == HashFlag::Beta && hashEntry->second.evaluation <= alpha) {
#ifdef DEBUG_STATS
                hashSkipsCounter++;
#endif
                new_val = hashEntry->second.evaluation;
            } else if (hashEntry != knownPositions.end() &&
                       hashEntry->second.remainingDepth >= currentMaxDepth - (depth + 1) &&
                       hashEntry->second.hashFlag == HashFlag::Alpha && hashEntry->second.evaluation >= beta) {
#ifdef DEBUG_STATS
                hashSkipsCounter++;
#endif
                new_val = hashEntry->second.evaluation;
            } else {
                new_val = minMax(board, nextBoardHashes[i], depth + 1, alpha, beta);
            }
#else
            float new_val = minMax(board, nextBoardHashes[i], depth + 1, alpha, beta);
#endif
            board.PopMove(move);


            if (board.WhiteToPlay()) {
                if (new_val >= beta) {
#ifdef HASH_TABLE
                    insertBoardToHashTable(boardHash, beta, HashFlag::Alpha, depth);
#endif
#ifdef DEBUG_STATS
                    alphaBetaCutOffs++;
#endif
                    return beta;
                }
                if (new_val > alpha) {
#ifdef HASH_TABLE
                    hashFlag = HashFlag::Exact;
#endif
                    alpha = new_val;
                    if(depth == 0){
                        currBestMove = move;
                    }
                }
            } else {
                if (new_val <= alpha) {
#ifdef HASH_TABLE
                    insertBoardToHashTable(boardHash, alpha, HashFlag::Beta, depth);
#endif
#ifdef DEBUG_STATS
                    alphaBetaCutOffs++;
#endif
                    return alpha;
                }
                if (new_val < beta) {
#ifdef HASH_TABLE
                    hashFlag = HashFlag::Exact;
#endif
                    beta = new_val;
                    if(depth == 0){
                        currBestMove = move;
                    }
                }
            }
        }
#ifdef HASH_TABLE
        insertBoardToHashTable(boardHash, board.WhiteToPlay() ? alpha : beta, hashFlag, depth);
#endif
        return board.WhiteToPlay() ? alpha : beta;
    }

    HashType calculateHash(thc::ChessRules &board) {
        uint32_t metaData = uint32_t(board.enpassant_target);
        metaData += uint32_t(board.WhiteToPlay()) << 10;
        metaData += uint32_t(board.wking_allowed()) << 11;
        metaData += uint32_t(board.wqueen_allowed()) << 12;
        metaData += uint32_t(board.bking_allowed()) << 13;
        metaData += uint32_t(board.bqueen_allowed()) << 14;
        return {board.Hash64Calculate(), metaData};
    }

    HashType updateHash(thc::ChessRules &board, HashType hashToUpdate, thc::Move move) {
        uint32_t metaData = uint32_t(board.enpassant_target);
        metaData += uint32_t(board.WhiteToPlay()) << 10;
        metaData += uint32_t(board.wking_allowed()) << 11;
        metaData += uint32_t(board.wqueen_allowed()) << 12;
        metaData += uint32_t(board.bking_allowed()) << 13;
        metaData += uint32_t(board.bqueen_allowed()) << 14;
        //TODO
        return {board.Hash64Calculate(), metaData};
        return {board.Hash64Update(hashToUpdate.first, move), metaData};
    }

    KnownPositionsType knownPositions;
    thc::Move bestMove;
    thc::Move currBestMove;

    std::function<float(thc::ChessRules &board)> evaluationFunction;
    int maxDepth;
    int currentMaxDepth;

    std::chrono::time_point<std::chrono::steady_clock> minMaxStart;

#ifdef ANTY_3_FOLD_REPETITION
    std::unordered_set<HashType, PairHash> gameHistory;
#endif


#ifdef DEBUG_STATS
    uint64_t evaluationFunctionInvokeCounter = 0;
    uint64_t hashSkipsCounter = 0;
    uint64_t alphaBetaCutOffs = 0;
#endif
};
