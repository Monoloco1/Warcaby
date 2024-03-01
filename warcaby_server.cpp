#include <iostream>
#include <vector>
#include "warcaby.h"
#include "const.h"
#include <syslog.h>

#define MAXLINE 1024
#define LISTENQ 2
#define	SENDRATE	5


int daemon_init();
int slog(string log, int facility);

int main(int argc, char **argv) {
	srand(time(0));
	Warcaby game;
	
	int recvfd;
	const int on = 1;
	socklen_t salen;
	struct sockaddr *sarecv;
	struct sockaddr_in6 *ipv6addr;
	struct sockaddr_in *ipv4addr;

	string multicastInterface;
	string multicastAddress{DEF_MADDR};
	int  multicastPort = DEF_MPORT;

	string unicastAddress{};	// unicast listening address TODO: try to get it automatically, maybe ioctl
	int unicastPort{}; 

	bool daemon{};
	switch(argc) {
		case 0:
		case 1:
			fprintf(stderr, "usage: interface (maddress mport) (uaddress uport) (daemon 1/0)");
			exit(0);
			break;
		case 7:
			if( (atoi(argv[6]) != 1) && (atoi(argv[6]) != 0) ) {
				fprintf(stderr, "usage: interface (maddress mport) (uaddress uport) (daemon 1/0)");
				exit(0);
			}
			if(atoi(argv[6]) == 1) daemon = true;
		case 6:
			unicastAddress = argv[4];
			unicastPort = atoi(argv[5]);
		case 4:
			multicastInterface = argv[1];
			multicastAddress = argv[2];
			multicastPort = atoi(argv[3]);
		case 2:
		case 3:
			multicastInterface = argv[1];
			break;
		default:
			fprintf(stderr, "usage: interface (maddress mport (uaddress uport) (daemon 1/0)");
			exit(0);
			break;		
	}
	// todo: sprawdzić czy wpisane wartości są prawidłowe
	
	// tworzenie demona jeśli tak wskaże użytkownik
	if(daemon) {
		slog("Started Warcaby server", LOG_DEBUG);
	}
	else cout << "Started Warcaby server\n";
	if(daemon) daemon_init();
	
	sockaddr *udpSA;
	// Tworzenie gniazda do wysyłania komunikatów multicast
	int udpFD = snd_udp_socket(multicastAddress.c_str(), multicastPort, &udpSA, &salen);
	
	udpSA->sa_family = AF_INET;

	sarecv = (sockaddr*)malloc(salen);
	memcpy(sarecv, udpSA, salen);

	setsockopt(udpFD, SOL_SOCKET, SO_BINDTODEVICE, multicastInterface.c_str(), strlen(argv[3]));

	// Ustawianie opcji multicast na gdnieździe udp
	if(sarecv->sa_family == AF_INET6) {
	  ipv6addr = (struct sockaddr_in6 *) sarecv;
	  ipv6addr->sin6_addr =  in6addr_any;

	  int32_t ifindex;
      ifindex = if_nametoindex(multicastInterface.c_str());
      if(setsockopt(udpFD, IPPROTO_IPV6, IPV6_MULTICAST_IF, 
	  				&ifindex, sizeof(ifindex)) < 0){
	  		perror("setting local interface");
			exit(1);};
	}
	if(sarecv->sa_family == AF_INET) {
	  ipv4addr = (struct sockaddr_in *) sarecv;
	  ipv4addr->sin_addr.s_addr =  htonl(INADDR_ANY);

	  struct in_addr        localInterface;
	  localInterface.s_addr = ipv4addr->sin_addr.s_addr;
	if (setsockopt(udpFD, IPPROTO_IP, IP_MULTICAST_IF,
		(char *)&localInterface,
		sizeof(localInterface)) < 0) {
		if(daemon) {
			slog("setting local interface", LOG_DEBUG);
		}
		else perror("setting local interface");
		exit(1);
	  }
	}
	
	char line[MAXLINE];

	// Tworzenie obiektu zawierającego dane o sytemie
	struct utsname	myname;
	if (uname(&myname) < 0) {
		if(daemon) {
			slog("uname error", LOG_DEBUG);
		}
		else perror("uname error");
	}

// Tworzenie zmiennych

	int  tcpListenFD, sndbuf, client1FD, client2FD;
	socklen_t  slen;
	char  buff[MAXLINE];
	time_t  ticks;
	struct sockaddr_in client1Addr, client2Addr;
	int  mss,i;
	int bufsize=2000;
	struct timeval start, stop;
	struct tcp_info tcp_i;

// Tworzenie gniazda nasłuchującego
	if ( (tcpListenFD = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
		if(daemon) {
			string err = "socket error: ";
			err += strerror(errno); 
			slog(err, LOG_DEBUG);
		}
		else fprintf(stderr,"socket error : %s\n", strerror(errno));
		return 1;
	}

	sockaddr_in  tcpSAIN{};
	bzero(&tcpSAIN, sizeof(tcpSAIN));
	tcpSAIN.sin_family = AF_INET;
	//if(argc == 1)
	//tcpSAIN.sin_addr.s_addr = htonl();//htonl(INADDR_ANY);
	tcpSAIN.sin_port = htons(unicastPort);//htons(DEF_UPORT);
	inet_pton(AF_INET, unicastAddress.c_str(), &tcpSAIN.sin_addr);
	
// Ustawianie opcji reuseaddr na gnieździe nasłuchujacym
	sndbuf = 1;
	if (setsockopt(tcpListenFD, SOL_SOCKET, SO_REUSEADDR, &sndbuf, sizeof(sndbuf)) < 0) {
		if(daemon) {
			string err = "tcp SO_REUSEADDR setsockopt error ";
			err += strerror(errno);
			slog(err, LOG_DEBUG);
		}
		else perror("tcp SO_REUSEADDR setsockopt error");
	}

	if ( bind( tcpListenFD, (struct sockaddr *) &tcpSAIN, sizeof(tcpSAIN)) < 0) {
		if(daemon) {
			string err = "tcp bind error ";
			err += strerror(errno);
			slog(err, LOG_DEBUG);
		}
		else perror("tcp bind error");
		return 1;
	}
// Nasłuchiwanie
	if ( listen(tcpListenFD, LISTENQ) < 0) {
		if(daemon) {
			string err = " tcp listen error ";
			err += strerror(errno);
			slog(err, LOG_DEBUG);
		}
		else perror("tcp listen error");
		return 1;
	}

// Tworzenie zmiennych
	int nready{};
	bool canAccept = true;
	bool startListen = false;
	int n{};
	time_t timeNow{};
	string message(MAXLINE, 0);

// Ustawianie polling'u
	array<pollfd, 3> pollFDs; // 1*listen + 2*client
	int pollFDs_s = pollFDs.size();
	pollFDs[0].fd = tcpListenFD;
	pollFDs[0].events = POLLIN;
	for (i = 1; i < pollFDs_s; i++)
		pollFDs[i].fd = -1;		/* -1 indicates available entry */
	int maxi = 0;					/* max index into client[] array */

// Tworzenie zmiennych gry
	bool gameStart{0};
	bool gameStarted{0};
	int playerTurn{};
	char currentPlayer = 'W';
	int playerWhite{}, playerBlack{};
	Warcaby warcaby;
	

// Główna pętla
	while (1) {
		// Rozpoczęcie nasłuchiwania(jeśli się rozłączy gracz)
		if(startListen) {
			if ( (tcpListenFD = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
				if(daemon) {
					string err = "socket error: ";
					err += strerror(errno);
					slog(err, LOG_DEBUG);
				}
				else fprintf(stderr,"socket error : %s\n", strerror(errno));
				
				sleep(1);
				//return 1;
			}
			else if (setsockopt(tcpListenFD, SOL_SOCKET, SO_REUSEADDR, &sndbuf, sizeof(sndbuf)) < 0) {
				if(daemon) {
					string err = "SO_REUSEADDR setsockopt error: ";
					err += strerror(errno);
					slog(err, LOG_DEBUG);
				}
				else fprintf(stderr,"SO_REUSEADDR setsockopt error : %s\n", strerror(errno));
				sleep(1);
			}
			else if( bind( tcpListenFD, (struct sockaddr *) &tcpSAIN, sizeof(tcpSAIN)) < 0 ) {
				if(daemon) {
					string err = "bind error: ";
					err += strerror(errno);
					slog(err, LOG_DEBUG);
				}
				else fprintf(stderr,"bind error : %s\n", strerror(errno));
				sleep(1);
				//return 1;
			}
			else if ( listen(tcpListenFD, LISTENQ) < 0) {
				if(daemon) {
					string err = "listen error: ";
					err += strerror(errno);
					slog(err, LOG_DEBUG);
				}
				else fprintf(stderr,"listen error : %s\n", strerror(errno));
				sleep(1);
				//return 1;
			}
			else startListen = false;
		}
		
		// Polling gniazd
		if ( nready = poll(pollFDs.data(), pollFDs_s, 10); nready < 0 ) { // 10ms poll blocking
			if(daemon) {
				string err = "poll error ";
				err += strerror(errno);
				slog(err, LOG_DEBUG);
			}
			else perror("poll error");
			//exit(1);
		}
		
		// Klient połączył się
		if (canAccept && (pollFDs[0].revents & POLLIN) ) {
			sockaddr_in clientAddress;
			socklen_t clientAddress_s = sizeof(clientAddress);
			int clientConnFD{};
			if ( (clientConnFD = accept(tcpListenFD, (SA *) &clientAddress, &clientAddress_s)) < 0 ) {
					if(daemon) {
						string err = "accept error ";
						err += strerror(errno);
						slog(err, LOG_DEBUG);
					}
					else perror("accept error");
					exit(1);
			}
			string clientAddressStr(INET_ADDRSTRLEN, 0);
			inet_ntop(AF_INET6, (struct sockaddr  *) &clientAddress.sin_addr,  clientAddressStr.data(), clientAddressStr.size());
			if(daemon) {
				string err;
				err += "new client: " + clientAddressStr + ", port " + std::to_string(ntohs(clientAddress.sin_port)) + "\n";
				slog(err, LOG_DEBUG);
			}
			else cout << "new client: " << clientAddressStr << ", port " << ntohs(clientAddress.sin_port) << "\n";
			
			// Wysłanie potwierdzenia połączenia
			if(send(clientConnFD, CONNECTION_ACKNOWLEDGE_MESSAGE, sizeof(CONNECTION_ACKNOWLEDGE_MESSAGE), 0) < 0) {
				if(daemon) {
					string err = "send error ";
					err += strerror(errno);
					slog(err, LOG_DEBUG);
				}
				else perror("send error");
			}
			for (i = 1; i < pollFDs_s; i++) {
				if (pollFDs[i].fd < 0) {
					pollFDs[i].fd = clientConnFD;
					break;
				}
			}
			if (i == pollFDs_s) {	// not possible?
				if(daemon) {
					string err = "too many clients";
					slog(err, LOG_DEBUG);
				}
				else cout << ("too many clients\n");
				continue;
			}

			pollFDs[i].events = POLLIN;
			if (i > maxi) maxi = i;
			
			// Jeśli są 2 klienci to rozpocznij grę, przerwij nasłuchiwanie
			if (pollFDs[1].fd != -1 && pollFDs[2].fd != -1) {
				gameStart = true;
				canAccept = false;
				// stop listening
				close(tcpListenFD);
				pollFDs[0].events = 0; // no more polling of listen socket
				startListen = false;
			}

			if (--nready <= 0) continue;
		}

		// Odczyt danych od klienta
		for (i = 1; i <= maxi; i++) {
			int clientFD = pollFDs[i].fd;
			// client not connected
			if ( clientFD < 0) continue;

			int peerFD = pollFDs[i==1? 2 : 1].fd;

			// client sent data or disconnected
			if (pollFDs[i].revents & (POLLIN | POLLERR)) {
				if ( n = read(clientFD, message.data(), MAXLINE); n < 0) {
					// connection reset
					if (errno == ECONNRESET) {
						if(daemon) {
							string err;
							err += "client[" + std::to_string(i) + "] aborted connection\n";
							slog(err, LOG_DEBUG);
						}
						else printf("client[%d] aborted connection\n", i);
						if(send(peerFD, PEER_DISCONNECT_MESSAGE, sizeof(PEER_DISCONNECT_MESSAGE), 0) < 0) {
							if(daemon) {
								string err = "send error ";
								err += strerror(errno);
								slog(err, LOG_DEBUG);
							}
							else perror("send error");
						}
						//close(clientFD);
						pollFDs[i].fd = -1;
						if(!canAccept) {
							startListen = true;
							canAccept = true;
						}
						gameStarted = 0;
						if(write(peerFD, PEER_DISCONNECT_MESSAGE, sizeof(PEER_DISCONNECT_MESSAGE)) < 0) {
							if(daemon) {
								string err;
								err += "client[" + std::to_string(i) + "] aborted connection\n";
								slog(err, LOG_DEBUG);
							}
							else cout << "client[" + std::to_string(i) + "] aborted connection\n";
						}
						
					}
					// read error
					else{
						if(daemon) {
							string err = "read error ";
							err += strerror(errno);
							slog(err, LOG_DEBUG);
						}
						else perror("read error");
					}
				}
				// connection closed
				else if (n == 0) {
					if(daemon) {
						string err;
						err += "client[" + std::to_string(i) + "] aborted connection\n";;
						slog(err, LOG_DEBUG);
					}
					else cout << "client[" + std::to_string(i) + "] aborted connection\n";
					if(send(peerFD, PEER_DISCONNECT_MESSAGE, sizeof(PEER_DISCONNECT_MESSAGE), 0) < 0) {
						if(daemon) {
							string err = "send error ";
							err += strerror(errno);
							slog(err, LOG_DEBUG);
						}
						else perror("send error");
					}
					//close(clientFD);
					pollFDs[i].fd = -1;
					if(!canAccept) {
						startListen = true;
						canAccept = true;
					}
					gameStarted = 0;
				}
				// received message from client
				else {
					string messageCut(n, 0);
					std::copy(message.cbegin(), message.cbegin() + n, messageCut.begin());
					//cout << "data from client: " << messageCut << '\n';
					string tcpLineCutPrefix;
					int num{};
					for(int j{}; j<n; j+=4) {
						tcpLineCutPrefix = messageCut.substr(j, j+4);
						if(tcpLineCutPrefix.length() < 4) break;
						num = 0;
						for(int s{}; s<messNumber.size(); ++s) {
							//cout << "comparing: " << tcpLineCutPrefix << " and " << messNumber.at(s).substr(0, 4) << '\n';
							if( tcpLineCutPrefix.compare(messNumber.at(s).substr(0, 4)) == 0) {
								num = s+1;
								break;
							}
						}
						switch( num ) {
							case 3:	//READ_ERROR_MESSAGE
								//cout << "read err\n";
								// not implemented
								break;
							case 6: //SEND_MOVE
								cout << "received move message: "<< messageCut.substr(j, j+8) << std::endl;
								// check if it is this player's turn
								if( (i == playerTurn) && (messageCut.length() >= 7)) {
									j+=4;
									string moveParams = messageCut.substr(j, j+4);
									if(moveParams.length() < 4) {
										//cout << "move params too short\n";
										break;
									}
									else j+= 4;
									int fromRow{}, toRow{};
									char fromCol{}, toCol{};
									fromRow = moveParams.at(0) - 48;
									fromCol = moveParams.at(1);
									toRow = moveParams.at(2) - 48;
									toCol = moveParams.at(3);
									//cout << moveParams << '\n';
									cout << fromRow << ' ' << fromCol << ' ' << toRow << ' ' << toCol << ' ' << currentPlayer << std::endl;
									//warcaby.printBoard();
									if( !warcaby.validateMakeMove(fromRow, fromCol, toRow, toCol, currentPlayer) ) {
										//cout << "bad move\n";
										// todo: tell client move is wrong
										if(daemon) {
											string err;
											err += "wrong move";
											slog(err, LOG_DEBUG);
										}
										else cout << "wrong move\n";
									}
									else {
										if(send(clientFD, MOVE_OK, sizeof(MOVE_OK), 0) < 0) {
											if(daemon) {
												string err = "send error ";
												err += strerror(errno);
												slog(err, LOG_DEBUG);
											}
											else perror("send error");
										}
										if(send(peerFD, messageCut.substr(j-8, j).data(), sizeof(messageCut.substr(j-4, j+4)), 0) < 0) {
											if(daemon) {
												string err = "send error ";
												err += strerror(errno);
												slog(err, LOG_DEBUG);
											}
											else perror("send error");
										}
									}
									currentPlayer = currentPlayer=='W'? 'B' : 'W';
									playerTurn = playerTurn==1? 2 : 1;
								}
								break;
							default:
								if(daemon) {
									string err;
									err += "error deciphering message";
									slog(err, LOG_DEBUG);
								}
								else cout << "error deciphering message\n";
								break;
						}
					}
				}
				// end received message from client

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		} // end check client data

		// Wysyłanie wiadomości multicast z portem do połączenia
		if( time(0)-MULTICAST_DELAY >= timeNow ) {
			timeNow = time(0);
			string udpMess = std::to_string(unicastPort);
			if( sendto(udpFD, udpMess.c_str(), udpMess.length(), 0, udpSA, salen) < 0 ) {
				if(daemon) {
					string err = "sendto error ";
					err += strerror(errno);
					slog(err, LOG_DEBUG);
				}
				else perror("sendto error");
			}
		}

		// Rozpoczęcie gry(setup)
		if(gameStart) {
			warcaby.initializeBoard();
			playerWhite = 1 + rand()%2;
			playerBlack = playerWhite==1? 2 : 1;
			playerTurn = playerWhite;
			// Wysłanie graczom informacji o kolejności gry
			if(send(pollFDs[playerWhite].fd, ASSIGNED_WHITE_MESSAGE, sizeof(ASSIGNED_WHITE_MESSAGE), 0) < 0) {
				if(daemon) {
					string err = "send error ";
					err += strerror(errno);
					slog(err, LOG_DEBUG);
				}
				else perror("send error");
			}
			if(send(pollFDs[playerBlack].fd, ASSIGNED_BLACK_MESSAGE, sizeof(ASSIGNED_BLACK_MESSAGE), 0) < 0) {
				if(daemon) {
					string err = "send error ";
					err += strerror(errno);
					slog(err, LOG_DEBUG);
				}
				else perror("send error");
			}
			gameStarted = true;
			gameStart = false;
		}
	}
}

//Funkcja tworząca proces demona dla serwera
int daemon_init() {
	int		i, p;
	pid_t	pid;

	if ( (pid = fork()) < 0)
		return (-1);
	else if (pid)
		exit(0);			/* parent terminates */

	/* child 1 continues... */

	if (setsid() < 0)			/* become session leader */
		return (-1);

	signal(SIGHUP, SIG_IGN);
	if ( (pid = fork()) < 0)
		return (-1);
	else if (pid)
		exit(0);			/* child 1 terminates */

	/* child 2 continues... */

	chdir("/tmp");				/* change working directory  or chroot()*/
//	chroot("/tmp");

	/* redirect stdin, stdout, and stderr to /dev/null */
	p= open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	//openlog(pname, LOG_PID, facility);
	
    //syslog(LOG_ERR," STDIN =   %i\n", p);
	setuid(1000); /* change user */
	
	return (0);				/* success */
}

// Funkcja dodająca wpis do logu systemu 
// przykładowe facility: LOG_USER, LOG_DEBUG, LOG_NOTICE, LOG_WARNING
int slog(string log, int facility) {
	setlogmask (LOG_UPTO (LOG_NOTICE | LOG_INFO));
	
	openlog (NULL, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
	
	syslog (facility, "%s", log.c_str());
	
	closelog ();
	return 0;
}