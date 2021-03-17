#ifndef NET_H
#define NET_H

#include <vector>
using namespace std;

class Net
{
public:
    // constructor and destructor
    Net(string& name) :
        _name(name) {
        _partCount[0] = 0; _partCount[1] = 0;
    }
    ~Net()  { }

    // basic access methods
    string getName()           const { return _name; }
    int getPartCount(int part) const { return _partCount[part]; }
    //vector<int> getCellList()  const { return _cellList; }

    // set functions
    void setName(const string name) { _name = name; }
    void setPartCount(int part, const int count) { _partCount[part] = count; }

    // modify methods
    void incPartCount(int part)     { ++_partCount[part]; }
    void decPartCount(int part)     { --_partCount[part]; }
    void addCell(const int cellId)  { _cellList.push_back(cellId); }

	//my function
    vector<int>* getCellListPtr() { return &_cellList; }
	void setAlongCell(int part, const int cellId){ _alongCell[part] = cellId; }
	int getAlongCell(int part) { return _alongCell[part]; }

private:
    int             _partCount[2];  // Cell number in partition A(0) and B(1)
    string          _name;          // Name of the net
    vector<int>     _cellList;      // List of cells the net is connected to
	//my implement
	int             _alongCell[2];  // The only cell in A(0) and B(1)
};

#endif  // NET_H
