#include "Tile.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <vector>
#include <string>


void readConfigFile(string fileName, int& col, int& row, int& numMines) {
    ifstream file(fileName);
    if (file.is_open()) {
        string c;
        string r;
        string nm;
        getline(file, c);
        getline(file, r);
        getline(file, nm);

        col = stoi(c);
        row = stoi(r);
        numMines = stoi(nm);
    }
    file.close();
}

void initializeBoardData(vector <string>& boardData, int row, int col, int numMines) {
    int max;
    max = row*col; //set the upper bound to generate the random number
    srand(time(0));
    int random;
    int count = 0;

    vector <int> mineData(row * col,0);

   while(count < numMines) { //generate numMines random numbers
        random = rand() % max;
        //cout << "The random number is: " << random << endl;
        if (mineData[random] != 1) {
            count++;
            mineData[random] = 1;
        }
    }
   //cout << "mineData initialized properly" << endl;

   int index = 0; //Used to ensure each index of mineData is added
    for (int r = 0; r < row; r++) {
        for (int c = 0; c < col; c++) {
            boardData[r] += to_string(mineData[index]);
            index++;
        }
        //cout << boardData[r] << endl;
    }
    mineData.clear();
}

void updateDigits(int numMines, int (&digits)[3]) {
    digits[0] = numMines / 100;
    digits[1] = numMines%100 / 10;
    digits[2] = (numMines % 100) % 10;
}

void updateSprite(sf::Sprite& tile, Tile& t, map <string, sf::Sprite>& sprites, sf::RenderWindow& window, bool gameLose) {
    if (t.debugReveal) {
        tile = sprites["mine"];
        tile.setPosition(t.xPos, t.yPos);
    }
    else if (t.flagged) {
        tile = sprites["flag"];
        tile.setPosition(t.xPos, t.yPos);
        //window.draw(sprites["tile_hidden"]);
   }
    else if (!t.is_Hidden || t.debugReveal) {
        tile = sprites[t.revealedSpriteName];
        tile.setPosition(t.xPos, t.yPos);
        //window.draw(sprites["tile_revealed"]);
    }
    else if (!t.flagged) {
        tile = sprites[t.spriteName];
        tile.setPosition(t.xPos, t.yPos);
        //window.draw(sprites["tile_revealed"]);
    }
    if (!gameLose && t.debugReveal || t.flagged) {
        sprites["tile_hidden"].setPosition(t.xPos, t.yPos);
        window.draw(sprites["tile_hidden"]);
    }
    else {
        sprites["tile_revealed"].setPosition(t.xPos, t.yPos);
        window.draw(sprites["tile_revealed"]);
    }
    window.draw(tile);
}

//Will read in the board file indicated by fileName, and store that data into boardData
//If row and col are greater than board file size, extra 0's will be added to fill in the gaps to match required board size
void readBoardFile(string fileName, vector <string>& boardData, int row, int col) {
    boardData.clear();
    ifstream file(fileName, ios_base::binary);
    if (file.is_open()) {
        string line;

        //This will add zeroes to the end of the getline to accomodate for the extra space allocated by config file
        string rowAppend = "";
        for (int r = 25; r < row; r++) {
            rowAppend += "0";
        }
        while (getline(file, line)) {
            //cout << line << endl;
            boardData.push_back(line + rowAppend);
        }

        //For each column needed after testBoard is initialized, create a string of all zeroes of row length and add it to vector
        for (int r = 0; r < 25; r++)
            rowAppend += "0";
        for (int c = 16; c < col; c++) {
            boardData.push_back(rowAppend);
        }
    }
    file.close();
}

//Reveals tiles above, below, left, and right of given tile at xPos, yPos if coordinates are valid
void revealNeighboringTiles(Tile& t, int xPos, int yPos, map <string, Tile>& tiles, int row, int col, int& tilesRemaining) {
    string mapKey;
    //Left of given tile
    if (xPos - 1 >= 0) {
        mapKey = to_string(xPos - 1) + "_" + to_string(yPos);
        if (!tiles[mapKey].flagged && tiles[mapKey].is_Hidden && tiles[mapKey].revealedSpriteName == "tile_revealed") {
            tiles[mapKey].reveal();
            tilesRemaining--;
            revealNeighboringTiles(tiles[mapKey], xPos - 1, yPos, tiles, row, col, tilesRemaining);
        }
    }
    //Right of given tile
    if (xPos + 1 < col) {
        mapKey = to_string(xPos + 1) + "_" + to_string(yPos);
        if (!tiles[mapKey].flagged && tiles[mapKey].is_Hidden && tiles[mapKey].revealedSpriteName == "tile_revealed") {
            tiles[mapKey].reveal();
            tilesRemaining--;
            revealNeighboringTiles(tiles[mapKey], xPos + 1, yPos, tiles, row, col, tilesRemaining);
        }
    }
    //Below given tile
    if (yPos + 1 < row) {
        mapKey = to_string(xPos) + "_" + to_string(yPos+1);
        if (!tiles[mapKey].flagged && tiles[mapKey].is_Hidden && tiles[mapKey].revealedSpriteName == "tile_revealed") {
            tiles[mapKey].reveal();
            tilesRemaining--;
            revealNeighboringTiles(tiles[mapKey], xPos, yPos+1 , tiles, row, col,tilesRemaining);
        }
    }
    //Above given tile
    if (yPos - 1 >= 0) {
        mapKey = to_string(xPos) + "_" + to_string(yPos - 1);
        if (!tiles[mapKey].flagged && tiles[mapKey].is_Hidden && tiles[mapKey].revealedSpriteName == "tile_revealed") {
            tiles[mapKey].reveal();
            tilesRemaining--;
            revealNeighboringTiles(tiles[mapKey], xPos, yPos - 1, tiles, row, col,tilesRemaining);
        }
    }
}

int main() {
    //Create all sprites with textures loaded from images file and store in a map (the key is the same name as the png name)
    map <string, sf::Sprite> sprites;
    sf::Texture debug_t;
    debug_t.loadFromFile("images/debug.png");
    sf::Sprite debug_sp;
    debug_sp.setTexture(debug_t);
    sprites.emplace("debug", debug_sp);

    sf::Texture digits_t;
    digits_t.loadFromFile("images/digits.png");
    sf::Sprite digits_sp;
    digits_sp.setTexture(digits_t);
    sprites.emplace("digits", digits_sp);

    sf::Texture face_happy_t;
    face_happy_t.loadFromFile("images/face_happy.png");
    sf::Sprite face_happy_sp;
    face_happy_sp.setTexture(face_happy_t);
    sprites.emplace("face_happy", face_happy_sp);

    sf::Texture face_lose_t;
    face_lose_t.loadFromFile("images/face_lose.png");
    sf::Sprite face_lose_sp;
    face_lose_sp.setTexture(face_lose_t);
    sprites.emplace("face_lose", face_lose_sp);

    sf::Texture face_win_t;
    face_win_t.loadFromFile("images/face_win.png");
    sf::Sprite face_win_sp;
    face_win_sp.setTexture(face_win_t);
    sprites.emplace("face_win", face_win_sp);

    sf::Texture flag_t;
    flag_t.loadFromFile("images/flag.png");
    sf::Sprite flag_sp;
    flag_sp.setTexture(flag_t);
    sprites.emplace("flag", flag_sp);

    sf::Texture mine_t;
    mine_t.loadFromFile("images/mine.png");
    sf::Sprite mine_sp;
    mine_sp.setTexture(mine_t);
    sprites.emplace("mine", mine_sp);

    sf::Texture number_1_t;
    number_1_t.loadFromFile("images/number_1.png");
    sf::Sprite number_1_sp;
    number_1_sp.setTexture(number_1_t);
    sprites.emplace("number_1", number_1_sp);

    sf::Texture number_2_t;
    number_2_t.loadFromFile("images/number_2.png");
    sf::Sprite number_2_sp;
    number_2_sp.setTexture(number_2_t);
    sprites.emplace("number_2", number_2_sp);

    sf::Texture number_3_t;
    number_3_t.loadFromFile("images/number_3.png");
    sf::Sprite number_3_sp;
    number_3_sp.setTexture(number_3_t);
    sprites.emplace("number_3", number_3_sp);

    sf::Texture number_4_t;
    number_4_t.loadFromFile("images/number_4.png");
    sf::Sprite number_4_sp;
    number_4_sp.setTexture(number_4_t);
    sprites.emplace("number_4", number_4_sp);

    sf::Texture number_5_t;
    number_5_t.loadFromFile("images/number_5.png");
    sf::Sprite number_5_sp;
    number_5_sp.setTexture(number_5_t);
    sprites.emplace("number_5", number_5_sp);

    sf::Texture number_6_t;
    number_6_t.loadFromFile("images/number_6.png");
    sf::Sprite number_6_sp;
    number_6_sp.setTexture(number_6_t);
    sprites.emplace("number_6", number_6_sp);

    sf::Texture number_7_t;
    number_7_t.loadFromFile("images/number_7.png");
    sf::Sprite number_7_sp;
    number_7_sp.setTexture(number_7_t);
    sprites.emplace("number_7", number_7_sp);

    sf::Texture number_8_t;
    number_8_t.loadFromFile("images/number_8.png");
    sf::Sprite number_8_sp;
    number_8_sp.setTexture(number_8_t);
    sprites.emplace("number_8", number_8_sp);

    sf::Texture test_1_t;
    test_1_t.loadFromFile("images/test_1.png");
    sf::Sprite test_1_sp;
    test_1_sp.setTexture(test_1_t);
    sprites.emplace("test_1", test_1_sp);

    sf::Texture test_2_t;
    test_2_t.loadFromFile("images/test_2.png");
    sf::Sprite test_2_sp;
    test_2_sp.setTexture(test_2_t);
    sprites.emplace("test_2", test_2_sp);

    sf::Texture test_3_t;
    test_3_t.loadFromFile("images/test_3.png");
    sf::Sprite test_3_sp;
    test_3_sp.setTexture(test_3_t);
    sprites.emplace("test_3", test_3_sp);

    sf::Texture tile_hidden_t;
    tile_hidden_t.loadFromFile("images/tile_hidden.png");
    sf::Sprite tile_hidden_sp;
    tile_hidden_sp.setTexture(tile_hidden_t);
    sprites.emplace("tile_hidden", tile_hidden_sp);

    sf::Texture tile_revealed_t;
    tile_revealed_t.loadFromFile("images/tile_revealed.png");
    sf::Sprite tile_revealed_sp;
    tile_revealed_sp.setTexture(tile_revealed_t);
    sprites.emplace("tile_revealed", tile_revealed_sp); //This is the collapsed code (using ctrl+h) for creating all sprites!

    map <string, Tile> tiles; //map which will hold all tiles printed to the board

    int col = 0;
    int row = 0;
    int numMines = 0;
    int numFlags = 0;
    readConfigFile("boards/config.cfg", col, row, numMines);
    int mineTotalDisplayed = numMines - numFlags;
    int tilesRemaining = row * col;

    vector <string> boardData(row* col, ""); //vector size will equal num rows, each string in it will be num cols length
    initializeBoardData(boardData, row, col, numMines);
    numMines = 0;

    int width = col * 32;
    int height = (row * 32) + 100;
    int tileCount = col * row;

    //positions of face icon and test buttons 1-3
    int face_pos;
    int test_pos;
    int test_pos2;
    int test_pos3;
    string boardFile = "boards/testboard1.brd"; //will be loaded with file name of board to be loaded
    string faceFile = "face_happy";

    sf::Sprite tile; //Sprite will be taken from the sprites map using tile's "spriteName" and xPos/yPos to emplace
    //Create a window
    sf::RenderWindow window(sf::VideoMode(width, height), "My window", sf::Style::Close);
    // run the program as long as the window is open
    bool initialize = true;
    bool revealMines = false; //True when debug button is clicked
    bool flagMines = false; //True when victory achieved
    bool gameOver = false; //true when mine is revealed
    bool gameLost = false;
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            //When mouse buttom event detected enter here
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Right)
                {
                    if (!gameOver && event.mouseButton.x < width && event.mouseButton.y < height - 100) {
                        int xPosition = event.mouseButton.x / 32;
                        int yPosition = event.mouseButton.y / 32;
                        string mapKey = to_string(xPosition) + "_" + to_string(yPosition);

                        if (tiles[mapKey].is_Hidden) {
                            tiles[mapKey].invertFlag();
                            if (tiles[mapKey].flagged == true)
                                numFlags++;
                            else
                                numFlags--;
                            updateSprite(tile, tiles[mapKey], sprites, window, gameLost);

                            cout << "RIGHT CLICK " << "mapKey is " << mapKey << endl << "Flag is " << tiles[mapKey].flagged << endl;
                            cout << "Number of flags: " << numFlags << endl << endl;
                            //cout << "Coordinates are " << xPosition << " and " << yPosition << endl << endl;
                        }
                    }
                }

                else if (event.mouseButton.button == sf::Mouse::Left)
                {
                    if (!gameOver && event.mouseButton.x < width && event.mouseButton.y < height - 100) {
                        int xPosition = event.mouseButton.x / 32;
                        int yPosition = event.mouseButton.y / 32;
                        string mapKey = to_string(xPosition) + "_" + to_string(yPosition);

                        //Only reveal if not flagged and is hidden
                        if (!tiles[mapKey].flagged && tiles[mapKey].is_Hidden) {
                            tiles[mapKey].reveal(); 
                            tilesRemaining--;

                            if (tiles[mapKey].revealedSpriteName == "mine") {
                                revealMines = true;
                                faceFile = "face_lose";
                                gameOver = true;
                                gameLost = true;
                            }
                            else if (tiles[mapKey].revealedSpriteName == "tile_revealed") {
                                revealNeighboringTiles(tiles[mapKey], xPosition, yPosition, tiles, row, col, tilesRemaining);
                            }

                            //Check if game win
                            if (tilesRemaining - numMines == 0) {
                                gameOver = true;
                                faceFile = "face_win";
                                flagMines = true;
                                numFlags=numMines;
                                revealMines = false;
                                }

                        }
                        updateSprite(tile, tiles[mapKey], sprites, window, gameLost);

                        //cout << "LEFT CLICK " << "mapKey is " << mapKey << endl;
                        //cout << "Coordinates are " << xPosition << " and " << yPosition << endl << endl;
                    }

                    //Face icon is selected, create new board from scratch
                    else if (event.mouseButton.x < face_pos+64 && event.mouseButton.x > face_pos && event.mouseButton.y > row * 32 && event.mouseButton.y < (row * 32 + 64)) {
                        cout << "Start new game!" << endl;
                        readConfigFile("boards/config.cfg", col, row, numMines);
                        numFlags = 0;
                        tilesRemaining = col * row;
                        initialize = true;
                        revealMines = false;
                        gameOver = false;
                        gameLost = false;
                        flagMines = false;
                        faceFile = "face_happy";
                        tiles.clear();
                        boardData.clear();
                        boardData.resize(row * col);
                        initializeBoardData(boardData, row, col, numMines);
                        numMines = 0;
                    }

                    else if (!gameOver && event.mouseButton.x <test_pos && event.mouseButton.x > test_pos-64 && event.mouseButton.y > row * 32 && event.mouseButton.y < (row * 32 + 64)) {
                        cout << "Debug mode selected!" << endl;
                        revealMines = !revealMines;
                    }
                    //if new board is selected, REMEMBER TO SET INITIALIZE BACK TO TRUE AND NUMMINES to 0
                    else if (event.mouseButton.x <test_pos2 && event.mouseButton.x > test_pos && event.mouseButton.y > row * 32 && event.mouseButton.y < (row*32 + 64)) {
                        cout << "Test Board 1 selected!" << endl;
                        boardFile = "boards/testboard1.brd";
                        numMines = 0;
                        numFlags = 0;
                        tilesRemaining = col * row;
                        initialize = true;
                        revealMines = false;
                        flagMines = false;
                        gameOver = false;
                        gameLost = false;
                        faceFile = "face_happy";
                        tiles.clear();
                        boardData.clear();
                        boardData.resize(row * col);
                        readBoardFile(boardFile, boardData, row, col);
                        initializeBoardData(boardData, row, col, numMines);
                    }
                    else if (event.mouseButton.x <test_pos3 && event.mouseButton.x > test_pos2 && event.mouseButton.y > row * 32 && event.mouseButton.y < (row * 32 + 64)) {
                        cout << "Test Board 2 selected!" << endl;
                        boardFile = "boards/testboard2.brd";
                        numMines = 0;
                        numFlags = 0;
                        tilesRemaining = col * row;
                        initialize = true;
                        revealMines = false;
                        flagMines = false;
                        gameOver = false;
                        gameLost = false;
                        faceFile = "face_happy";
                        tiles.clear();
                        boardData.clear();
                        boardData.resize(row * col);
                        readBoardFile(boardFile, boardData, row, col);
                        initializeBoardData(boardData, row, col, numMines);
                    }
                    else if (event.mouseButton.x <test_pos3+64 && event.mouseButton.x > test_pos3 && event.mouseButton.y > row * 32 && event.mouseButton.y < (row * 32 + 64)) {
                        cout << "Test Board 3 selected!" << endl;
                        boardFile = "boards/testboard3.brd";
                        numMines = 0;
                        numFlags = 0;
                        tilesRemaining = col * row;
                        initialize = true;
                        revealMines = false;
                        flagMines = false;
                        gameOver = false;
                        gameLost = false;
                        faceFile = "face_happy";
                        tiles.clear();
                        boardData.clear();
                        boardData.resize(row* col);
                        readBoardFile(boardFile, boardData, row, col);
                        initializeBoardData(boardData, row, col, numMines);
                    }
                }
            }
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        //INITIALIZE BOARD FROM HERE...
        window.clear(sf::Color(255, 255, 255, 0));

        //Iterates through each tile, initializing it if necessary, otherwise updating it
        for (int c = 0; c < col; c++) {
            for (int r = 0; r < row; r++) {
                string colNum = to_string(c);
                string rowNum = to_string(r);
                string mapString = colNum + "_" + rowNum;
                if (initialize) {
                    Tile t(c * 32, r * 32);
                    if (boardData[r].at(c) == ('1')) {
                        t.setRevealedName("mine");
                        numMines++;
                    }
                    else {
                        t.setRevealedName("tile_revealed");
                    }

                    //map key will be c_r
                    //When clicking, the window location 90, 20 will divide each by 32 to get tile # for col_row (2_0)
                    //To access, get int of clicked, convert each to string to_string(int) and use as map key for access
                    tiles.emplace(mapString, t);

                    updateSprite(tile, t, sprites, window, gameLost);
                }
                else {
                    if (flagMines && tiles[mapString].revealedSpriteName == "mine")
                        tiles[mapString].flagged = true;
                    else if (revealMines && tiles[mapString].revealedSpriteName == "mine")
                        tiles[mapString].debugReveal = true;
                    else if (!revealMines && tiles[mapString].revealedSpriteName == "mine")
                        tiles[mapString].debugReveal = false;
                    if (gameLost && tiles[mapString].revealedSpriteName == "mine")
                        tiles[mapString].flagged = false;
                    updateSprite(tile, tiles[mapString], sprites, window, gameLost);
                }
            }
        }

        //After initializing, find number of mines that border each tile
        if (initialize) {
            int prevCol;
            int nextCol;
            int prevRow;
            int nextRow;
            string colNum;
            string rowNum;
            string mapString;

            for (int c = 0; c < col; c++) {
                for (int r = 0; r < row; r++) {
                    if (boardData[r].at(c) == ('1')) {
                        prevCol = c - 1;
                        nextCol = c + 1;
                        prevRow = r - 1;
                        nextRow = r + 1;
                        //these 3 are the boxes in the column left of the mine
                        if (prevCol >= 0 && prevRow >=0) {
                            colNum = to_string(prevCol);
                            rowNum = to_string(prevRow);
                            mapString = colNum + "_" + rowNum;
                            tiles[mapString].increaseMineCount();
                        }
                        if (prevCol >= 0) {
                            colNum = to_string(prevCol);
                            rowNum = to_string(r);
                            mapString = colNum + "_" + rowNum;
                            tiles[mapString].increaseMineCount();
                        }
                        if (prevCol >= 0 && nextRow < row) {
                            colNum = to_string(prevCol);
                            rowNum = to_string(nextRow);
                            mapString = colNum + "_" + rowNum;
                            tiles[mapString].increaseMineCount();
                        }

                        //these 2 are the boxes above and below the mine
                        if (prevRow >= 0) {
                            colNum = to_string(c);
                            rowNum = to_string(prevRow);
                            mapString = colNum + "_" + rowNum;
                            tiles[mapString].increaseMineCount();
                        }
                        if (nextRow < row) {
                            colNum = to_string(c);
                            rowNum = to_string(nextRow);
                            mapString = colNum + "_" + rowNum;
                            tiles[mapString].increaseMineCount();
                        }
                        
                        //these 3 are the boxes in the column to the right of the mine
                        if (nextCol < col && nextRow < row) {
                            colNum = to_string(nextCol);
                            rowNum = to_string(nextRow);
                            mapString = colNum + "_" + rowNum;
                            tiles[mapString].increaseMineCount();
                        }
                        if (nextCol < col) {
                            colNum = to_string(nextCol);
                            rowNum = to_string(r);
                            mapString = colNum + "_" + rowNum;
                            tiles[mapString].increaseMineCount();
                        }
                        if (nextCol < col && prevRow >= 0) {
                            colNum = to_string(nextCol);
                            rowNum = to_string(prevRow);
                            mapString = colNum + "_" + rowNum;
                            tiles[mapString].increaseMineCount();
                        }
                    }
                }
            }

            //Iterate through whole map and set revealedSpriteName to mineCount if tile is not a mine and mineCount > 0
            for (int c = 0; c < col; c++) {
                for (int r = 0; r < row; r++) {
                    colNum = to_string(c);
                    rowNum = to_string(r);
                    mapString = colNum + "_" + rowNum;
                    if (tiles[mapString].numMineNeighbors == 1 && tiles[mapString].revealedSpriteName != "mine") {
                        tiles[mapString].setRevealedName("number_1");
                    }
                    else if (tiles[mapString].numMineNeighbors == 2 && tiles[mapString].revealedSpriteName != "mine") {
                        tiles[mapString].setRevealedName("number_2");
                    }
                    else if (tiles[mapString].numMineNeighbors == 3 && tiles[mapString].revealedSpriteName != "mine") {
                        tiles[mapString].setRevealedName("number_3");
                    }
                    else if (tiles[mapString].numMineNeighbors == 4 && tiles[mapString].revealedSpriteName != "mine") {
                        tiles[mapString].setRevealedName("number_4");
                    }
                    else if (tiles[mapString].numMineNeighbors == 5 && tiles[mapString].revealedSpriteName != "mine") {
                        tiles[mapString].setRevealedName("number_5");
                    }
                    else if (tiles[mapString].numMineNeighbors == 6 && tiles[mapString].revealedSpriteName != "mine") {
                        tiles[mapString].setRevealedName("number_6");
                    }
                    else if (tiles[mapString].numMineNeighbors == 7 && tiles[mapString].revealedSpriteName != "mine") {
                        tiles[mapString].setRevealedName("number_7");
                    }
                    else if (tiles[mapString].numMineNeighbors == 8 && tiles[mapString].revealedSpriteName != "mine") {
                        tiles[mapString].setRevealedName("number_8");
                    }
                }
            }
        }
        initialize = false;

        //Draw the three digits (number of mines) in bottom left corner
        int digits[3];
        bool negative;
        if (numMines >= numFlags) {
            mineTotalDisplayed = numMines - numFlags;
            negative = false;
        }
        else {
            mineTotalDisplayed = numFlags - numMines;
            negative = true;
        }
        updateDigits(mineTotalDisplayed, digits);
        //cout << "TEST UPDATE DIGITS: " << digits[0] << digits[1] << digits[2] << endl;

        //Set sprite[digits] to the number for each digit in the int array and print them in order
        for (int d = 0; d < 3; d++) {
            if (negative && d == 0) {
                sprites["digits"].setTextureRect(sf::IntRect(210, 0, 21, 32));
                d--;
                negative = false;
            }
            else {
                if (digits[d] == 0)
                    sprites["digits"].setTextureRect(sf::IntRect(0, 0, 21, 32));
                else if (digits[d] == 1)
                    sprites["digits"].setTextureRect(sf::IntRect(21, 0, 21, 32));
                else if (digits[d] == 2)
                    sprites["digits"].setTextureRect(sf::IntRect(42, 0, 21, 32));
                else if (digits[d] == 3)
                    sprites["digits"].setTextureRect(sf::IntRect(63, 0, 21, 32));
                else if (digits[d] == 4)
                    sprites["digits"].setTextureRect(sf::IntRect(84, 0, 21, 32));
                else if (digits[d] == 5)
                    sprites["digits"].setTextureRect(sf::IntRect(105, 0, 21, 32));
                else if (digits[d] == 6)
                    sprites["digits"].setTextureRect(sf::IntRect(126, 0, 21, 32));
                else if (digits[d] == 7)
                    sprites["digits"].setTextureRect(sf::IntRect(147, 0, 21, 32));
                else if (digits[d] == 8)
                    sprites["digits"].setTextureRect(sf::IntRect(168, 0, 21, 32));
                else if (digits[d] == 9)
                    sprites["digits"].setTextureRect(sf::IntRect(189, 0, 21, 32));
            }

            int digit_pos = 21 * (d + 1);
            sprites["digits"].setPosition(digit_pos, row * 32);
            window.draw(sprites["digits"]);
        }

        face_pos = (col) * 32 / 2 - 32;
        sprites[faceFile].setPosition(face_pos, row * 32);
        window.draw(sprites[faceFile]);

        int debug_pos = face_pos + 128;
        sprites["debug"].setPosition(debug_pos, row * 32);
        window.draw(sprites["debug"]);

        test_pos = debug_pos + 64;
        sprites["test_1"].setPosition(test_pos, row * 32);
        window.draw(sprites["test_1"]);
        test_pos2 = test_pos + 64;
        sprites["test_2"].setPosition(test_pos2, row * 32);
        window.draw(sprites["test_2"]);
        test_pos3 = test_pos2 + 64;
        sprites["test_3"].setPosition(test_pos3, row * 32);
        window.draw(sprites["test_3"]);
        //... TO HERE (END OF BOARD INITILIZATION)

        window.display();

        
    }

	return 0;
}


