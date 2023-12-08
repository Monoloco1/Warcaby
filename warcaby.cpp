#include <iostream>

//#ifdef _WIN32
//
////#include <winsock.h>
//
//#include        <sys/types.h>
//
//#elif defined(unix) || defined(__unix__) || defined(__unix)
//
////#include        <sys/socket.h>  /* basic socket definitions */
////#include        <sys/time.h>    /* timeval{} for select() */
//#include        <time.h>                /* timespec{} for pselect() */
////#include        <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
////#include        <arpa/inet.h>   /* inet(3) functions */
//#include        <errno.h>
//#include        <fcntl.h>               /* for nonblocking */
////#include        <netdb.h>
//#include        <signal.h>
//#include        <stdio.h>
//#include        <stdlib.h>
//#include        <string.h>
////#include 	<netdb.h>
////#include 	<resolv.h>
////#include 	<unistd.h>
////#include 	<syslog.h>
//
//#endif

#include <iostream>
#include <vector>

void clearScreen() {
#ifdef _WIN32

    system("cls");

#include        <sys/types.h>

#elif defined(unix) || defined(__unix__) || defined(__unix)

    system("clear");

#endif
}

class CheckersGame {
private:
    const int boardSize = 8;
    std::vector<std::vector<char>> board;

public:
    CheckersGame() {
        initializeBoard();
    }
    void initializeBoard() {
        board.resize(boardSize, std::vector<char>(boardSize, ' '));

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

    /*void initializeBoard() {
        board.resize(boardSize, std::vector<char>(boardSize, ' '));

        for (int i = 0; i < boardSize; i += 2) {
            board[0][i + 1] = 'B';
            board[boardSize - 2][i] = 'W';
            board[boardSize - 1][i + 1] = 'W';
        }

        for (int i = 1; i < boardSize; i += 2) {
            board[1][i] = 'B';
            board[boardSize - 3][i + 1] = 'W';
        }
    }*/

    void printBoard() {
        std::cout << "  0 1 2 3 4 5 6 7" << std::endl;
        for (int i = 0; i < boardSize; ++i) {
            std::cout << i << " ";
            for (int j = 0; j < boardSize; ++j) {
                std::cout << board[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    bool isValidMove(int fromRow, int fromCol, int toRow, int toCol, char player) {
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

    void makeMove(int fromRow, int fromCol, int toRow, int toCol, char player) {
        board[toRow][toCol] = board[fromRow][fromCol];
        board[fromRow][fromCol] = ' ';

        // Remove captured piece
        if (std::abs(toRow - fromRow) == 2) {
            int capturedRow = (fromRow + toRow) / 2;
            int capturedCol = (fromCol + toCol) / 2;
            board[capturedRow][capturedCol] = ' ';
        }
    }


    // check how many pieces each player has
    bool isGameOver() {
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
};

int main() {
    CheckersGame game;
    char currentPlayer = 'B';

    while (!game.isGameOver()) {
        clearScreen(); // Clear the terminal

        game.printBoard();

        int fromRow, fromCol, toRow, toCol;

        std::cout << "Player " << currentPlayer << "'s turn." << std::endl;
        std::cout << "Enter move (fromRow fromCol toRow toCol): ";
        std::cin >> fromRow >> fromCol >> toRow >> toCol;

        if (game.isValidMove(fromRow, fromCol, toRow, toCol, currentPlayer)) {
            game.makeMove(fromRow, fromCol, toRow, toCol, currentPlayer);
            currentPlayer = (currentPlayer == 'B') ? 'W' : 'B';
        }
        else {
            std::cout << "Invalid move. Try again." << std::endl;
        }
    }

    clearScreen();
    game.printBoard();
    std::cout << "Player " << currentPlayer << " wins!" << std::endl;

    return 0;
}