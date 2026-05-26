#include "Morabaraba.h"
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
using namespace std;

int main() {
    ifstream inputFile("input.txt");
    ofstream outputFile("morabarabaResults.txt");

    if (!inputFile.is_open()) {
        return 1;
    }

    vector<int> games;
    int cows;
    while (inputFile >> cows) {
        games.push_back(cows);
    }
    inputFile.close();

    for (int cowsPerPlayer : games) {
        outputFile << cowsPerPlayer << "\n";

        Morabaraba game(cowsPerPlayer);

        game.playPlacementPhase();

        string result = game.getGameResult();

        if (result.find("wins") == string::npos && result.find("draw") == string::npos) {
            game.playMovementPhase();
            result = game.getGameResult();
        }

        outputFile << result;
    }

    outputFile.close();
    cout << "\033[1;32m";
    cout << "** Compiling Done and Successfully **" << endl;
    cout << "\033[0m";

    return 0;
}
