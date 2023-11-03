#pragma once
#include <iostream>
#include <string>
#include<vector>
using namespace std;

// Class của tập tin, thư mục
class Element
{
protected:
	string name;
	int first_sector;
	int level;
	int size;
	string type;
	bool isFolder; 
	vector<Element> items;
public:

	Element();
	Element(string name, int first_sector, int level,int size, string type);
	//Lay kich thuoc cua TT/TM
	int getSize();
	//Lay sector dau tien cua TT/TM dat no lam ID
	int getID();
	//Lay ra loai cua tap tin thu muc
	string getType();
	//in ra ten va ID cua TT/TM
	void getElement();
	//Lay ra ten TT/TM
	string getName();
	//Tab phan cap
	static string tabLevel(int level);
	//Lấy level
	int getLevel();


};
