#include "board.hpp"
#include <iostream>

Board::Board(){
	ships[0] = 4;
	ships[1] = 3;
	ships[2] = 3;
	ships[3] = 2;
	ships[4] = 2;
	ships[5] = 2;
	ships[6] = 1;
	ships[7] = 1;
	ships[8] = 1;
	ships[9] = 1;
        reset();
}

bool Board::possibleToPlace(int x, int y, bool isHorizontal){
        int xx, yy;
        if(placeComplete())
                return false;
        if((isHorizontal && x-1+ships[currentShipIndex] > 9)||(!isHorizontal && y-1+ships[currentShipIndex] > 9))
                return false;
        for(int i = -1; i < 2; i++)
                for(int j = -1; j <= ships[currentShipIndex]; j++){
                        if(isHorizontal){
                                xx = x + j;
                                yy = y + i;
                        }else{
                                xx = x + i;
                                yy = y + j;
                        }
                        if(xx > 9 || xx < 0 || yy > 9 || yy < 0)
                                continue;
                        else if(board[xx][yy])
                                return false;
                }
        return true;
}

int Board::getCurrentShipSize(){
        return ships[currentShipIndex];
}

bool Board::place(int x, int y, bool isHorizontal){
        if(!possibleToPlace(x,y,isHorizontal))
                return false;
        for(int i = 0; i < ships[currentShipIndex]; i++){
                if(isHorizontal)
                        board[x+i][y] = true;
                else
                        board[x][y+i] = true;
        }
        currentShipIndex++;
        return true;
}

bool Board::placeComplete(){
        if(currentShipIndex > 9)
                return true;
        else
                return false;
}

int Board::shot(int x, int y){
        return board[x][y];
}

void Board::getBoard(bool b[][10]){
        for(int i = 0; i < 10; i++)
                for(int j = 0; j < 10; j++)
                        b[i][j] = board[i][j];
}

void Board::reset(){
        currentShipIndex = 0;
        for(bool* it = &board[0][0]; it != &board[9][10]; it++){
                *it = 0;
        }
        
}
