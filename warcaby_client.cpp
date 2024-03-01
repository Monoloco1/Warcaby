#include <iostream>
#include <vector>
#include "warcaby.h"
#include "const.h"


int main(int argc, char **argv) {
	if(argc < 2) {
		cout << "usage: interface\n";
		exit(0);
	}

    Warcaby game;
    char currentPlayer = 'X'; // jeszcze nie wiadomo jakim graczem jest klient
	bool foundServer = false;


	//	Konfiguracja multicast
	int sendfd, recvfd;
	const int on = 1;
	socklen_t salen;
	struct sockaddr_in6 *ipv6addr;
	struct sockaddr_in *ipv4addr;

	// Ustawienia multicast
	string multicastInterface = argv[1];
	string multicastAddress{DEF_MADDR};
	int multicastPort{DEF_MPORT};
	
	sockaddr *udpSA{};
	
	//Tworzenie gniazda odbierającego komunikaty multicast
	int udpFD{};
	snd_udp_socket(multicastAddress.c_str(), multicastPort, &udpSA, &salen);
	udpSA->sa_family = AF_INET;
	if ( (udpFD = socket(udpSA->sa_family, SOCK_DGRAM, 0)); udpFD < 0){
		fprintf(stderr,"socket error : %s\n", strerror(errno));
		return 1;
	}

	if (setsockopt(udpFD, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
		fprintf(stderr,"setsockopt error : %s\n", strerror(errno));
		return 1;
	}
	
	if( bind(udpFD, udpSA, salen) < 0 ) {
	    fprintf(stderr,"bind error : %s\n", strerror(errno));
	    return 1;
	}

	// Dołączenie do grupy multicast
	if( mcast_join(udpFD, udpSA, salen, multicastInterface.c_str(), 0) < 0 ) {
		fprintf(stderr,"mcast_join() error : %s\n", strerror(errno));
		return 1;
	}

	char		line[MAXLINE];
	struct utsname	myname;

	if (uname(&myname) < 0)
		perror("uname error");
	snprintf(line, sizeof(line), "%s, PID=%d", myname.nodename, getpid());

	printf("listening multicast:\n");
	int					n;
	struct sockaddr		*safrom;
	char str[128];
	struct sockaddr_in6*	 cliaddr;
	struct sockaddr_in*	 cliaddrv4;
	char			addr_str[INET6_ADDRSTRLEN+1];
	int	sockFdTcp;

	safrom = (sockaddr*)malloc(salen);
	socklen_t len = salen;
	string udpLine;
	string tcpLine;
	const string helloPrefix = "WARCABY:";
	const size_t helloPrefix_s = helloPrefix.length()-1;
	int tcpPort{};

// Ustawienie zmiennych gry
	Warcaby warcaby;
	bool gameStarted{};
	bool assignedWhite{};
	char player{};
	char peer{};
	bool myTurn{};
	bool drawBoard{};

// Główna pętla
	while(1) {
		// Odnajdowanie serwera Warcaby z wiadomości multicast
		if (foundServer == 0) {
			sockaddr_in	addrServTcp;
			if ( (sockFdTcp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				fprintf(stderr,"socket error : %s\n", strerror(errno));
				continue;
				//return 1;
			}
			udpLine = string(MAXLINE, 0);
			if( (n = recvfrom(udpFD, udpLine.data(), MAXLINE, 0, udpSA, &len)) < 0 ) perror("recvfrom() error");//recvfrom(recvfd, line, MAXLINE, 0, safrom, &len))
			
			if( udpSA->sa_family == AF_INET6 ) { //safrom->sa_family == AF_INET6
				cliaddr = (struct sockaddr_in6*) udpSA; //cliaddr = (struct sockaddr_in6*) safrom;
				inet_ntop(AF_INET6, (struct sockaddr  *) &cliaddr->sin6_addr,  addr_str, sizeof(addr_str));
				
			}
			else {
				cliaddrv4 = (struct sockaddr_in*) udpSA;
				inet_ntop(AF_INET, (struct sockaddr  *) &cliaddrv4->sin_addr,  addr_str, sizeof(addr_str));
				addrServTcp = * ( (sockaddr_in*)udpSA );
			}
			
			string udpLineCut(n, 0);
			std::copy(udpLine.cbegin(), udpLine.cbegin() + n, udpLineCut.begin());
			//debug multicast
			cout << udpLineCut << ' ' << addr_str << '\n';
			
			addrServTcp.sin_port = htons(atoi(udpLineCut.c_str()));
			addrServTcp.sin_family = AF_INET;////
			string addrServTcp_str(47, 0);
			inet_ntop(AF_INET, (sockaddr*)&addrServTcp.sin_addr, addrServTcp_str.data(), sizeof(addrServTcp_str));
			cout << "connecting: " << addrServTcp_str << ":" << std::to_string(ntohs(addrServTcp.sin_port)) << '\n';
			
			if( connect(sockFdTcp, (SA *) &addrServTcp, sizeof( *( (SA *) &addrServTcp ) )) < 0) {
				fprintf(stderr,"connect: %s\n", strerror(errno));
			}
			else {
				cout << "connected\n";
				foundServer = true;
				//close(sockFdTcp);
				//cout << "closed connection\n";
			}

			// TODO: dodać do wiadomości multicast od serwera prefiksu albo informacji
			// o tym że wiadomość pochodzi od serwera Warcaby

			// if(helloPrefix.compare(0, helloPrefix_s, udpLineCut, helloPrefix_s) == 0) {
			// 	// message is Warcaby hello message
			// 	string address;
			// 	string port;
			// 	bool isPort{0};
			// 	for(int j{}; j<udpLineCut.length()-helloPrefix_s; ++j) {
			// 		if(isPort) {
			// 			port += udpLineCut[helloPrefix_s+j];
			// 		}
			// 		else if( udpLineCut[helloPrefix_s+j] != ':' ) {
			// 			address += udpLineCut[helloPrefix_s+j];
			// 		} else isPort = 1;
			// 	}
			// 	if(inet_pton(AF_INET, address.data(), &tcpAddress_in.sin_addr) <= 0) {
			// 		cout << "bad hello message\n";
			// 	}
			// 	else {
			// 		cout << "good hello message\n";
			// 	}
			// }
			// else cout << "bad hello prefix\n";
		}

		// Serwer został odnaleziony
		else {
			tcpLine = string(MAXLINE, 0);
			// Odczyt komunikatów od serwera
			if(n = read(sockFdTcp, tcpLine.data(), MAXLINE); n < 0) {
				// connection reset
				if(errno == ECONNRESET) {
					printf("server aborted connection\n");
					close(sockFdTcp);
					exit(0);
				}
				// read error
				else {
					perror("read error");
					if(write(sockFdTcp, READ_ERROR_MESSAGE, sizeof(READ_ERROR_MESSAGE)) < 0) {
						perror("send error");
						close(sockFdTcp);
						exit(0);
					}
				}
			}
			// connection closed
			else if (n == 0) {
				printf("server closed connection\n");
				exit(0);
			}
			else {
				string tcpLineCut(n, 0);
				std::copy(tcpLine.cbegin(), tcpLine.cbegin() + n, tcpLineCut.begin());
				cout << tcpLineCut << '\n';
				
				// Cięcie komunikatu na słowa czteroznakowe. Znaczenie słów znajduję się w pliku const.h
				if(tcpLineCut.length() < 4) {
					cout << "err";
				}
				string tcpLineCutPrefix(4, 0);
				// Odszyfrowanie jakie komendy są wskazane przez słowa czteroznakowe
				for(int j{}; j<n; j+=4) {
					tcpLineCutPrefix = tcpLineCut.substr(j, j+4);
					if(tcpLineCutPrefix.length() < 4) break;
					cout << "deciphering: " << tcpLineCutPrefix << "...\n";
					int num{};
					for(int s{}; s<messNumber.size(); ++s) {
						//cout << "comparing: " << tcpLineCutPrefix << " and " << messNumber.at(s).substr(0, 4) << '\n';
						if( tcpLineCutPrefix.compare(messNumber.at(s).substr(0, 4)) == 0) {
							num = s+1;
							break;
						}
					}
					switch( num ) {
						case 1:	//PEER_DISCONNECT_MESSAGE
							cout << "Peer disconnected, exiting...\n";
							close(sockFdTcp);
							exit(0);
							break;
						case 2:	//CONNECTION_ACKNOWLEDGE_MESSAGE
							// Potwierdzenie połączenie
							cout << "ack conn\n";
							break;
						case 3:	//READ_ERROR_MESSAGE
							cout << "read err\n";
							break;
						case 4:	//ASSIGNED_WHITE_MESSAGE
							assignedWhite = true;
							myTurn = true;
							player = 'W';
							peer = 'B';
							gameStarted = true;
							warcaby.initializeBoard();
							cout << "assigned White\n";
							break;
						case 5:	//ASSIGNED_BLACK_MESSAGE
							assignedWhite = false;
							myTurn = false;
							player = 'B';
							peer = 'W';
							gameStarted = true;
							warcaby.initializeBoard();
							drawBoard = true;
							cout << "assigned Black\n";
							break;
						case 6: //SEND_MOVE
							// Wykonanie ruchu przez wroga
							// ta komenda została przekazana od wroga przez serwer
							cout << "received move message: "<< tcpLineCut.substr(j, j+8) << std::endl;
							j+=4;
							{
								string moveParams = tcpLineCut.substr(j, j+4);
								if(moveParams.length() < 4) {
									cout << "move params too short\n";
									break;
								}
								else j+= 4;
								int fromRow{}, toRow{};
								char fromCol{}, toCol{};
								fromRow = moveParams.at(0) - 48;
								fromCol = moveParams.at(1);
								toRow = moveParams.at(2) - 48;
								toCol = moveParams.at(3);
								cout << moveParams << '\n';
								cout << fromRow << ' ' << fromCol << ' ' << toRow << ' ' << toCol << std::endl;
								if( !warcaby.validateMakeMove(fromRow, fromCol, toRow, toCol, peer) ) {
									cout << "bad move\n";
								}
								else {
									myTurn = true;
									drawBoard = false;
								}
							}
							break;
						case 7: //MOVE_OK
							cout << "move ok\n";
							if(myTurn) {
								myTurn = false;
								drawBoard = true;
							}
							break;
						default:
							cout << "error decpihering\n";
							break;
					}
				}

				// Gdy to nie jest kolej gracza, narysuj planszę gry
				if(drawBoard) {
					if(myTurn) cout << "error in board draw logic\n";
					clearScreen();
					warcaby.printBoard();
					cout << "Czekam na ruch przeciwnika...\n";
					drawBoard = false;
				}
				// Wykonanie ruchu przez gracza i wysłanie go do serwera
				if(myTurn) {
					bool isMoveValid{true};
					int fromRow{}, toRow{};
					char fromCol{}, toCol{};
					do {
						clearScreen();
						warcaby.printBoard();
						cout << "Twoja kolej, ruszasz: \"" << player << "\"\n" << "Wpisz swoj ruch (wierszZ kolumnaZ wierszDO kolumnaDO): ";
						if(isMoveValid) cout << '\n';
						else cout << "Zly ruch...\n";
						string input;
						do {
							input = string(0, 0);
							cin >> input;
							cin.clear();
							cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
							cout << input << "\n";
						} while(input.length() != 4);
						fromRow = input.at(0) - 48;
						fromCol = input.at(1);
						toRow = input.at(2) - 48;
						toCol = input.at(3);
						cout << fromRow << ' ' << fromCol << ' ' << toRow << ' ' << toCol << std::endl;
						isMoveValid = warcaby.validateMakeMove(fromRow, fromCol, toRow, toCol, player);
					} while(!isMoveValid);
					string move = SEND_MOVE + std::to_string(fromRow) + fromCol + std::to_string(toRow) + toCol;
					cout << "wysylanie ruchu do serwera...\n";
					if(write(sockFdTcp, move.data(), move.length()) < 0) {
						cout << "error sending move\n";
					}
				}
			}
		}
	}

    return 0;
}