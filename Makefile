all: warcaby_client warcaby_server

warcaby_client:
	g++ -L. warcaby_client.cpp warcaby.cpp const.cpp -o warcaby_client -std=c++2a

warcaby_server:
	g++ -L. warcaby_server.cpp warcaby.cpp const.cpp -o warcaby_server -std=c++2a
