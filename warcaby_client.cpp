#include <iostream>
#include <vector>
#include "warcaby.h"

int main() {
    Warcaby game;
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