#ifndef MORABARABA_H
#define MORABARABA_H

#include <vector>
#include <string>
#include <utility>
#include <set>
#include <cstdlib>
#include <ctime>

class Morabaraba {
private:
    int board[3][8];
    int cowsPlaced[2];
    int cowsRemaining[2];
    int cowsInHand[2];
    int currentPlayer;
    int totalCowsPerPlayer;
    bool gameOver;
    int winner;
    bool draw;
    
    int moveCount;
    int threeCowMoveCount;
    bool inThreeCowState;
    int lastMillMoveCount;
    std::set<std::string> lastMillPositions;
    std::string lastMillKey;
    
    std::string p1LastBrokenMill;
    std::string p2LastBrokenMill;
    
    std::vector<int> neighbors[24];
    
    int posToIndex(int square, int idx) { return square * 8 + idx; }
    void indexToPos(int index, int& square, int& idx);
    void buildConnections();
    
    bool checkMill(int square, int idx, int player);
    bool isPartOfMill(int square, int idx, int player);
    std::vector<std::pair<int, int>> getMillPositions(int square, int idx, int player);
    std::vector<int> getRemovableCows(int opponentPlayer);
    void removeCow(int square, int idx);
    bool checkWin(int playerWhoJustPlayed);
    bool hasLegalMoves(int player);
    std::vector<std::pair<int, int>> getLegalMoves(int player);
    bool wouldReformSameMill(int fromSquare, int fromIdx, int toSquare, int toIdx, int player);
    std::vector<std::pair<int, int>> getEmptySpots();
    bool isMidpoint(int idx);
    bool isCorner(int idx);
    void updateThreeCowCounter();
    std::string millPositionsToString(const std::vector<std::pair<int, int>>& positions);
    std::string formatMillString(const std::vector<std::pair<int, int>>& millPositions);
    
    int evaluateBoard(int player);  
    
    bool alg1Place(int turnNumber);
    bool alg2Place(int turnNumber);
    bool alg1Move();
    bool alg2Move();
    
    int numberOfCurrentCows(int player);
    bool attemptedMill(int square, int idx, int player);
    bool strategicMove(int& fromS, int& fromI, int& toS, int& toI);
    bool blockMill(int& fromS, int& fromI, int& toS, int& toI);
    bool millFormed(int square, int idx, int player);
    bool legalMove(int fromS, int fromI, int toS, int toI, int player);
    void positionCow(int s, int i, int player);
    void alternateTurn();
    void printMoveToFile(const std::string& move);
    bool playGame();
    bool gameOverCondition();
    void placePiece(int s, int i, int player);  
    
public:
    Morabaraba(int cowsPerPlayer);
    void resetGame(int cowsPerPlayer);
    bool playPlacementPhase();
    bool playMovementPhase();
    std::string getGameResult();
    
    std::vector<std::string> moveLog;
};

#endif