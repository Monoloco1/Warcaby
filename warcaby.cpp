#include "warcaby.h"

void clearScreen() {
#ifdef _WIN32

    system("cls");

#include        <sys/types.h>

#elif defined(unix) || defined(__unix__) || defined(__unix)

    system("clear");

#endif
}

Warcaby::Warcaby() {
	initializeBoard();
}


void Warcaby::initializeBoard() {
    board.resize(boardSize, CV(boardSize, ' '));

    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            if ((i % 2 == 0 && j % 2 == 1) || (i % 2 == 1 && j % 2 == 0)) {
                if (i < 3) {
                    board[i][j] = 'B'; // Black pieces
                }
                else if (i >= boardSize - 3) {
                    board[i][j] = 'W'; // White pieces
                }
            }
        }
    }
}

void Warcaby::printBoard() {
    //std::cout << "  0 1 2 3 4 5 6 7" << std::endl;
    std::cout << "  A B C D E F G H" << std::endl;
    for (int i = 0; i < boardSize; ++i) {
        std::cout << i << " ";
        for (int j = 0; j < boardSize; ++j) {
            std::cout << board[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

bool Warcaby::isValidMove(int fromRow, int fromCol, int toRow, int toCol, char player) {
    //check board bounds
    if (fromRow < 0 || fromRow >= boardSize || fromCol < 0 || fromCol >= boardSize ||
        toRow < 0 || toRow >= boardSize || toCol < 0 || toCol >= boardSize) {
        return false;
    }

    //check if piece is the player's
    if (board[fromRow][fromCol] == ' ' || board[fromRow][fromCol] != player) {
        return false;
    }

    //check if toPosition is empty
    if (board[toRow][toCol] != ' ') {
        return false;
    }

    if (player == 'B') {
        if (toRow - fromRow == 1 && std::abs(toCol - fromCol) == 1) {
            return true; // Regular move
        }
        else if (toRow - fromRow == 2 && std::abs(toCol - fromCol) == 2 &&
            board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == 'W') {
            return true; // Capture move
        }
    }
    else if (player == 'W') {
        if (fromRow - toRow == 1 && std::abs(toCol - fromCol) == 1) {
            return true; // Regular move
        }
        else if (fromRow - toRow == 2 && std::abs(toCol - fromCol) == 2 &&
            board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == 'B') {
            return true; // Capture move
        }
    }

    return false; // Invalid move
}

void Warcaby::makeMove(int fromRow, int fromCol, int toRow, int toCol, char player) {
    board[toRow][toCol] = board[fromRow][fromCol];
    board[fromRow][fromCol] = ' ';

    // Remove captured piece
    if (std::abs(toRow - fromRow) == 2) {
        int capturedRow = (fromRow + toRow) / 2;
        int capturedCol = (fromCol + toCol) / 2;
        board[capturedRow][capturedCol] = ' ';
    }
}

bool Warcaby::validateMakeMove(int fromRow, char fromCol, int toRow, char toCol, char player)  {
    fromCol = std::toupper(fromCol);
    toCol = std::toupper(toCol);
    cout << fromRow << ' ' << fromCol << ' ' << toRow << ' ' << toCol << std::endl;
    if(fromCol > 'H' || fromCol < 'A') return 0;
    if(toCol > 'H' || toCol < 'A') return 0;

    int fromColNum = min(7, max(0, fromCol - 'A'));
    int toColNum = min(7, max(0, toCol - 'A'));
     cout << fromColNum << ' ' << toColNum << std::endl;
    if(isValidMove(fromRow, fromColNum, toRow, toColNum, player)) {
        makeMove(fromRow, fromColNum, toRow, toColNum, player);
        return true;
    }
    else return 0;
}

// check how many pieces each player has
bool Warcaby::isGameOver() {
    int blackCount = 0;
    int whiteCount = 0;

    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            if (board[i][j] == 'B') {
                blackCount++;
            }
            else if (board[i][j] == 'W') {
                whiteCount++;
            }
        }
    }

    return (blackCount == 0 || whiteCount == 0);
}

