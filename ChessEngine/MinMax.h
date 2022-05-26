#pragma once

#include <functional>
#include <chrono>
#include <unordered_map>
#include <iostream>
#include <array>
#include "chess_rules/thc.h"

//#define SAVE_MOVE_SEQUENCE

//TODO its is broken right now, because it does not save moveSequence properly
//TODO TO W OGOLE NIE DZIALA
//#define MOVE_ORDERING

#ifdef MOVE_ORDERING
#ifndef SAVE_MOVE_SEQUENCE
static_assert(false, "MOVE_ORDERING REQUIRES SAVE_MOVE_SEQUENCE");
#endif
#endif

//#define HASH_TABLE
#define ALPHA_BETA


#define TIME_IS_OUT

//#define DEBUG_STATS

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

#ifdef SAVE_MOVE_SEQUENCE
    static constexpr int MoveSequenceSizeLimit = 20;

    struct MoveSequence {
        //first move in array is the last in sequence(moves are reversed)
        std::array<thc::Move, MoveSequenceSizeLimit> moves;
        int size = 0;

        void add(thc::Move move) {
            moves[size++] = move;
        }
    };

#endif

    using KnownPositionsType = std::unordered_map<std::pair<uint64_t, int>, std::pair<int, float>, PairHash>;
    // int is remaining depth after getting to this position (it is possible to get to known position, but with better depth)

    MinMax(int maxDepth_, std::function<float(thc::ChessRules &board)> evaluationFunction_) : evaluationFunction{
            evaluationFunction_}, maxDepth{maxDepth_} {
#ifdef SAVE_MOVE_SEQUENCE
        if (maxDepth_ >= MoveSequenceSizeLimit) {
            std::cout << "\n[WARNING] Move sequence max size is 20. Changing max depth to 20 !!!\n\n";
            maxDepth = 20;
        }
#endif
        knownPositions.reserve(knownPositionsSizeLimit);
        reset();
#ifdef DEBUG_STATS
        resetDebugStats();
#endif
    }

    void reset() {
        knownPositions.clear();
#ifdef SAVE_MOVE_SEQUENCE
        bestMoveSequence.size = 0;
#endif
    }

#ifdef DEBUG_STATS

    void printDebugStats() const {
        std::cout << "Timings:\n";
        for (uint32_t i = 0; i < timingsIndicesToNames.size(); i++) {
            std::cout << "\t" << timingsIndicesToNames.at(i) << ": " << double(accumulatedTimings[i]) / 1000.0
                      << " ms\n";
        }
        std::cout << "\nEvaluation function invoke counter: " << evaluationFunctionInvokeCounter << "\n";
    }

    void resetDebugStats() {
        evaluationFunctionInvokeCounter = 0;
        for (auto &v : accumulatedTimings) {
            v = 0;
        }
    }

#endif
#ifdef SAVE_MOVE_SEQUENCE

    MoveSequence getMoveSequence() const {
        return bestMoveSequence;
    }

#endif

    std::pair<thc::Move, float> run(thc::ChessRules board) {
        constexpr int depthIncrement = 2;
        float eval = 0.f;
        currentBestAction.Invalid();
        bestAction.Invalid();
#ifdef SAVE_MOVE_SEQUENCE
        bestMoveSequence.size = 0;
#endif
        std::cout << "Depth: ";
        minMaxStart = std::chrono::high_resolution_clock::now();
#ifdef TIME_IS_OUT
        try {
#endif
#ifdef SAVE_MOVE_SEQUENCE
            MoveSequence moveSequence;
#endif
            for (int depth = 2; depth <= maxDepth; depth += depthIncrement) {
                currentMaxDepth = depth;
                std::cout << depth << " | ";
#ifdef SAVE_MOVE_SEQUENCE
                eval = minMax_clear(board, 0, moveSequence);
                bestMoveSequence = std::move(moveSequence);
#else
                eval = minMax(board, 0);
#endif
                bestAction = currentBestAction;
                if (eval == 1000.f || eval == -1000.f) {
                    break;
                }
            }
#ifdef TIME_IS_OUT
        } catch (TimeIsOut &) {
            std::cout << "TIME IS OUT: finished depth: " << currentMaxDepth - depthIncrement << std::endl;
        }
#endif
        auto minMaxEnd = std::chrono::high_resolution_clock::now();
        auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(minMaxEnd - minMaxStart);
        std::cout << "Time: " << ms_int.count() << "ms\n";

        return {bestAction, eval};
    }


private:
#ifdef TIME_IS_OUT
    struct TimeIsOut {
    };
#endif
#ifdef SAVE_MOVE_SEQUENCE

    float minMax(thc::ChessRules board, int depth, MoveSequence &moveSequence,
                 float alpha = std::numeric_limits<float>::lowest(),
                 float beta = std::numeric_limits<float>::max()) {
#else

    float minMax(thc::ChessRules board, int depth,
                 float alpha = std::numeric_limits<float>::lowest(),
                 float beta = std::numeric_limits<float>::max()) {
#endif
#ifdef HASH_TABLE
        BeginTiming(5)
        int metaData = 1;
        //TODO TO JEST ZLE, dodasje en passant
        metaData <<= int(board.WhiteToPlay());
        metaData <<= board.wking_allowed();
        metaData <<= board.wqueen_allowed();
        metaData <<= board.bking_allowed();
        metaData <<= board.bqueen_allowed();

        std::pair<uint64_t, int> hash = {board.Hash64Calculate(), metaData};
        if (knownPositions.contains(hash)) {
            if (knownPositions[hash].first >= currentMaxDepth - depth + 1) {
                return knownPositions[hash].second;
            }
        }
        EndTiming(5)

        auto insertBoardToHashTable = [&](std::pair<uint64_t, int> hash, float val) {
            BeginTiming(5)
            if (knownPositions.contains(hash) && knownPositions[hash].first < currentMaxDepth - depth) {
                knownPositions.at(hash) = {currentMaxDepth - depth, val};
            } else if (knownPositions.size() < knownPositionsSizeLimit) {
                knownPositions[hash] = {currentMaxDepth - depth, val};
            }
            EndTiming(5)
        };
#endif


        thc::TERMINAL evalPos;
        BeginTiming(4)
        board.Evaluate(evalPos);
        EndTiming(4)
        if (evalPos != thc::TERMINAL::NOT_TERMINAL || depth == currentMaxDepth) {
            if (evalPos == thc::TERMINAL::TERMINAL_BCHECKMATE) {    //black is mated
#ifdef HASH_TABLE
                insertBoardToHashTable(hash, 1000.f);
#endif
                return 1000.f;
            } else if (evalPos == thc::TERMINAL::TERMINAL_WCHECKMATE) { //white is mated
#ifdef HASH_TABLE
                insertBoardToHashTable(hash, -1000.f);
#endif
                return -1000.f;
            } else if (evalPos == thc::TERMINAL::TERMINAL_BSTALEMATE ||
                       evalPos == thc::TERMINAL::TERMINAL_WSTALEMATE) {    //stalemate
#ifdef HASH_TABLE
                insertBoardToHashTable(hash, 0.f);
#endif
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

#ifdef TIME_IS_OUT
        auto currTime = std::chrono::high_resolution_clock::now();
        auto algTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - minMaxStart);
        if (algTime.count() > 10'000) {
            throw TimeIsOut{};
        }
#endif

        thc::MOVELIST moveList;

        if (board.WhiteToPlay()) {
            float val = std::numeric_limits<float>::lowest();
#ifdef MOVE_ORDERING
            //TODO usun currentMaxDepth == 6 &&
            if (currentMaxDepth == 6 && depth == 0 && bestMoveSequence.size != 0) {
                thc::ChessRules boardAfterMove = board;
                for (int i = bestMoveSequence.size - 1; i >= 0; i--) {
                    boardAfterMove.PlayMove(bestMoveSequence.moves[i]);
                }
#ifdef SAVE_MOVE_SEQUENCE

                MoveSequence tempMoveSequence;
                float new_val = minMax(boardAfterMove, bestMoveSequence.size, tempMoveSequence, alpha, beta);
#else
                float new_val = minMax(boardAfterMove, bestMoveSequence.size, alpha, beta);
#endif

#ifdef HASH_TABLE
                //TODO zrobic zeby sie hashowalo
                //            insertBoardToHashTable(hash, new_val);
#endif


                if (new_val > val) {
                    val = new_val;
#ifdef SAVE_MOVE_SEQUENCE
                    //TODO zrobic zeby dzialalo
                    //                moveSequence = std::move(tempMoveSequence);
                    //                moveSequence.add(move);
#endif
                }
#ifdef ALPHA_BETA
                if (new_val >= beta) {
                    return val;
                }

                if (new_val > alpha) {
                    alpha = new_val;
                    if (depth == 0) {
                        currentBestAction = bestMoveSequence.moves[bestMoveSequence.size - 1];
                    }
                }
#endif
            }
#endif
            BeginTiming(1)
            board.GenLegalMoveList(&moveList);
//            std::sort(moveList.moves, moveList.moves + moveList.count, [&board, this](thc::Move m1, thc::Move m2) {
//            });
            EndTiming(1)

            for (int i = 0; i < moveList.count; i++) {
                auto &move = moveList.moves[i];
                BeginTiming(2)
                thc::ChessRules boardAfterMove = board;
                EndTiming(2)
                BeginTiming(3)
                boardAfterMove.PlayMove(move);
                EndTiming(3)

#ifdef SAVE_MOVE_SEQUENCE
                MoveSequence tempMoveSequence;
                float new_val = minMax(boardAfterMove, depth + 1, tempMoveSequence, alpha, beta);
#else
                float new_val = minMax(boardAfterMove, depth + 1, alpha, beta);
#endif

#ifdef HASH_TABLE
                insertBoardToHashTable(hash, new_val);
#endif


                if (new_val > val) {
                    val = new_val;
                    if (depth == 0) {
                        currentBestAction = move;
                    }
#ifdef SAVE_MOVE_SEQUENCE
                    moveSequence = std::move(tempMoveSequence);
                    moveSequence.add(move);
#endif
                }
#ifdef ALPHA_BETA
                if (new_val >= beta) {
                    return val;
                }

                if (new_val > alpha) {
                    alpha = new_val;
                    if (depth == 0) {
                        currentBestAction = move;
                    }
                }
#endif
            }
            return val;
        } else {
            float val = std::numeric_limits<float>::max();

#ifdef MOVE_ORDERING
            //TODO usun currentMaxDepth == 6 &&
            if (currentMaxDepth == 6 && depth == 0 && bestMoveSequence.size != 0) {
                thc::ChessRules boardAfterMove = board;
                for (int i = bestMoveSequence.size - 1; i >= 0; i--) {
                    boardAfterMove.PlayMove(bestMoveSequence.moves[i]);
                }
#ifdef SAVE_MOVE_SEQUENCE
                MoveSequence tempMoveSequence;
                float new_val = minMax(boardAfterMove, bestMoveSequence.size, tempMoveSequence, alpha, beta);
#else
                float new_val = minMax(boardAfterMove, bestMoveSequence.size, alpha, beta);
#endif

#ifdef HASH_TABLE
                //TODO zrobic zeby sie hashowalo
                //            insertBoardToHashTable(hash, new_val);
#endif


                if (new_val < val) {
                    val = new_val;
#ifdef SAVE_MOVE_SEQUENCE
                    //TODO zrobic zeby dzialalo
                    //    moveSequence = std::move(tempMoveSequence);
                    //    moveSequence.add(move);
#endif
                }
#ifdef ALPHA_BETA
                if (new_val <= alpha) {
                    return val;
                }

                if (new_val < beta) {
                    beta = new_val;
                    if (depth == 0) {
                        currentBestAction = bestMoveSequence.moves[bestMoveSequence.size - 1];
                    }
                }
#endif
            }
#endif

            BeginTiming(1)
            board.GenLegalMoveList(&moveList);
//            std::sort(moveList.moves, moveList.moves + moveList.count, [&board, this](thc::Move m1, thc::Move m2) {
//            });
            EndTiming(1)

            for (int i = 0; i < moveList.count; i++) {
                auto &move = moveList.moves[i];
                BeginTiming(2)
                thc::ChessRules boardAfterMove = board;
                EndTiming(2)
                BeginTiming(3)
                boardAfterMove.PlayMove(move);
                EndTiming(3)

#ifdef SAVE_MOVE_SEQUENCE
                MoveSequence tempMoveSequence;
                float new_val = minMax(boardAfterMove, depth + 1, tempMoveSequence, alpha, beta);
#else
                float new_val = minMax(boardAfterMove, depth + 1, alpha, beta);
#endif
#ifdef HASH_TABLE
                insertBoardToHashTable(hash, new_val);
#endif


                if (new_val < val) {
                    val = new_val;
                    if (depth == 0) {
                        currentBestAction = move;
                    }
#ifdef SAVE_MOVE_SEQUENCE
                    moveSequence = std::move(tempMoveSequence);
                    moveSequence.add(move);
#endif
                }
#ifdef ALPHA_BETA
                if (new_val <= alpha) {
                    return val;
                }

                if (new_val < beta) {
                    beta = new_val;
                    if (depth == 0) {
                        currentBestAction = move;
                    }
                }
#endif
            }
            return val;
        }
    }

#ifdef SAVE_MOVE_SEQUENCE

    float minMax_clear(thc::ChessRules board, int depth, MoveSequence &moveSequence,
                       float alpha = std::numeric_limits<float>::lowest(),
                       float beta = std::numeric_limits<float>::max()) {
#else

    float minMax_clear(thc::ChessRules board, int depth,
                       float alpha = std::numeric_limits<float>::lowest(),
                       float beta = std::numeric_limits<float>::max()) {
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
                return eval;
            }
        }

#ifdef TIME_IS_OUT
        auto currTime = std::chrono::high_resolution_clock::now();
        auto algTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - minMaxStart);
        if (algTime.count() > 10'000) {
            throw TimeIsOut{};
        }
#endif

        thc::MOVELIST moveList;

        if (board.WhiteToPlay()) {
            float val = std::numeric_limits<float>::lowest();
            board.GenLegalMoveList(&moveList);

            for (int i = 0; i < moveList.count; i++) {
                auto &move = moveList.moves[i];
                thc::ChessRules boardAfterMove = board;
                boardAfterMove.PlayMove(move);


#ifdef SAVE_MOVE_SEQUENCE
                MoveSequence tempMoveSequence;
                float new_val = minMax(boardAfterMove, depth + 1, tempMoveSequence, alpha, beta);
#else
                float new_val = minMax(boardAfterMove, depth + 1, alpha, beta);
#endif

                if (new_val > val) {
                    val = new_val;
#ifdef SAVE_MOVE_SEQUENCE
                    moveSequence = std::move(tempMoveSequence);
                    moveSequence.add(move);
#endif
                }
#ifdef ALPHA_BETA
                if (new_val >= beta) {
                    return val;
                }

                if (new_val > alpha) {
                    alpha = new_val;
                    if (depth == 0) {
                        currentBestAction = move;
                    }
                }
#endif
            }
            return val;
        } else {
            float val = std::numeric_limits<float>::max();
            board.GenLegalMoveList(&moveList);

            for (int i = 0; i < moveList.count; i++) {
                auto &move = moveList.moves[i];
                thc::ChessRules boardAfterMove = board;
                boardAfterMove.PlayMove(move);

#ifdef SAVE_MOVE_SEQUENCE
                MoveSequence tempMoveSequence;
                float new_val = minMax(boardAfterMove, depth + 1, tempMoveSequence, alpha, beta);
#else
                float new_val = minMax(boardAfterMove, depth + 1, alpha, beta);
#endif

                if (new_val < val) {
                    val = new_val;
#ifdef SAVE_MOVE_SEQUENCE
                    moveSequence = std::move(tempMoveSequence);
                    moveSequence.add(move);
#endif
                }
#ifdef ALPHA_BETA
                if (new_val <= alpha) {
                    return val;
                }

                if (new_val < beta) {
                    beta = new_val;
                    if (depth == 0) {
                        currentBestAction = move;
                    }
                }
#endif
            }
            return val;
        }
    }

    KnownPositionsType knownPositions;
#ifdef SAVE_MOVE_SEQUENCE
    MoveSequence bestMoveSequence;
#endif

    std::function<float(thc::ChessRules &board)> evaluationFunction;
    int maxDepth;

    thc::Move bestAction;
    thc::Move currentBestAction;
    int currentMaxDepth;

    std::chrono::time_point<std::chrono::steady_clock> minMaxStart;

#ifdef DEBUG_STATS
    uint64_t evaluationFunctionInvokeCounter = 0;
    std::array<uint64_t, 200> accumulatedTimings;
    std::unordered_map<uint32_t, std::string> timingsIndicesToNames{
#ifdef HASH_TABLE
            {5, "Hash table calculations"},
#endif
            {0, "Evaluation function"},
            {1, "Generate legal move list"},
            {2, "Copy board (boardAfterMove)"},
            {3, "Board play move"},
            {4, "Check if game has ended"}
    };
#endif
};
