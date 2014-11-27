#include "board.hpp"
#include "clientserver.hpp"

#include <string>
#include <curses.h>
#include <stdlib.h>
#include <iostream>

#define PNAME "TSB"
#define READY "I'm Ready"
#define SHIP_SEGMENT '#'
#define MEXIT "exit!!!"
#define YES "y"
#define NO "n"

static const char* usage = "Usage:\n\ttseaBattle [address]";

enum Colors {
        NONE = 1,
        CURRENT = 2,
        MISS = 3,
        HIT = 4
};

void initCurses(){
	initscr();
        start_color();
        init_pair(NONE, COLOR_WHITE, COLOR_BLACK);
        init_pair(CURRENT, COLOR_GREEN, COLOR_BLACK);
        init_pair(MISS, COLOR_BLUE, COLOR_BLACK);
        init_pair(HIT, COLOR_RED, COLOR_BLACK);
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
}

void drawPlacement(Board* b, int x, int y, bool h){
        bool board[10][10];
        b->getBoard(board);
        int shipSize = b->getCurrentShipSize();
	const char* r = "+-+-+-+-+-+-+-+-+-+-+\n";
	const char n = '\n';
	const char w = '|';
	mvaddstr(0, 0 , r);
	for(int i = 0; i < 10; i++){
                addch(w);
		for(int j = 0; j < 10; j++){
                        if((h && y == i && j >= x && j < x+shipSize)||(!h && x == j && i >= y && i < y+shipSize)){
                                attron(COLOR_PAIR(CURRENT));
                                addch(SHIP_SEGMENT);
                                attron(COLOR_PAIR(NONE));
                        } else if(board[j][i]) {
                                addch(SHIP_SEGMENT);
                        } else {
                                addch(' ');
                        }
                addch(' ');
		}
                addch('\b');
		addch(w);
                addch(n);
        }
        addstr(r);
}

void drawGame(Board* b, int x, int y, bool l[][10], int enemyBoard[][10], bool me){
        bool board[10][10];
        b->getBoard(board);
	const char* r = "+-+-+-+-+-+-+-+-+-+-+";
	const char n = '\n';
	const char w = '|';
	const char* p = "   ";
	mvaddstr(0, 0 , r);
	addstr(p);
	addstr(r);
	addch(n);
	for(int i = 0; i < 10; i++){
                addch(w);
		for(int j = 0; j < 10; j++){
			if(l[j][i] && board[j][i]) {
                                attron(COLOR_PAIR(HIT));
                                addch('X');
                                attron(COLOR_PAIR(NONE));
                        }
                        else if(l[j][i]) {
                                attron(COLOR_PAIR(MISS));
                                addch('*');
                                attron(COLOR_PAIR(NONE));
                        }
                        else if(board[j][i])
                                addch('#');
                        else
                                addch(' ');
			addch(' ');
		}
                addch('\b');
		addch(w);
		addstr(p);
		addch(w);
		for(int j = 0; j < 10; j++){
                        if(x == j && y == i) {
                                attron(COLOR_PAIR(CURRENT));
			        addch('+');
                                attron(COLOR_PAIR(NONE));
                        }
                        else if(enemyBoard[j][i] > 0) {
                                attron(COLOR_PAIR(HIT));
                                addch('X');
                                attron(COLOR_PAIR(NONE));
                        }
                        else if(enemyBoard[j][i] < 0) {
                                attron(COLOR_PAIR(MISS));
                                addch('*');
                                attron(COLOR_PAIR(NONE));
                        }
                        else
                                addch(' ');
			addch(' ');
		}
                addch('\b');
                addch(w);
                addch(n);
	}
        addstr(r);
        addstr(p);
        addstr(r);
        addch(n);
        if(me)
                addstr("yes");
        else
                addstr("no ");
}

int main(int argc, char* argv[]){
	Board* b = new Board();
	int c;
        int hitCount = 0;
        int enemyHits = 0;
        int x = 0;
        int y = 0;
        bool h = false;
        bool shboard[10][10] = {false};
        int enemyB[10][10] = {0};
        bool me;
        std::string msg;

        //b->place(3,3,1);
        
        TCPBase* connection;

        if(argc == 1){
                connection = new TCPServer(PNAME);
                me = true;
        }else if(argc == 2){
                connection = new TCPClient(argv[1], PNAME);
                me = false;
        }else{
                std::cout<<usage<<std::endl;
                return -1;
        }
        std::cout<<"Waiting for connection..."<<std::endl;;
        if(!connection->start()){
                std::cout<<"Can't create connection, please try later"<<std::endl;
                return -1;
        }
        std::cout<<"Done"<<std::endl;
        initCurses();
        while(c != 27 && !b->placeComplete()){
                drawPlacement(b,x,y,h);
                c = getch();
                switch(c){
                        case KEY_LEFT:
                                x--;
                                if(x < 0)
                                        x = 0;
                                break;
                        case KEY_RIGHT:
                                x++;
                                if(x > 9 || (h && x + b->getCurrentShipSize() > 10))
                                        x--;
                                break;
                        case KEY_UP:
                                y--;
                                if(y < 0)
                                        y = 0;
                                break;
                        case KEY_DOWN:
                                y++;
                                if(y > 9 || (!h && y + b->getCurrentShipSize() > 10))
                                        y--;
                                break;
                        case 9:
                        case 32:
                                h=!h;
                                break;
                        case 82:
                                b->reset();
                                 x = 0;
                                 y = 0;
                                break;
                        case 13:
                                if(b->place(x, y, h)){
                                     //  x = 0;
                                     //  y = 0;
                                }
                                break;
                        default:
                                break;
                }
        }
        if(c != 27){
                addstr("\nwaiting...\n");
                refresh();
                connection->send(READY);
                msg = connection->breceive();
                clear();
                if(msg == READY)
                        while(c != 27 && hitCount != 20 && enemyHits != 20){
                                drawGame(b, x, y, shboard, enemyB, me);
                                refresh();
                                if(!me){
                                        msg = connection->breceive();
                                        if(msg == MEXIT)
                                                break;
                                        int sx, sy;
                                        sx = msg[0] - '0';
                                        sy = msg[1] - '0';
                                        if(b->shot(sx, sy)){
                                                connection->send(YES);
                                                if(!shboard[sx][sy]){
                                                        enemyHits++;
                                                }
                                        }else{
                                                connection->send(NO);
                                                me = true;
                                        }
                                        shboard[sx][sy] = true;
                                }else{
                                        c = getch();
                                        if(c == 27)
                                                connection->send(MEXIT);
                                        switch(c){
                                                case KEY_LEFT:
                                                        x--;
                                                        if(x < 0)
                                                                x = 0;
                                                        break;
                                                case KEY_RIGHT:
                                                        x++;
                                                        if(x > 9)
                                                                x = 9;
                                                        break;
                                                case KEY_UP:
                                                        y--;
                                                        if(y < 0)
                                                                y = 0;
                                                        break;
                                                case KEY_DOWN:
                                                        y++;
                                                        if(y > 9)
                                                                y = 9;
                                                        break;
                                                case 13:
                                                        msg = char('0' + x);
                                                        msg = msg + char('0' + y);
                                                        connection->send(msg);
                                                        msg = connection->breceive();
                                                        if(msg == NO){
                                                                enemyB[x][y] = -1;
                                                                me = false;
                                                        } else if(enemyB[x][y] != 1){
                                                                hitCount++;
                                                                enemyB[x][y] = 1;
                                                        }
                                                        break;
                                                default:
                                                        break;
                                        }
                                }
                        }
        }
        endwin();
        delete(connection);
        delete(b);
	return 0;
}
