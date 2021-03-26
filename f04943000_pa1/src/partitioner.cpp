#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <map>
#include "cell.h"
#include "net.h"
#include "partitioner.h"
using namespace std;


void Partitioner::parseInput(fstream& inFile)
{
    string str;
    // Set balance factor
    inFile >> str;
    _bFactor = stod(str);

    // Set up whole circuit
    while (inFile >> str) {
        if (str == "NET") {
            string netName, cellName, tmpCellName = "";
            inFile >> netName;
            int netId = _netNum;
            _netArray.push_back(new Net(netName));
            _netName2Id[netName] = netId;
            while (inFile >> cellName) {
                if (cellName == ";") {
                    tmpCellName = "";
                    break;
                }
                else {
                    // a newly seen cell
                    if (_cellName2Id.count(cellName) == 0) {
                        int cellId = _cellNum;
                        _cellArray.push_back(new Cell(cellName, 0, cellId));
                        _cellName2Id[cellName] = cellId;
                        _cellArray[cellId]->addNet(netId);
                        _cellArray[cellId]->incPinNum();
                        _netArray[netId]->addCell(cellId);
                        ++_cellNum;
                        tmpCellName = cellName;
                    }
                    // an existed cell
                    else {
                        if (cellName != tmpCellName) {
                            assert(_cellName2Id.count(cellName) == 1);
                            int cellId = _cellName2Id[cellName];
                            _cellArray[cellId]->addNet(netId);
                            _cellArray[cellId]->incPinNum();
                            _netArray[netId]->addCell(cellId);
                            tmpCellName = cellName;
                        }
                    }
                }
            }
            ++_netNum;
        }
    }
    return;
}

void Partitioner::initParti()
{
	//move node and set part size and set max pin
	const int halfNode = getCellNum() / 2;
	Cell* cell;
	for(size_t i = 0; i < halfNode; ++i){
		cell = _cellArray[i];
		cell->move();
	}
	setBSize(halfNode);

	//set max pin num
	int maxPin = 0;
	for(size_t i = 0, n = getCellNum(); i < n; ++i){
		cell = _cellArray[i];
		if(cell->getPinNum() > maxPin) maxPin = cell->getPinNum();
	}
	setMaxPinNum(maxPin); 
	//cout << "max pin num = " << maxPin << endl;

	//set part count and cut size
	int cutSize = 0;
	Net* net;
	vector<int>* cellList;
	for(size_t i = 0, n = getNetNum(); i < n; ++i){
		net = _netArray[i];
		cellList = net->getCellListPtr();
		for(size_t j = 0, m = cellList->size(); j < m; ++j){
			cell = _cellArray[(*cellList)[j]];
			const bool& part = cell->getPart();
			//set part count
			net->incPartCount(part);
		}
		//record cut size
		if(net->getPartCount(0) > 0 && net->getPartCount(1) > 0) ++cutSize;

	}
	setCutSize(cutSize);
}

/*
void Partitioner::initBList()
{
	for(int i = -_maxPinNum; i <= _maxPinNum; ++i){
		_bList[0][i] = new Node(-1);
		_bList[1][i] = new Node(-1);
	}
}
*/

void Partitioner::setInitG()
{
	const int n = getCellNum();
	int maxG = 0;
	int maxP = 0;
	//Cell* maxGainCell;
	Cell* cell;
	Node* node;
	vector<int>* netList;
	Net* net;
	for(size_t i = 0; i < n; ++i){
		cell = _cellArray[i];
		const bool& part = cell->getPart();
        netList = cell->getNetListPtr();
		//compute gain of each cell
		for (size_t j = 0, m = netList->size(); j < m; ++j) {
			net = _netArray[(*netList)[j]];
			if(net->getPartCount(part) == 1){
				cell->incGain();
			}
			else if(net->getPartCount(!part) == 0){
				cell->decGain();
			}
		}
		
		const int& gain = cell->getGain();
		//record max gain cell
		if(gain > maxG) {
			maxP = part;
			maxG = gain;
		}
		//record bList
		node = cell->getNode();
		insertNode(node);
	}
	setMaxGainNode(_bList[maxP][maxG]);

	//for debug
	printBList();
}

void Partitioner::printBList()
{
	Node* node;
	cout << endl << "A list:" << endl;
	for(map<int, Node*>::iterator it = _bList[0].begin(); it != _bList[0].end(); ++it){
		node = it->second;
		//if(node->getNext() == NULL) continue;
		cout << "gain = " << it->first << endl;
		while(node->getNext() != NULL){
			node = node->getNext();
            cout << "	---> node " << _cellArray[node->getId()]->getName() << endl;
        }
	}

	cout << endl << "B list:" << endl;
    for(map<int, Node*>::iterator it = _bList[1].begin(); it != _bList[1].end(); ++it){
        node = it->second;
		//if(node->getNext() == NULL) continue;
        cout << "gain = " << it->first << endl;
        while(node->getNext() != NULL){
            node = node->getNext();
            cout << "	---> node " << _cellArray[node->getId()]->getName() << endl;
        }
    }
	cout << endl;
}

void Partitioner::deleteNode(Node* node)
{
	Node* pre = node->getPrev();
	Node* next = node->getNext();

	pre->setNext(next);	
	if(next != NULL){
		//pre->setNext(next);
		next->setPrev(pre);
	}
	
	else if(pre->getPrev() == NULL){
		/*
		//check if this empty entry is max gain
		if(pre == _maxGainCell){
			const int& maxGain = _cellArray[node->getId()]->getGain();
		}
		*/
		const int& gain = pre->getId();
		if(_bList[0].find(gain) != _bList[0].end() && _bList[0][gain] == pre){
			_bList[0].erase(gain);
		}
		else if(_bList[1].find(gain) != _bList[1].end() && _bList[1][gain] == pre){
			_bList[1].erase(gain);
		}
		else{
			cerr << "error in delete" << endl;
			cerr << "cell: " << _cellArray[node->getId()]->getName() << endl;
			cerr << "old gain: " << pre->getId() << endl;
			cerr << "new gain: " << _cellArray[node->getId()]->getGain() << endl;
		}
			
	}
}

void Partitioner::insertNode(Node* node)
{
	Cell* cell = _cellArray[node->getId()];
	const int& gain = cell->getGain();
	const int& part = cell->getPart();

	
	//if open an empty entry, insert a dummy node first
	map<int, Node*>::iterator it = _bList[part].find(gain);
	if(it == _bList[part].end()){
		_bList[part][gain] = new Node(gain);
	}

	
	Node* dNode = _bList[part][gain];
	Node* next = dNode->getNext();
	node->setPrev(dNode);
	node->setNext(next);
	dNode->setNext(node);
	if(next != NULL) next->setPrev(node);
	
}

void Partitioner::updateList(Cell* baseCell)
{
	vector<int>* netList = baseCell->getNetListPtr();
	Net* net;
	Cell* cell;
	Node* node;
	vector<int>* cellList;
	
	vector<bool> recordMap;
	recordMap.reserve(_cellNum);
	recordMap.resize(_cellNum);
	for(size_t i = 0, n = netList->size(); i < n; ++i){
		net = _netArray[(*netList)[i]];
		cellList = net->getCellListPtr();
		for(size_t j = 0, m = cellList->size(); j < m; ++j){
			cell = _cellArray[(*cellList)[j]];
			node = cell->getNode();
			if(!recordMap[node->getId()]){//if haven't update
				deleteNode(node);
				insertNode(node);
				recordMap[node->getId()] = true;
			}
		}
	}
}

void Partitioner::changeAllGainOnNet(Net* net, const bool& b)
{
	//cout << "for all cell in net " << net->getName() << endl;
	vector<int>* cellList = net->getCellListPtr();
	Cell* cell;
	Node* node;
    for(int j = 0, m = cellList->size(); j < m; ++j){
        cell = _cellArray[(*cellList)[j]];
        if(!cell->getLock()){
			if(b) {
				cell->incGain();
			}
			else {
				cell->decGain();
			}
        }
    }
}

void Partitioner::changeOneGainOnNet(Net* net, const bool& b, const bool& part)
{
	//cout << "for one cell in net " << net->getName() << " in part " << part << endl;
	vector<int>* cellList = net->getCellListPtr();
	Cell* cell;
	Node* node;
    for(int j = 0, m = cellList->size(); j < m; ++j){
        cell = _cellArray[(*cellList)[j]];
        if(cell->getPart() == part && (!cell->getLock()) ){
			//cout << "---> find cell " << cell->getName() << endl;
            if(b) {
	            cell->incGain();
			}
			else {
				cell->decGain();
			}
			break;
        }
    }
}

void Partitioner::updateGain(Cell* baseCell)
{
	//cout << "consider cell " << baseCell->getName() << endl;
	const bool& F = baseCell->getPart();
	const bool& T = !F;

	baseCell->move();
	baseCell->lock();

	//update gain
	vector<int>* netList = baseCell->getNetListPtr();
	Net* net;
    //for each net on base cell
    for (size_t j = 0, m = netList->size(); j < m; ++j) {
        net = _netArray[(*netList)[j]];
		//cout << "consider net " << net->getName() << endl;
		//before move
		if(net->getPartCount(T) == 0){
			//++gain for all free cell on n
			changeAllGainOnNet(net, 1);//1 for inc		
		}
		else if(net->getPartCount(T) == 1){
			//--gain of that cell
			changeOneGainOnNet(net, 0, T);// 0 for dec
		}

		//during move
		//F = F - 1 
		net->decPartCount(F);
		//T = T + 1
		net->incPartCount(T);

		//after move
		if(net->getPartCount(F) == 0){
			//--gain for all cell on net
			changeAllGainOnNet(net, 0); // 0 for dec
		}
		else if(net->getPartCount(F) == 1){
            //++gain of that cell
			changeOneGainOnNet(net, 1, F); //1 for inc
        }
    }//done for each net on base cell
}
/*
Cell* pickBaseCell()
{
	Cell* baseCell = _cellArray[_maxGainCell->getNext()->getId()];
	return baseCell;
}
*/
void Partitioner::partition()
{
	//TODO
	//initial partition
	initParti();
	//initial bucket list
	//initBList();
	//set initial gain
	setInitG();

	//iteratively until no improve
	do{
		//for loop until all lock
		for(_iterNum = 0; _iterNum <_cellNum; ++_iterNum){
			//move the node with max gain
			
			//Cell* baseCell = pickBaseNode()
			Cell* baseCell = _cellArray[_iterNum];
			
			//for debug, try to move each node
			updateGain(baseCell);
			updateList(baseCell);
			//updateMaxGainPtr();
			
			cout << "after moving cell " << baseCell->getName() << endl;
			printBList();
			/*		
			for(int i = 0; i < _cellNum; ++i){
				Cell* cell = _cellArray[i];
				cout << "(cell, gain) = (" << cell->getName() << ", " << cell->getGain() << ")" << endl; 
			}
			*/
			//update gain, size, lock
		
		}
		//find max subseq

			
		
	} while(_maxAccGain > 0);

}

void Partitioner::printSummary() const
{
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[0] << endl;
    cout << " Cell Number of partition B: " << _partSize[1] << endl;
    cout << "=================================================" << endl;
    cout << endl;
    return;
}

void Partitioner::reportNet() const
{
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i) {
        cout << setw(8) << _netArray[i]->getName() << ": ";
        vector<int>* cellList = _netArray[i]->getCellListPtr();
        for (size_t j = 0, end_j = cellList->size(); j < end_j; ++j) {
            cout << setw(8) << _cellArray[(*cellList)[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportCell() const
{
    cout << "Number of cells: " << _cellNum << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        vector<int>* netList = _cellArray[i]->getNetListPtr();
        for (size_t j = 0, end_j = netList->size(); j < end_j; ++j) {
            cout << setw(8) << _netArray[(*netList)[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::writeResult(fstream& outFile)
{
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 0) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 1) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    return;
}

void Partitioner::clear()
{
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i) {
        delete _netArray[i];
    }
    return;
}
