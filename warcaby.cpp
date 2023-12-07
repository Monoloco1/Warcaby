#include <iostream>
#include <vector>

#using std::vector;

enum pole {
    empty, black, white
}

#using PV = vector<pole>;
#using PM = vector< vector<pole> >

class Warcaby {
private:
    PM board;

};