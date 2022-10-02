#include "Tile.h"

Tile::Tile() {
	spriteName = "tile_hidden";

	is_Hidden = true;

	xPos = 0;
	yPos = 0;

	flagged = false;

	numMineNeighbors = 0;
}

Tile::Tile(int x, int y) {
	if (flagged) {
		spriteName = "flag";
	}
	else if (is_Hidden) {
		spriteName = "tile_hidden";
	}
	else if (!is_Hidden) {
		spriteName = revealedSpriteName;
	}

	xPos = x;
	yPos = y;
	
	numMineNeighbors = 0;
}

void Tile::invertFlag() {
		flagged = !flagged;
	}

void Tile::reveal() {
	is_Hidden = false;
}

void Tile::setRevealedName(string s) {
	revealedSpriteName = s;
}

void Tile::increaseMineCount() {
	numMineNeighbors++;
}
