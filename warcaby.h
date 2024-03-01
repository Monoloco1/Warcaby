#include <iostream>
#include <vector>
#include <math.h>

using std::vector, std::cout, std::min, std::max, std::cin;
using CV = vector<char>;
using CM = vector<CV>;

void clearScreen();

class Warcaby {
private:
    const int boardSize = 8;
    CM board;

public:
    void initializeBoard();
    void printBoard();
    bool isValidMove(int fromRow, int fromCol, int toRow, int toCol, char player);
    void makeMove(int fromRow, int fromCol, int toRow, int toCol, char player);

    bool validateMakeMove(int fromRow, char fromCol, int toRow, char toCol, char player) ;
    bool isGameOver();

    Warcaby();
};