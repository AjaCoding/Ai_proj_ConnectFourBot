#ifndef CONN4_H
#define CONN4_H

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <tuple>
#include <cmath>
#include <ctime>
#include <cctype>

using namespace std;

#define NSIZE 8
#define COLS 7
#define ROWS 6
#define P1 "\033[31m"
#define P2 "\033[32m"
#define RESET "\033[0m"



struct Cell {
    int x, y;
    char player;
    int value = 0;
    int lineValue = 0;

    Cell *directions[NSIZE] = {nullptr}; // NW N NE E SE S SW W
    int chains[NSIZE] = {};

    Cell(int x, int y, char player);

    int setValue();

    ~Cell();
};

struct Board;



struct State {
    string stateStr;
    State *parent;
    int depth = 0;
    char curPlayer = '1';
    int value = 0;
    bool isWin = false;

    int aMoves[COLS];
    int pMoves[COLS];
    Cell* cells[COLS * ROWS];
    tuple<int, int> linePairs = make_tuple(0, 0);

    State(State *parent);

    State(string input);

    State(const State &other);

    void prep();

    bool calcNextChains();

    void applyMove(int i);

    string stringify();

    void printState();

    bool hasFourInARow(char player);

    int rateState(char aiPlayer);

    tuple<int, int> minimax(int alpha, int beta);

    ~State();
};



struct Board {
    string start;
    State *startState;
    int moveCount = 0;

    Board(string input);

    int findBlockingMove();

    int findBestMove();

    void play();

    ~Board();
};

#endif