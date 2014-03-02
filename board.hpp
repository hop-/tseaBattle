#ifndef _BOARD_HPP_
#define _BOARD_HPP_

class Board{
        private:
                bool board[10][10];
		int ships[10];
                int currentShipIndex;
        public:
                Board();
                int getCurrentShipSize();
                bool possibleToPlace(int x, int y, bool isHorizontal);
                bool place(int x, int y, bool isHorizontal);
                bool placeComplete();
                int shot(int x, int y);
                void getBoard(bool b[][10]);
                void reset();
};

#endif
