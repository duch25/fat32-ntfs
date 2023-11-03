#include "Element.h"

Element::Element()
{
	this->name = "No-know";
	this->size = -1;
	this->level = -1;
	this->first_sector = -1;
	this->isFolder = false;
	this->type = "No-know";
}

int Element::getSize()
{
	return this->size;
}

int Element::getID()
{
	return this->first_sector;
}

string Element::getType()
{
	return type;
}

string Element::getName()
{
	return this->name;
}

void Element::getElement()
{
	cout << Element::tabLevel(this->level) << this->name;
	
	cout << "("<< this->first_sector<<")";
	cout << endl;
}

Element::Element(string name, int first_sector,int level, int size, string type)
{
	this->name = "";
	int n = (int)name.size() -1;
	if (int(name[n]) == -1)
		while (int(name[n]) == -1)
			n--;
	for (int i = 0; i < n+1; i++)
		this->name += name[i];
	this->first_sector = first_sector;
	this->level = level;
	this->size = size;
	this->isFolder = size == 0 ? true : false;
	this->type = type;
}

string Element::tabLevel(int level)
{
	string tab = "";
	for (int i = 0; i < level; i++)
		tab += "\t";
	return tab;
}

int Element::getLevel() {
	return level;
}
