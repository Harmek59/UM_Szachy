#include "chess_rules/chess.cpp"
#include <iostream>
#include <random>
#include <chrono>
#include <unordered_map>


constexpr size_t knownPositionsSizeLimit = 100'000'000;
using KnownPositionsType = std::unordered_map<std::string, int>;
// int is remaining depth after getting to this position (it is possible to get to known position, but with better depth)

KnownPositionsType knownPositions;


float evaluate(chess::Board &board) {
    if (board.is_game_over()) {
        if (board.result() == "1-0") {
            return 100.f;
        } else if (board.result() == "1-0") {
            return -100.f;
        } else {
            return 0.f;
        }
    } else {
        float val = 0.f;
        std::array<float, 6> pieceValues{1.f, 3.f, 3.f, 5.f, 9.f, 0.f};// 0 for king
        for (int i = 0; i < 6; i++) {
            auto pieceType = chess::PIECE_TYPES[i];
            auto positions = board.pieces(pieceType, chess::WHITE);
            val += pieceValues[i] * positions.size();

            positions = board.pieces(pieceType, chess::BLACK);
            val -= pieceValues[i] * positions.size();
        }
        return val;
    }
}

chess::Move currentBestAction = chess::Move::null();
int currentMaxDepth;

float minMax(chess::Board board, int depth, float alpha = std::numeric_limits<float>::lowest(),
             float beta = std::numeric_limits<float>::max()) {
    if (board.is_game_over() || depth == currentMaxDepth) {
        return evaluate(board);
    }

    if (board.turn == chess::WHITE) {
        float val = std::numeric_limits<float>::lowest();
        for (auto move : board.legal_moves()) {
            chess::Board boardAfterMove = board;
            boardAfterMove.push(move);

            // check is it known position
            auto fen = boardAfterMove.fen();
            fen = fen.substr(0, fen.find(' '));
            if (knownPositions.contains(fen)) {
                if (knownPositions[fen] > currentMaxDepth - depth) {
                    continue;
                } else {
                    knownPositions.at(fen) = currentMaxDepth - depth;
                }
            } else {
                if (knownPositions.size() < knownPositionsSizeLimit) {
                    knownPositions[fen] = currentMaxDepth - depth;
                }
            }

            float new_val = minMax(boardAfterMove, depth + 1, alpha, beta);

            if (new_val > val) {
                val = new_val;
            }

            if (new_val >= beta) {
                return val;
            }

            if (new_val > alpha) {
                alpha = new_val;
                if (depth == 0) {
                    currentBestAction = move;
                }
            }

        }
        return val;
    } else {
        float val = std::numeric_limits<float>::max();
        for (auto move : board.legal_moves()) {
            chess::Board boardAfterMove = board;
            boardAfterMove.push(move);

            // check is it known position
            auto fen = boardAfterMove.fen();
            fen = fen.substr(0, fen.find(' '));
            if (knownPositions.contains(fen)) {
                if (knownPositions[fen] > currentMaxDepth - depth) {
                    continue;
                } else {
                    knownPositions.at(fen) = currentMaxDepth - depth;
                }
            } else {
                if (knownPositions.size() < knownPositionsSizeLimit) {
                    knownPositions[fen] = currentMaxDepth - depth;
                }
            }

            float new_val = minMax(boardAfterMove, depth + 1, alpha, beta);

            if (new_val < val) {
                val = new_val;
            }

            if (new_val <= alpha) {
                return val;
            }

            if (new_val < beta) {
                beta = new_val;
                if (depth == 0) {
                    currentBestAction = move;
                }
            }

        }
        return val;
    }

}


std::pair<chess::Move, float> aiMove(const chess::Board &board) {
    int maxDepth = 5;
    float eval = 0.f;
    currentBestAction = chess::Move::null();

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    auto t1 = high_resolution_clock::now();
    for (int depth = 1; depth <= maxDepth; depth++) {
        currentMaxDepth = depth;
        std::cout << "Current depth: " << depth << " |";
        eval = minMax(board, 0);
    }
    auto t2 = high_resolution_clock::now();
    auto ms_int = duration_cast<milliseconds>(t2 - t1);
    std::cout << "time: " << ms_int.count() << "ms\n";
    std::cout << "\n";

    return {currentBestAction, eval};
}

int main() {
    system("chcp 65001");   //enable unicode
    srand(time(NULL));

    chess::Board board;

    knownPositions.reserve(knownPositionsSizeLimit);

    std::optional<chess::Color> humanPlayer = std::nullopt;
//    humanPlayer = chess::WHITE;


    while (!board.is_game_over(false)) {
        std::cout << board.unicode(false, true, " ") << std::endl;

        if (humanPlayer && humanPlayer == board.turn) {
            while (true) {
                std::cout << board.ply() + 1 << ". " << (board.turn ? "[WHITE] " : "[BLACK] ") << "Enter Move: ";
                std::string san;
                std::cin >> san;
                std::cout << std::endl;


                try {
                    chess::Move move = board.parse_san(san);
                    if (!move) {
                        throw std::invalid_argument("");
                    }
                    board.push(move);
                    break;
                } catch (std::invalid_argument) {
                    std::cout << "Invalid Move, Try Again..." << std::endl;
                }
            }
        } else {
            std::cout << board.ply() + 1 << ". AI move" << (board.turn ? "[WHITE] " : "[BLACK] ");
            if (!humanPlayer) {
//                std::cout << "Press any key.\n";
//                std::getchar();
            } else {
                std::cout << "\n";
            }
            auto[move, eval] = aiMove(board);
            std::cout << std::string(move) << " | Evaluation: " << eval << std::endl;
            board.push(move);
        }
    }

    std::cout << board.unicode(false, true, " ") << std::endl;

    std::cout << "Game Over! Result: " << board.result(true);
}