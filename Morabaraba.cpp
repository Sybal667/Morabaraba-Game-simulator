#include "Morabaraba.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <set>
#include <iomanip>
#include <random>
#include <chrono>

using namespace std;

Morabaraba::Morabaraba(int cowsPerPlayer) {
    buildConnections();
    resetGame(cowsPerPlayer);
}

void Morabaraba::resetGame(int cowsPerPlayer) {
    for (int s = 0; s < 3; s++) {
        for (int i = 0; i < 8; i++) {
            board[s][i] = -1;
        }
    }
    
    cowsPlaced[0] = 0;
    cowsPlaced[1] = 0;
    cowsRemaining[0] = 0;
    cowsRemaining[1] = 0;
    cowsInHand[0] = cowsPerPlayer;
    cowsInHand[1] = cowsPerPlayer;
    currentPlayer = 0;
    totalCowsPerPlayer = cowsPerPlayer;
    gameOver = false;
    winner = -1;
    draw = false;
    
    moveCount = 0;
    threeCowMoveCount = 0;
    inThreeCowState = false;
    lastMillMoveCount = -1;
    lastMillPositions.clear();
    lastMillKey = "";
    
    p1LastBrokenMill = "";
    p2LastBrokenMill = "";
    
    moveLog.clear();
}

void Morabaraba::indexToPos(int index, int& square, int& idx) {
    square = index / 8;
    idx = index % 8;
}

void Morabaraba::buildConnections() {
    for (int i = 0; i < 24; i++) {
        neighbors[i].clear();
    }
    
    for (int square = 0; square < 3; square++) {
        for (int i = 0; i < 8; i++) {
            int current = posToIndex(square, i);
            int next = posToIndex(square, (i + 1) % 8);
            int prev = posToIndex(square, (i + 7) % 8);
            neighbors[current].push_back(next);
            neighbors[current].push_back(prev);
        }
    }
    
    for (int idx = 0; idx < 8; idx++) {
        for (int square = 0; square < 3; square++) {
            int current = posToIndex(square, idx);
            if (square > 0) {
                int inner = posToIndex(square - 1, idx);
                neighbors[current].push_back(inner);
            }
            if (square < 2) {
                int outer = posToIndex(square + 1, idx);
                neighbors[current].push_back(outer);
            }
        }
    }
    
    for (int i = 0; i < 24; i++) {
        sort(neighbors[i].begin(), neighbors[i].end());
        neighbors[i].erase(unique(neighbors[i].begin(), neighbors[i].end()), neighbors[i].end());
    }
}

bool Morabaraba::isMidpoint(int idx) {
    return (idx == 1 || idx == 3 || idx == 5 || idx == 7);
}

bool Morabaraba::isCorner(int idx) {
    return (idx == 0 || idx == 2 || idx == 4 || idx == 6);
}

vector<pair<int, int>> Morabaraba::getEmptySpots() {
    vector<pair<int, int>> emptySpots;
    for (int s = 0; s < 3; s++) {
        for (int i = 0; i < 8; i++) {
            if (board[s][i] == -1) {
                emptySpots.push_back({s, i});
            }
        }
    }
    return emptySpots;
}

int Morabaraba::numberOfCurrentCows(int player) {
    return cowsRemaining[player];
}

bool Morabaraba::attemptedMill(int square, int idx, int player) {
    if (board[square][idx] != -1) return false;
    
    int originalValue = board[square][idx];
    board[square][idx] = player;
    bool formsMill = checkMill(square, idx, player);
    board[square][idx] = originalValue;
    
    return formsMill;
}

bool Morabaraba::millFormed(int square, int idx, int player) {
    return checkMill(square, idx, player);
}

bool Morabaraba::legalMove(int fromS, int fromI, int toS, int toI, int player) {
    if (board[fromS][fromI] != player) return false;
    if (board[toS][toI] != -1) return false;
    
    int fromIdx = posToIndex(fromS, fromI);
    int toIdx = posToIndex(toS, toI);
    
    bool isAdjacent = false;
    for (int neighbor : neighbors[fromIdx]) {
        if (neighbor == toIdx) {
            isAdjacent = true;
            break;
        }
    }
    
    if (!isAdjacent) return false;
    
    if (wouldReformSameMill(fromS, fromI, toS, toI, player)) return false;
    
    return true;
}

void Morabaraba::positionCow(int s, int i, int player) {
    board[s][i] = player;
    cowsPlaced[player]++;
    cowsRemaining[player]++;
    cowsInHand[player]--;
}

void Morabaraba::placePiece(int s, int i, int player) {
    positionCow(s, i, player);
}

void Morabaraba::alternateTurn() {
    currentPlayer = 1 - currentPlayer;
}

void Morabaraba::printMoveToFile(const string& move) {
    moveLog.push_back(move);
}

bool Morabaraba::gameOverCondition() {
    return gameOver;
}

bool Morabaraba::wouldReformSameMill(int fromSquare, int fromIdx, int toSquare, int toIdx, int player) {
    string forbiddenKey = (player == 0) ? p1LastBrokenMill : p2LastBrokenMill;
    
    if (forbiddenKey.empty()) return false;
    
    int originalFromValue = board[fromSquare][fromIdx];
    int originalToValue = board[toSquare][toIdx];
    
    board[toSquare][toIdx] = player;
    board[fromSquare][fromIdx] = -1;
    
    bool formsMill = checkMill(toSquare, toIdx, player);
    string currentMillKey = "";
    
    if (formsMill) {
        vector<pair<int, int>> millPositions = getMillPositions(toSquare, toIdx, player);
        currentMillKey = millPositionsToString(millPositions);
    }
    
    board[toSquare][toIdx] = originalToValue;
    board[fromSquare][fromIdx] = originalFromValue;
    
    return (formsMill && currentMillKey == forbiddenKey);
}

vector<pair<int, int>> Morabaraba::getLegalMoves(int player) {
    vector<pair<int, int>> legalMoves;
    
    for (int s = 0; s < 3; s++) {
        for (int i = 0; i < 8; i++) {
            if (board[s][i] == player) {
                int currentIdx = posToIndex(s, i);
                for (int neighborIdx : neighbors[currentIdx]) {
                    int ns, ni;
                    indexToPos(neighborIdx, ns, ni);
                    if (board[ns][ni] == -1) {
                        if (!wouldReformSameMill(s, i, ns, ni, player)) {
                            legalMoves.push_back({neighborIdx, currentIdx});
                        }
                    }
                }
            }
        }
    }
    
    return legalMoves;
}

bool Morabaraba::hasLegalMoves(int player) {
    return !getLegalMoves(player).empty();
}

string Morabaraba::millPositionsToString(const vector<pair<int, int>>& positions) {
    stringstream ss;
    for (size_t i = 0; i < positions.size(); i++) {
        if (i > 0) ss << " , ";
        ss << "S" << positions[i].first << "i" << positions[i].second;
    }
    return ss.str();
}

string Morabaraba::formatMillString(const vector<pair<int, int>>& millPositions) {
    stringstream ss;
    ss << "(mill = ";
    for (size_t i = 0; i < millPositions.size(); i++) {
        if (i > 0) ss << " , ";
        ss << "S" << millPositions[i].first << "i" << millPositions[i].second;
    }
    ss << ")";
    return ss.str();
}

vector<pair<int, int>> Morabaraba::getMillPositions(int square, int idx, int player) {
    vector<pair<int, int>> mill;
    
    if ((idx == 0 || idx == 1 || idx == 2) && 
        board[square][0] == player && board[square][1] == player && board[square][2] == player) {
        mill = {{square, 0}, {square, 1}, {square, 2}};
    }
    else if ((idx == 2 || idx == 3 || idx == 4) && 
             board[square][2] == player && board[square][3] == player && board[square][4] == player) {
        mill = {{square, 2}, {square, 3}, {square, 4}};
    }
    else if ((idx == 4 || idx == 5 || idx == 6) && 
             board[square][4] == player && board[square][5] == player && board[square][6] == player) {
        mill = {{square, 4}, {square, 5}, {square, 6}};
    }
    else if ((idx == 6 || idx == 7 || idx == 0) && 
             board[square][6] == player && board[square][7] == player && board[square][0] == player) {
        mill = {{square, 6}, {square, 7}, {square, 0}};
    }
    else if ((idx == 0) && board[0][0] == player && board[1][0] == player && board[2][0] == player) {
        mill = {{0, 0}, {1, 0}, {2, 0}};
    }
    else if ((idx == 2) && board[0][2] == player && board[1][2] == player && board[2][2] == player) {
        mill = {{0, 2}, {1, 2}, {2, 2}};
    }
    else if ((idx == 4) && board[0][4] == player && board[1][4] == player && board[2][4] == player) {
        mill = {{0, 4}, {1, 4}, {2, 4}};
    }
    else if ((idx == 6) && board[0][6] == player && board[1][6] == player && board[2][6] == player) {
        mill = {{0, 6}, {1, 6}, {2, 6}};
    }
    else if ((idx == 1) && board[0][1] == player && board[1][1] == player && board[2][1] == player) {
        mill = {{0, 1}, {1, 1}, {2, 1}};
    }
    else if ((idx == 3) && board[0][3] == player && board[1][3] == player && board[2][3] == player) {
        mill = {{0, 3}, {1, 3}, {2, 3}};
    }
    else if ((idx == 5) && board[0][5] == player && board[1][5] == player && board[2][5] == player) {
        mill = {{0, 5}, {1, 5}, {2, 5}};
    }
    else if ((idx == 7) && board[0][7] == player && board[1][7] == player && board[2][7] == player) {
        mill = {{0, 7}, {1, 7}, {2, 7}};
    }
    
    return mill;
}

bool Morabaraba::checkMill(int square, int idx, int player) {
    return !getMillPositions(square, idx, player).empty();
}

bool Morabaraba::isPartOfMill(int square, int idx, int player) {
    if (idx == 0 || idx == 1 || idx == 2) {
        if (board[square][0] == player && board[square][1] == player && board[square][2] == player) return true;
    }
    if (idx == 2 || idx == 3 || idx == 4) {
        if (board[square][2] == player && board[square][3] == player && board[square][4] == player) return true;
    }
    if (idx == 4 || idx == 5 || idx == 6) {
        if (board[square][4] == player && board[square][5] == player && board[square][6] == player) return true;
    }
    if (idx == 6 || idx == 7 || idx == 0) {
        if (board[square][6] == player && board[square][7] == player && board[square][0] == player) return true;
    }
    if (idx == 0 && board[0][0] == player && board[1][0] == player && board[2][0] == player) return true;
    if (idx == 2 && board[0][2] == player && board[1][2] == player && board[2][2] == player) return true;
    if (idx == 4 && board[0][4] == player && board[1][4] == player && board[2][4] == player) return true;
    if (idx == 6 && board[0][6] == player && board[1][6] == player && board[2][6] == player) return true;
    if (idx == 1 && board[0][1] == player && board[1][1] == player && board[2][1] == player) return true;
    if (idx == 3 && board[0][3] == player && board[1][3] == player && board[2][3] == player) return true;
    if (idx == 5 && board[0][5] == player && board[1][5] == player && board[2][5] == player) return true;
    if (idx == 7 && board[0][7] == player && board[1][7] && board[2][7] == player) return true;
    
    return false;
}

vector<int> Morabaraba::getRemovableCows(int opponentPlayer) {
    vector<int> removable;
    vector<int> cowsInMills;
    
    for (int s = 0; s < 3; s++) {
        for (int i = 0; i < 8; i++) {
            if (board[s][i] == opponentPlayer) {
                if (isPartOfMill(s, i, opponentPlayer)) {
                    cowsInMills.push_back(posToIndex(s, i));
                } else {
                    removable.push_back(posToIndex(s, i));
                }
            }
        }
    }
    
    if (removable.empty() && !cowsInMills.empty()) {
        return cowsInMills;
    }
    
    return removable;
}

void Morabaraba::removeCow(int square, int idx) {
    if (board[square][idx] != -1) {
        int player = board[square][idx];
        board[square][idx] = -1;
        cowsRemaining[player]--;
        
        threeCowMoveCount = 0;
        inThreeCowState = (cowsRemaining[0] == 3 && cowsRemaining[1] == 3);
        if (inThreeCowState) {
            threeCowMoveCount = 0;
        }
    }
}

bool Morabaraba::checkWin(int playerWhoJustPlayed) {
    int opponent = 1 - playerWhoJustPlayed;
    int totalOpponentCows = cowsInHand[opponent] + cowsRemaining[opponent];
    
    if (totalOpponentCows <= 2) {
        winner = playerWhoJustPlayed;
        gameOver = true;
        return true;
    }

    if (cowsInHand[0] == 0 && cowsInHand[1] == 0) {
        if (!hasLegalMoves(opponent)) {
            winner = playerWhoJustPlayed;
            gameOver = true;
            return true;
        }
    }
    
    return false;
}

void Morabaraba::updateThreeCowCounter() {
    bool bothHaveThree = (cowsRemaining[0] == 3 && cowsRemaining[1] == 3);
    
    if (bothHaveThree && !inThreeCowState) {
        inThreeCowState = true;
        threeCowMoveCount = 0;
    } else if (!bothHaveThree) {
        inThreeCowState = false;
        threeCowMoveCount = 0;
    }
    
    if (inThreeCowState) {
        threeCowMoveCount++;
        if (threeCowMoveCount >= 10) {
            gameOver = true;
            draw = true;
        }
    }
}

int Morabaraba::evaluateBoard(int player) {
    int score = 0;
    int opponent = 1 - player;
    
    score += cowsRemaining[player] * 10;
    score -= cowsRemaining[opponent] * 8;
    
    int potentialMills = 0;
    for (int s = 0; s < 3; s++) {
        for (int i = 0; i < 8; i++) {
            if (board[s][i] == player) {
                if (i == 0 || i == 1 || i == 2) {
                    int count = 0;
                    if (board[s][0] == player) count++;
                    if (board[s][1] == player) count++;
                    if (board[s][2] == player) count++;
                    if (count == 2) potentialMills++;
                }
                if (i == 2 || i == 3 || i == 4) {
                    int count = 0;
                    if (board[s][2] == player) count++;
                    if (board[s][3] == player) count++;
                    if (board[s][4] == player) count++;
                    if (count == 2) potentialMills++;
                }
                if (i == 4 || i == 5 || i == 6) {
                    int count = 0;
                    if (board[s][4] == player) count++;
                    if (board[s][5] == player) count++;
                    if (board[s][6] == player) count++;
                    if (count == 2) potentialMills++;
                }
                if (i == 6 || i == 7 || i == 0) {
                    int count = 0;
                    if (board[s][6] == player) count++;
                    if (board[s][7] == player) count++;
                    if (board[s][0] == player) count++;
                    if (count == 2) potentialMills++;
                }
            }
        }
    }
    score += potentialMills * 15;
    
    for (int i = 0; i < 8; i++) {
        if (board[1][i] == player) {
            score += 5;
        }
    }
    
    int opponentPotential = 0;
    for (int s = 0; s < 3; s++) {
        for (int i = 0; i < 8; i++) {
            if (board[s][i] == opponent) {
                if (i == 0 || i == 1 || i == 2) {
                    int count = 0;
                    if (board[s][0] == opponent) count++;
                    if (board[s][1] == opponent) count++;
                    if (board[s][2] == opponent) count++;
                    if (count == 2) opponentPotential++;
                }
                if (i == 2 || i == 3 || i == 4) {
                    int count = 0;
                    if (board[s][2] == opponent) count++;
                    if (board[s][3] == opponent) count++;
                    if (board[s][4] == opponent) count++;
                    if (count == 2) opponentPotential++;
                }
                if (i == 4 || i == 5 || i == 6) {
                    int count = 0;
                    if (board[s][4] == opponent) count++;
                    if (board[s][5] == opponent) count++;
                    if (board[s][6] == opponent) count++;
                    if (count == 2) opponentPotential++;
                }
                if (i == 6 || i == 7 || i == 0) {
                    int count = 0;
                    if (board[s][6] == opponent) count++;
                    if (board[s][7] == opponent) count++;
                    if (board[s][0] == opponent) count++;
                    if (count == 2) opponentPotential++;
                }
            }
        }
    }
    score -= opponentPotential * 20;
    
    return score;
}

bool Morabaraba::alg1Place(int turnNumber) {
    if (cowsInHand[0] <= 0) return false;
    
    vector<pair<int, int>> emptySpots = getEmptySpots();
    if (emptySpots.empty()) return false;
    
    static mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_int_distribution<size_t> dist(0, emptySpots.size() - 1);
    size_t idx = dist(rng);
    
    int bestS = emptySpots[idx].first;
    int bestI = emptySpots[idx].second;
    
    positionCow(bestS, bestI, 0);
    
    stringstream ss;
    ss << "Alg_1 00-S" << bestS << ", i" << bestI;
    
    if (millFormed(bestS, bestI, 0)) {
        vector<pair<int, int>> millPositions = getMillPositions(bestS, bestI, 0);
        vector<int> removable = getRemovableCows(1);
        if (!removable.empty()) {
            int removeIdx = removable[0];
            int remSquare, remIdx;
            indexToPos(removeIdx, remSquare, remIdx);
            removeCow(remSquare, remIdx);
            int remaining = cowsInHand[1] + cowsRemaining[1];
            ss << " " << formatMillString(millPositions) << " Alg_2 losses cow (S" << remSquare << ", i" << remIdx << ") | " << remaining << " left";
            checkWin(0);
        }
    }
    moveLog.push_back(ss.str());
    return true;
}

bool Morabaraba::alg2Place(int turnNumber) {
    if (cowsInHand[1] <= 0) return false;

    vector<pair<int, int>> spots = getEmptySpots();
    int bestS = -1, bestI = -1;
    int maxScore = -99999;

    for (auto& spot : spots) {
        board[spot.first][spot.second] = 1;
        int score = evaluateBoard(1); 
        if (checkMill(spot.first, spot.second, 1)) score += 500;
        if (attemptedMill(spot.first, spot.second, 0)) score += 200;

        if (score > maxScore) {
            maxScore = score;
            bestS = spot.first;
            bestI = spot.second;
        }
        board[spot.first][spot.second] = -1;
    }

    positionCow(bestS, bestI, 1);
    
    stringstream ss;
    ss << "Alg_2 00-S" << bestS << ", i" << bestI;
    
    if (checkMill(bestS, bestI, 1)) {
        vector<pair<int, int>> millPositions = getMillPositions(bestS, bestI, 1);
        vector<int> removable = getRemovableCows(0);
        if (!removable.empty()) {
            int rs, ri;
            indexToPos(removable[0], rs, ri);
            removeCow(rs, ri);
            int remaining = cowsInHand[0] + cowsRemaining[0];
            ss << " " << formatMillString(millPositions) << " Alg_1 losses cow (S" << rs << ", i" << ri << ") | " << remaining << " left";
            checkWin(1);
        }
    }
    moveLog.push_back(ss.str());
    return true;
}

bool Morabaraba::alg1Move() {
    vector<pair<int, int>> moves = getLegalMoves(0);
    if (moves.empty()) return false;
    
    static mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_int_distribution<size_t> dist(0, moves.size() - 1);
    size_t moveIdx = dist(rng);
    
    int bestToS, bestToI, bestFromS, bestFromI;
    indexToPos(moves[moveIdx].first, bestToS, bestToI);
    indexToPos(moves[moveIdx].second, bestFromS, bestFromI);
    
    string brokenKey = "";
    if (isPartOfMill(bestFromS, bestFromI, 0)) {
        brokenKey = millPositionsToString(getMillPositions(bestFromS, bestFromI, 0));
    }
    
    board[bestToS][bestToI] = 0;
    board[bestFromS][bestFromI] = -1;
    p1LastBrokenMill = brokenKey;
    
    stringstream ss;
    ss << "Alg_1 S" << bestFromS << ", i" << bestFromI << "-S" << bestToS << ", i" << bestToI;
    
    if (millFormed(bestToS, bestToI, 0)) {
        vector<pair<int, int>> millPositions = getMillPositions(bestToS, bestToI, 0);
        vector<int> removable = getRemovableCows(1);
        if (!removable.empty()) {
            int remSquare, remIdx;
            indexToPos(removable[0], remSquare, remIdx);
            removeCow(remSquare, remIdx);
            ss << " " << formatMillString(millPositions) << " Alg_2 losses cow (S" << remSquare << ", i" << remIdx << ") | " << cowsRemaining[1] << " left";
            checkWin(0);
        }
    }
    
    moveCount++;
    moveLog.push_back(ss.str());
    updateThreeCowCounter();
    return true;
}

bool Morabaraba::alg2Move() {
    vector<pair<int, int>> moves = getLegalMoves(1);
    if (moves.empty()) return false;

    int bestToS = -1, bestToI = -1, bestFromS = -1, bestFromI = -1;
    int maxScore = -99999;
    bool moveFound = false;

    for (auto& move : moves) {
        int ts, ti, fs, fi;
        indexToPos(move.first, ts, ti);
        indexToPos(move.second, fs, fi);

        board[ts][ti] = 1;
        board[fs][fi] = -1;
        int score = evaluateBoard(1);
        if (checkMill(ts, ti, 1)) score += 1000;
        score += (getLegalMoves(1).size() * 10);

        if (score > maxScore) {
            maxScore = score;
            bestToS = ts; bestToI = ti;
            bestFromS = fs; bestFromI = fi;
            moveFound = true;
        }
        board[ts][ti] = -1;
        board[fs][fi] = 1;
    }

    if (!moveFound) return false;

    string brokenKey = "";
    if (isPartOfMill(bestFromS, bestFromI, 1)) {
        brokenKey = millPositionsToString(getMillPositions(bestFromS, bestFromI, 1));
    }

    board[bestToS][bestToI] = 1;
    board[bestFromS][bestFromI] = -1;
    p2LastBrokenMill = brokenKey;

    stringstream ss;
    ss << "Alg_2 S" << bestFromS << ", i" << bestFromI << "-S" << bestToS << ", i" << bestToI;
    
    if (checkMill(bestToS, bestToI, 1)) {
        vector<pair<int, int>> millPositions = getMillPositions(bestToS, bestToI, 1);
        vector<int> removable = getRemovableCows(0);
        if (!removable.empty()) {
            int rs, ri;
            indexToPos(removable[0], rs, ri);
            removeCow(rs, ri);
            ss << " " << formatMillString(millPositions) << " Alg_1 losses cow (S" << rs << ", i" << ri << ") | " << cowsRemaining[0] << " left";
            checkWin(1);
        }
    }

    moveCount++;
    moveLog.push_back(ss.str());
    updateThreeCowCounter();
    return true;
}

bool Morabaraba::playGame() {
    playPlacementPhase();
    if (!gameOver) {
        playMovementPhase();
    }
    return gameOver;
}

bool Morabaraba::playPlacementPhase() {
    int totalTurns = totalCowsPerPlayer * 2;
    int turnsPlayed = 0;
    while (!gameOver && turnsPlayed < totalTurns) {
        if (currentPlayer == 0) alg1Place(turnsPlayed);
        else alg2Place(turnsPlayed);
        turnsPlayed++;
        alternateTurn();
    }
    return !gameOver;
}

bool Morabaraba::playMovementPhase() {
    while (!gameOver) {
        if (!hasLegalMoves(currentPlayer)) {
            gameOver = true;
            winner = 1 - currentPlayer;
            break;
        }
        
        // Check for 50-move draw rule
        if (moveCount >= 20) {
            gameOver = true;
            draw = true;
            break;
        }
        
        if (currentPlayer == 0) alg1Move();
        else alg2Move();
        alternateTurn();
    }
    return true;
}

string Morabaraba::getGameResult() {
    stringstream ss;
    for (const string& log : moveLog) {
        ss << log << "\n";
    }
    if (winner == 0) ss << "\nAlg_1 wins\n\n";
    else if (winner == 1) ss << "\nAlg_2 wins\n\n";
    else if (draw) ss << "\nits a draw\n\n";
    return ss.str();
}