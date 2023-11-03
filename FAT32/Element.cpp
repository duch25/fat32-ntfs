#include "Element.h"

Element::Element()
{
	this->name = "No-know";
	this->size = -1;
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

string padding = "";

void Element::getElement()
{
	cout << padding << this->name;
	
	cout << "("<< this->first_sector<<")\n";
	if (this->isFolder) {
		padding += "    ";
		for (auto c : this->items) {
			c.getElement();
		}
		for (int i = 0; i < 4; ++i) padding.pop_back();
	}
}

Element::Element(string name, int first_sector, int size, string type)
{
	this->name = "";
	int n = (int)name.size() -1;
	if (int(name[n]) == -1)
		while (int(name[n]) == -1)
			n--;
	for (int i = 0; i < n+1; i++)
		this->name += name[i];
	this->first_sector = first_sector;
	this->size = size;
	this->isFolder = size == 0 ? true : false;
	this->type = type;
}

