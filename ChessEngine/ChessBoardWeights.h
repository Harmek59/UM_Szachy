#pragma once

#include <vector>
#include <map>

const std::vector<int> pawnsOnBoardPositions = {
        0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5, 5, 10, 25, 25, 10, 5, 5,
        0, 0, 0, 20, 20, 0, 0, 0,
        5, -5, -10, 0, 0, -10, -5, 5,
        5, 10, 10, -20, -20, 10, 10, 5,
        0, 0, 0, 0, 0, 0, 0, 0
};

const std::vector<int> knightsOnBoardPositions = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -30, 0, 10, 15, 15, 10, 0, -30,
        -30, 5, 15, 20, 20, 15, 5, -30,
        -30, 0, 15, 20, 20, 15, 0, -30,
        -30, 5, 10, 15, 15, 10, 5, -30,
        -40, -20, 0, 5, 5, 0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
};

const std::vector<int> bishopsOnBoardPositions = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 10, 10, 5, 0, -10,
        -10, 5, 5, 10, 10, 5, 5, -10,
        -10, 0, 10, 10, 10, 10, 0, -10,
        -10, 10, 10, 10, 10, 10, 10, -10,
        -10, 5, 0, 0, 0, 0, 5, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
};

const std::vector<int> rookOnBoardPositions = {
        0,  0,  0,  0,  0,  0,  0,  0,
        5,  10, 10, 10, 10, 10, 10, 5,
        -5, 0,  0,  0,  0,  0,  0,  -5,
        -5, 0,  0,  0,  0,  0,  0,  -5,
        -5, 0,  0,  0,  0,  0,  0,  -5,
        -5, 0,  0,  0,  0,  0,  0,  -5,
        -5, 0,  0,  0,  0,  0,  0,  -5,
        0,  0,  0,  5,  5,  0,  0,  0
};

const std::vector<int> queenOnBoardPositions = {
        -20, -10, -10, -5, -5, -10, -10, -20,
        -10, 0,   0,   0,  0,  0,   0,   -10,
        -10, 0,   5,   5,  5,  5,   0,   -10,
        -5,  0,   5,   5,  5,  5,   0,   -5,
        0,   0,   5,   5,  5,  5,   0,   -5,
        -10, 5,   5,   5,  5,  5,   0,   -10,
        -10, 0,   5,   0,  0,  0,   0,   -10,
        -20, -10, -10, -5, -5, -10, -10, -20
};

const std::vector<int> kingOnBoardPositions = {
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -10, -20, -20, -20, -20, -20, -20, -10,
        20,  20,  0,   0,   0,   0,   20,  20,
        20,  30,  10,  0,   0,   10,  30,  20
};

std::map<char, std::vector<int>> mapOfPiecePositions
        {
                {'p', pawnsOnBoardPositions},
                {'k', knightsOnBoardPositions},
                {'b', bishopsOnBoardPositions},
                {'r', rookOnBoardPositions},
                {'q', queenOnBoardPositions},
                {'k', kingOnBoardPositions}
        };