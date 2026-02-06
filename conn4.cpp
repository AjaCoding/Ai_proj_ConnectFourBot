#include "conn4.h"

// Cell
Cell::Cell(int x, int y, char player) : x(x), y(y), player(player) {}

int Cell::setValue() {
    value = 0;
    for (int i : chains)
        value += i;
    return value;
}

Cell::~Cell() {
    // cout << "Deleted cell at (" << x << ", " << y << ")" << endl;
}



// State
State::State(State *parent) : parent(parent), depth(parent->depth + 1) {
    for (int i = 0; i < (COLS * ROWS); i++)
    {
        cells[i] = new Cell(i % COLS, i / COLS, parent->cells[i]->player);
        stateStr += parent->cells[i]->player;
    }
    curPlayer = (parent->curPlayer == '1') ? '2' : '1';
}

State::State(string input) : stateStr(input), parent(nullptr), depth(0)
{
    int count1 = 0, count2 = 0;
    for (int i = 0; i < (COLS * ROWS); i++)
    {
        cells[i] = new Cell(i % COLS, i / COLS, input[i]);
        if (input[i] == '1')
            count1++;
        else if (input[i] == '2')
            count2++;
    }
    // Determine whose turn it is: if equal or 1 fewer '1' then 1's turn.
    curPlayer = (count1 <= count2) ? '1' : '2';
    prep();
}

State::State(const State &other) {
    parent = nullptr;
    depth = other.depth + 1;
    curPlayer = other.curPlayer; // DO NOT toggle here
    value = other.value;

    for (int i = 0; i < COLS; i++)
    {
        aMoves[i] = other.aMoves[i];
        pMoves[i] = other.pMoves[i];
    }
    for (int i = 0; i < COLS * ROWS; i++)
    {
        cells[i] = new Cell(*other.cells[i]); // deep copy
    }
    linePairs = other.linePairs;
}

void State::prep() {
    fill(begin(aMoves), end(aMoves), -1);
    fill(begin(pMoves), end(pMoves), -1);
    stateStr = "";
    for (int i = 0; i < (COLS * ROWS); i++)
    {
        Cell *curCell = cells[i];
        stateStr += curCell->player;

        // 0  1 2  3 4  5 6  7
        // NW N NE E SE S SW W
        if ((i >= COLS) && (i % COLS != 0))
            curCell->directions[0] = cells[(i - COLS) - 1]; // NorthWest
        if (i >= COLS)
            curCell->directions[1] = cells[i - COLS]; // North
        if ((i >= COLS) && ((i + 1) % COLS != 0))
            curCell->directions[2] = cells[(i - COLS) + 1]; // NorthEast
        if ((i + 1) % COLS != 0)
            curCell->directions[3] = cells[i + 1]; // East
        if ((i < (ROWS * COLS) - COLS) && ((i + 1) % COLS != 0))
            curCell->directions[4] = cells[(i + COLS) + 1]; // SouthEast
        if (i < (ROWS * COLS) - COLS)
            curCell->directions[5] = cells[i + COLS]; // South
        if ((i < (ROWS * COLS) - COLS) && (i % COLS != 0))
            curCell->directions[6] = cells[(i + COLS) - 1]; // SouthWest
        if (i % COLS != 0)
            curCell->directions[7] = cells[i - 1]; // West
    }

    // Calculate chain values post neighbors being populated
    for (int i = 0; i < (COLS * ROWS); i++)
    {
        Cell *curCell = cells[i];
        fill(begin(curCell->chains), end(curCell->chains), 0);
        for (int j = 0; j < NSIZE; j++)
        {
            if (curCell->directions[j])
            {
                Cell *checkCell = curCell;
                while (checkCell->directions[j] && checkCell->directions[j]->player == curCell->player && curCell->player != '0')
                {
                    checkCell = checkCell->directions[j];
                    curCell->chains[j]++;
                }
            }
        }
        if (curCell->player == '0')
        {
            aMoves[curCell->x] = curCell->y;
        }
    }
    calcNextChains();
}

bool State::calcNextChains() {
    bool isWinner = false;
    // Iterate through each potential move and collect the player data on all possible moves
    for (int i = 0; i < COLS; i++)
    {
        if (aMoves[i] <= -1 || aMoves[i] >= ROWS) continue;
        Cell *curCell = cells[aMoves[i] * COLS + i]; // Grab available cell from aMoves list
        char placeholder = curCell->player;
        curCell->player = curPlayer;

        fill(begin(curCell->chains), end(curCell->chains), 0);

        for (int j = 0; j < NSIZE; j++)
        {
            if (curCell->directions[j])
            {
                Cell *checkCell = curCell;
                curCell->chains[j] = 0;
                while (checkCell->directions[j] && checkCell->directions[j]->player == curCell->player && curCell->player != '0')
                {
                    checkCell = checkCell->directions[j];
                    curCell->chains[j]++;
                    if (curCell->chains[j] >= 4)
                        isWinner = true;
                }
            }
        }

        for (int j = 0; j < NSIZE / 2; j++)
        {
            int value = curCell->chains[j] + curCell->chains[(j + (NSIZE / 2))] + 1;
            if (pMoves[i] < value)
                pMoves[i] = value;

            if (value > curCell->lineValue)
                curCell->lineValue = value;
        }

        curCell->player = placeholder;
    }

    return isWinner;
}

void State::applyMove(int i) {
    Cell *curCell = cells[aMoves[i] * COLS + i];
    curCell->player = curPlayer;

    curCell->value = curCell->lineValue;

    stateStr[aMoves[i] * COLS + i] = curPlayer;
    if (aMoves[i] > 0)
        aMoves[i]--;
    else
        aMoves[i] = -1;

    curPlayer = (curPlayer == '1') ? '2' : '1';
}

string State::stringify() {
    string output = "";
    for (Cell *cell : cells)
        output += cell->player;
    return output;
}

void State::printState() {
    int i = 0;
    cout << "(1) (2) (3) (4) (5) (6) (7)\n" << endl;
    for (Cell *cell : cells)
    {
        string color = RESET;
        if (cell->player == '1')
            color = P1;
        if (cell->player == '2')
            color = P2;
        cout << "(" << color << cell->player << RESET << ") ";
        i++;
        if (i % COLS == 0)
            cout << endl;
    }
}

bool State::hasFourInARow(char player) {
    // directions: 0=NW,1=N,2=NE,3=E,4=SE,5=S,6=SW,7=W
    for (Cell* cell : cells) {
        if (cell->player != player) continue;

        // Only need 4 directions to check: N, E, NE, SE
        int dirs[4] = {1, 3, 2, 4};
        for (int d : dirs) {
            int count = 1;
            Cell* cur = cell;
            while (cur->directions[d] && cur->directions[d]->player == player) {
                count++;
                cur = cur->directions[d];
            }
            if (count >= 4) return true;
        }
    }
    return false;
}

int State::rateState(char player) {
    int score = 0;

    // Evaluate each cell
    for (Cell* cell : cells) {
        
        if (cell->player == '0') {
            continue;
        }

        // Exponential weighting for chain length
        for (int i = 0; i < 4; i++) { // N, E, NE, SE
            int len = cell->chains[i];
            if (len >= 1) {
                int val = 0;
                if (cell->player == player) val = pow(len, len);    // len^len for AI
                else val = -pow(len, len);                         // len^len penalty for opponent

                // Favor open-ended chains
                bool openStart = cell->directions[i] && cell->directions[i]->player == '0';
                bool openEnd = cell->directions[i + 4] && cell->directions[i + 4]->player == '0';
                if (openStart || openEnd) val *= 2; // double score for open chains

                score += val;
            }
        }
    }

    return score;
}

const int NEG_INF = -999999;
const int POS_INF =  999999;

tuple<int, int> State::minimax(int alpha, int beta) {
    // Check for terminal (win/loss) or max depth
    const int MAX_DEPTH = 6;

    // WE MUST evaluate wins for both players
    if (hasFourInARow('1')) return {POS_INF - depth, -1};
    if (hasFourInARow('2')) return {NEG_INF + depth, -1};

    if (depth >= MAX_DEPTH) {
        int heuristic = rateState(curPlayer);
        return {heuristic, -1};
    }

    tuple<int, int> bestVal;

    // Alternate max and min
    if (curPlayer == '1') { // Max (AI)
        // cout << "Maxing P1" << endl;
        bestVal = make_tuple(NEG_INF, -1);        
        for (int i = 0; i < COLS; i++) {
            if (aMoves[i] == -1)
                continue;
            State newState(*this);
            newState.applyMove(i);
            newState.prep();

            tuple<int, int> moveVal = newState.minimax(alpha, beta);

            if (get<0>(moveVal) > get<0>(bestVal)) {
                //cout << "Win at depth 1 for col: " << i << endl;
                get<0>(bestVal) = get<0>(moveVal);
                get<1>(bestVal) = i;
            }

            // Implement alpha beta pruning
            alpha = max(alpha, get<0>(bestVal));
            // cout << alpha << ", " << beta << endl;
            if (alpha >= beta) {
                // cout << "Alpha cutoff at depth: " << depth << endl;
                break;
            }
        }
        return bestVal;
    }
    else { // Min
        bestVal = make_tuple(POS_INF, -1);
        for (int i = 0; i < COLS; i++) {
            if (aMoves[i] == -1)
                continue;
            State newState(*this);
            newState.applyMove(i);
            newState.prep();

            tuple<int, int> moveVal = newState.minimax(alpha, beta);

            //if (get<1>(moveVal) != -1) return moveVal;
            if (get<0>(moveVal) < get<0>(bestVal)) {
                get<0>(bestVal) = get<0>(moveVal);
                get<1>(bestVal) = i;
            }

            beta = min(beta, get<0>(bestVal));
            if (alpha >= beta) {
                //cout << depth << " beta cut: " << beta << endl;
                break; // cut-off
            }
        }
        return bestVal;
    }
}

State::~State() {
    for (Cell *cell : cells)
        delete cell;
}



// Board
Board::Board(string input) : start(input) {
    startState = new State(input);
}

int Board::findBestMove() {
    int alpha = -999999, beta = 999999;
    tuple<int, int> bestVal;

    State* curState = startState;
    curState->prep();

    bestVal = curState->minimax(alpha, beta);

    cout << "Predicted best move: " << get<1>(bestVal) << " with value " << get<0>(bestVal) << endl;
    return get<1>(bestVal);
}

int Board::findBlockingMove() {
    for (int col = 0; col < COLS; col++) {
        if (startState->aMoves[col] == -1) continue; // Column full

        // Copy the board
        State temp(*startState);
        temp.curPlayer = '2';   
        temp.applyMove(col); 
        temp.prep();

        if (temp.hasFourInARow('2')) {
            return col;
        }
    }
    return -1;
}

void Board::play() {
    while (true) {

        // --- Human Turn ---
        startState->curPlayer = '2';
        startState->printState();
        cout << "\nSelect a column to play (1-7) or 'q' to quit: ";
        char playerInput;
        cin >> playerInput;

        // Validate input
        while (true)
        {
            if (playerInput == 'q')
                return;
            int col = playerInput - '1';
            if (col >= 0 && col < COLS && startState->aMoves[col] != -1)
                break;
            cout << "Invalid input. Enter a column 1-7: ";
            cin >> playerInput;
        }
        int humanCol = playerInput - '1';
        startState->applyMove(humanCol);
        startState->prep();
        moveCount++;

        // Check if human just won
        if (startState->hasFourInARow('2'))
        {
            cout << "Human wins!" << endl;
            break;
        }

        // Check for tie (board full)
        if (moveCount >= (COLS * ROWS) - 1)
        {
            cout << "Board full, game ends in a tie!" << endl;
            break;
        }

        startState->prep();

        // --- AI Turn ---
        startState->curPlayer = '1';


        int winCol = -1;
        for (int col = 0; col < COLS; col++)
        {
            if (startState->aMoves[col] == -1)
                continue;
            State temp(*startState);
            temp.curPlayer = '1';
            temp.applyMove(col);
            temp.prep();
            if (temp.hasFourInARow('1'))
            {
                winCol = col;
                break;
            }
        }

        int blockCol = findBlockingMove();

        int aiCol = -1;
        if (winCol != -1)
        {
            aiCol = winCol;
            cout << "WINNING" << endl;
        } // Win if possible
        else if (blockCol != -1)
        {
            aiCol = blockCol;
            cout << "BLOCKING" << endl;
        } // Block human
        else
            aiCol = findBestMove(); // Else normal AI move

        if (aiCol != -1)
        {
            startState->applyMove(aiCol);
            startState->prep();
            cout << "AI played column " << aiCol + 1 << endl;
        }

        // Check wins
        if (startState->hasFourInARow('1'))
        {
            cout << "AI wins!" << endl;
            break;
        }
        else if (startState->hasFourInARow('2'))
        {
            cout << "Human wins!" << endl;
            break;
        }
        // Check for tie (board full)
        if (moveCount >= (COLS * ROWS) - 1)
        {
            cout << "Board full, game ends in a tie!" << endl;
            break;
        }
        moveCount++;

        startState->prep();
    }

    cout << "Game ended :3" << endl;
    startState->printState();
}

Board::~Board() {
    delete startState;
    startState = nullptr;
}
