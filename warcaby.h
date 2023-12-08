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

void clearScreen();

class Warcaby {
private:
    const int boardSize = 8;
    std::vector<std::vector<char>> board;

public:
    Warcaby();
    void initializeBoard();
    void printBoard();
    bool isValidMove(int fromRow, int fromCol, int toRow, int toCol, char player);
    void makeMove(int fromRow, int fromCol, int toRow, int toCol, char player);
    bool isGameOver();
};