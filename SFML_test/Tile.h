#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
using namespace std;

class Tile {
public:
	string spriteName; 
	string revealedSpriteName = " ";
	bool is_Hidden = true; //True if tile is still hidden (has not been clicked), otherwise false
	bool flagged = false; //true = flag shown, false = no flag;
	int xPos;
	int yPos;
	int numMineNeighbors;
	bool debugReveal = false; //True when debug is selected

	Tile();
	Tile(int x, int y);

	void invertFlag();

	void reveal();

	void setRevealedName(string s);

	void increaseMineCount();
};

