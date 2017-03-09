#include <ncurses.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define SPEED 15000

int num_arr[4][4] = {0,};	//Game number array
int undo_arr[4][4] = {0,};	//Array to Undo

WINDOW * block_arr[4][4];	//Block Window array (1 Block == 1 WINDOW)

void draw_map();		//Draw game board
void print_number(WINDOW *, int);	//print current block's number

int get_num(int, int);		//get separated number
int get_level(int);		//get number's position level

void get_move();		//get next keyboard move
char getkey();			//get character key to get_move()

void up_move(int(*arr)[4], int);	//move blocks up
void down_move(int(*arr)[4], int);	//move blocks down
void left_move(int(*arr)[4], int);	//move blocks left
void right_move(int(*arr)[4], int);	//move blocks right

int can_move(char);			//check blocks can move
int can_move_gameover();		//check gameover

void make_random_new_number();		//make new number(2 or 4) on random position
void make_undo_array();			//make Undo Array to undo
void undo();				//Undo move

int get_highest_number();		//get highest number on blocks
void print_cheers();			//print cheers depending highest number on game board
void print_help();			//print key help
int print_score();			//print score and highscore
void print_title();			//print game title at top
void print_all();			//execute all print functions

void restart();				//restart the game

int score=0;
int prev_score=0;
int made_score=0;
int highest_number;
int highscore=0;

int is_gameover=0;

WINDOW * score_win;
WINDOW * cheers_win;
WINDOW * help_win;
WINDOW * title_win;
WINDOW * highscore_win;

FILE * highscore_file;

int main(void){
	srand(time(NULL));

	initscr();
	noecho();
	start_color();

	//initialize colors
	init_color(1, 700,700,700);	//1
	init_color(2, 1000,1000,600);	//2
	init_color(3, 1000,1000,0);	//4
	init_color(4, 1000,800,200);	//8
	init_color(5, 1000,400,400);	//16
	init_color(6, 1000,200,200);	//32
	init_color(7, 1000,0,400);	//64
	init_color(8, 800,400,800);	//128
	init_color(9, 800,200,1000);	//256
	init_color(10, 1000,0,600);	//512
	init_color(11, 600,1000,800);	//1024
	init_color(12, 400,1000,400);	//2048
	init_color(14, 200,0,800);
	init_color(15, 0,1000,1000);
	init_color(16, 400, 200, 600);
	init_color(17, 1000,800,1000);
	init_color(18, 0,400,0);
	init_color(19, 0,1000,0);
	init_color(20,600,1000,800);
	init_color(13, 0, 0, 1000);	//number color : BLUE

	//number character color pair set
	init_pair(1, 13, 2);	//2 block number character color
	init_pair(2, 13, 3);	//4 block number character color
	init_pair(3, 13, 4);	//8
	init_pair(4, 13, 5);	//16
	init_pair(5, 13, 6);	//32
	init_pair(6, 13, 7);	//64
	init_pair(7, 13, 8);	//128
	init_pair(8, 13, 9);	//256
	init_pair(9, 13, 10);	//512
	init_pair(10, 13, 11);	//1024
	init_pair(11, 13, 12);	//2048

	//block main color pair set
	init_pair(12, 1, 1);	//1
	init_pair(13, 2, 2);	//2
	init_pair(14, 3, 3);	//4
	init_pair(15, 4, 4);	//8
	init_pair(16, 5, 5);	//16
	init_pair(17, 6, 6);	//32
	init_pair(18, 7, 7);	//64
	init_pair(19, 8, 8);	//128
	init_pair(20, 9, 9);	//256
	init_pair(21, 10, 10);	//512
	init_pair(22, 11, 11);	//1024
	init_pair(23, 12, 12);	//2048

	//utility window color pair set
	init_pair(24, 15, 14);	//score window
	init_pair(25, 17, 16);	//cheers window
	init_pair(26, 19, 18);	//help window
	init_pair(27, 20, COLOR_BLACK);
	init_pair(1, COLOR_BLACK, COLOR_BLACK);

	//get highscore
	highscore_file = fopen("./highscore.txt", "r");
	fscanf(highscore_file, "%d", &highscore);
	fclose(highscore_file);

	//game start
	draw_map();	//initialize map

	make_random_new_number();
	make_random_new_number();
	draw_map();

	print_all();

	make_undo_array();

	while(1){
		print_all();
		if(can_move_gameover()){
			//can move anywhere
			get_move();
			draw_map();
			print_score();
			if(get_highest_number()==2048){
				mvwprintw(cheers_win, 2, 3, "CONGULATULATIONS! YOU MADE 2048 BLOCK!\n\nPress any key to restart the game.");
				print_all();
				if(score > highscore){
					highscore_file = fopen("./highscore.txt", "w");
					highscore = score;
					fprintf(highscore_file, "%d", highscore);
					fclose(highscore_file);
				}
				break;
			}else
				print_cheers();
		}
		else{
			//can't move anywhere
			werase(cheers_win);
			box(cheers_win, '|', '*');
			mvwprintw(cheers_win,2, 3,  "GAME OVER\n");
			refresh();
			wrefresh(cheers_win);

			is_gameover=1;
			get_move();
		}
	}

	getch();

	endwin();
}	

void draw_map(){
	const int B_HEIGHT = 5;
	const int B_WIDTH = 10;
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			block_arr[i][j] = newwin(B_HEIGHT, B_WIDTH, 5 + (i*B_HEIGHT)+i, 5 + (j*B_WIDTH)+j+j);
			box(block_arr[i][j], '|', '-');
			switch(num_arr[i][j]){
				case 0:
					wbkgd(block_arr[i][j], COLOR_PAIR(12));
					wbkgd(stdscr, COLOR_PAIR(1));
					break;
				case 2:
					wbkgd(block_arr[i][j], COLOR_PAIR(13));
					break;
				case 4:
					wbkgd(block_arr[i][j], COLOR_PAIR(14));
					break;
				case 8:
					wbkgd(block_arr[i][j], COLOR_PAIR(15));
					break;
				case 16:
					wbkgd(block_arr[i][j], COLOR_PAIR(16));
					break;
				case 32:
					wbkgd(block_arr[i][j], COLOR_PAIR(17));
					break;
				case 64:
					wbkgd(block_arr[i][j], COLOR_PAIR(18));
					break;
				case 128:
					wbkgd(block_arr[i][j], COLOR_PAIR(19));
					break;
				case 256:
					wbkgd(block_arr[i][j], COLOR_PAIR(20));
					break;
				case 512:
					wbkgd(block_arr[i][j], COLOR_PAIR(21));
					break;
				case 1024:
					wbkgd(block_arr[i][j], COLOR_PAIR(22));
					break;
				case 2048:
					wbkgd(block_arr[i][j], COLOR_PAIR(23));
					break;
				default:
					break;
			}

		}
	}

	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			if( num_arr[i][j] )
				print_number(block_arr[i][j], num_arr[i][j]);
		}
	}

	refresh();
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			touchwin(block_arr[i][j]);
			wrefresh(block_arr[i][j]);
		}
	}	

	wrefresh(stdscr);
}

void print_number(WINDOW * block, int number){
	//It can't print formatted number like wprintw()...
	//so print seperated number
	switch(number){
		case 2:
			wmove(block, 2, 5);
			waddch(block, number+'0' | COLOR_PAIR(2));
			break;
		case 4:
			wmove(block, 2, 5);
			waddch(block, number+'0' | COLOR_PAIR(2));
			break;
		case 8:
			wmove(block, 2, 5);
			waddch(block, number+'0' | COLOR_PAIR(3));
			break;
		case 16:
			wmove(block, 2, 4);
			for(int i=2;i>=1;i--)
				waddch(block, get_num(number, i)+'0' | COLOR_PAIR(4));
			break;
		case 32:
			wmove(block, 2, 4);
			for(int i=2;i>=1;i--)
				waddch(block, get_num(number, i)+'0' | COLOR_PAIR(5));
			break;
		case 64:
			wmove(block, 2, 4);
			for(int i=2;i>=1;i--)
				waddch(block, get_num(number, i)+'0' | COLOR_PAIR(6));
			break;
		case 128:
			wmove(block, 2, 4);
			for(int i=3;i>=1;i--)
				waddch(block, get_num(number, i)+'0' | COLOR_PAIR(7));
			break;
		case 256:
			wmove(block, 2, 4);
			for(int i=3;i>=1;i--)
				waddch(block, get_num(number, i)+'0' | COLOR_PAIR(8));
			break;
		case 512:
			wmove(block, 2, 4);
			for(int i=3;i>=1;i--)
				waddch(block, get_num(number, i)+'0' | COLOR_PAIR(9));
			break;
		case 1024:
			wmove(block, 2, 3);
			for(int i=4;i>=1;i--)
				waddch(block, get_num(number, i)+'0' | COLOR_PAIR(10));
			break;
		case 2048:
			wmove(block, 2, 3);
			for(int i=4;i>=1;i--)
				waddch(block, get_num(number, i)+'0' | COLOR_PAIR(11));
			break;
	}
}

int get_num(int number, int level){
	return (int)(number % (int)pow(10, level))/(int)(pow(10,level-1));
}

int get_level(int number){
	if(number>1000)
		return 4;
	else if(number>100)
		return 3;
	else if(number>10)
		return 2;
	else
		return 1;
}

void get_move(){
	switch(getkey()){
		case 'A':
			//up arrow
			if(!is_gameover){
				prev_score = score;
				if(can_move('A')){
					make_undo_array();
					up_move(num_arr, 0);
				}
			}
			break;
		case 'B':
			//down arrow
			if(!is_gameover){
				prev_score = score;
				if(can_move('B')){
					make_undo_array();
					down_move(num_arr, 0);
				}
			}
			break;
		case 'C':
			//right arrow
			if(!is_gameover){
				prev_score = score;
				if(can_move('C')){
					make_undo_array();
					right_move(num_arr, 0);
				}
			}
			break;
		case 'D':
			//left arrow
			if(!is_gameover){
				prev_score = score;
				if(can_move('D')){
					make_undo_array();
					left_move(num_arr, 0);
				}
			}
			break;
		case 'r':
			//restart the game
			restart();
			break;
		case 'u':
			//undo
			made_score = score - prev_score;
			score -= made_score;
			undo();
			draw_map();
			print_all();
			is_gameover=0;
			break;
		default:
			if( is_gameover )
				restart();
			break;
	}
}

void restart(){
	//number array reset
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			num_arr[i][j] = 0;
		}
	}

	make_random_new_number();
	make_random_new_number();

	make_undo_array();
	draw_map();

	if(score > highscore){
		//score save bug fix
		highscore_file = fopen("./highscore.txt", "w");
		highscore = score;
		fprintf(highscore_file, "%d", score);
		fclose(highscore_file);
	}

	score=0;
	prev_score=0;
	made_score=0;

	is_gameover=0;
	print_all();
}

char getkey(){
	char ch = getch();

	//arrow key has 3 characters
	if(ch == '\033'){
		getch();
		return getch();
	}else if(ch=='u')
		return 'u';
	else if(ch=='r')
		return 'r';
}

void up_move(int(*arr)[4], int is_testing){
	int y;

	//push blocks to wall
	for(int k=0;k<4;k++){
		for(int x=0;x<4;x++){
			y=0;
			while(y<3){
				if(arr[y][x] == 0 && arr[y+1][x] != 0){
					arr[y][x] = arr[y+1][x];
					arr[y+1][x]=0;
				}
				y++;
			}
		}
		if( !is_testing ){
			print_all();
			usleep(SPEED);
			draw_map();
		}
	}

	//move and combine blocks
	for(int x=0;x<4;x++){
		y=0;
		while(y<3){
			if(arr[y][x] != arr[y+1][x])
				y++;
			else{
				arr[y][x] += arr[y+1][x];
				if(!is_testing)
					score+=arr[y][x];
				arr[y+1][x] = 0;

				for(int i=y+1;i<3;i++){
					arr[i][x] = arr[i+1][x];
					arr[i+1][x] = 0;
				}

				y++;
			}
		}
	}

	if( !is_testing )
		make_random_new_number();
}

void down_move(int(*arr)[4], int is_testing){
	int y;

	//push blocks to wall
	for (int k=0;k<4;k++) {
		for (int x=0;x<4;x++) {
			y=3;
			while (y>0) {
				if (arr[y][x] == 0 && arr[y-1][x] != 0) {
					arr[y][x] = arr[y-1][x];
					arr[y-1][x] = 0;
				}
				y--;
			}
		}
		if( !is_testing ){
			print_all();
			usleep(SPEED);
			draw_map();
		}
	}

	//move and combine blocks
	for (int x=0;x<4;x++) {
		y=3;
		while (y>0) {
			if (arr[y][x] != arr[y-1][x])
				y--;
			else {
				arr[y][x] += arr[y-1][x];
				if(!is_testing)
					score += arr[y][x];
				arr[y-1][x] = 0;

				for (int i = y - 1; i > 0; i--) {
					arr[i][x] = arr[i-1][x];
					arr[i-1][x] = 0;
				}

				y--;
			}
		}
	}

	if(!is_testing)
		make_random_new_number();
}

void right_move(int(*arr)[4], int is_testing){
	int x;

	//push blocks to wall
	for (int k=0;k<4;k++) {
		for (int y=0;y<4;y++) {
			x=3;
			while (x>0) {
				if (arr[y][x] == 0 && arr[y][x-1] != 0) {
					arr[y][x] = arr[y][x-1];
					arr[y][x-1] = 0;
				}
				x--;
			}
		}
		if( !is_testing ){
			print_all();
			usleep(SPEED);
			draw_map();
		}
	}

	//move and combine blocks
	for (int y=0;y<4;y++) {
		x=3;
		while (x>0) {
			if (arr[y][x] != arr[y][x-1])
				x--;
			else {
				arr[y][x] += arr[y][x-1];
				if(!is_testing)
					score += arr[y][x];
				arr[y][x-1] = 0;

				for (int i=x-1;i>0;i--) {
					arr[y][i] = arr[y][i-1];
					arr[y][i-1] = 0;
				}

				x--;
			}
		}
	}

	if(!is_testing)
		make_random_new_number();
}

void left_move(int(*arr)[4], int is_testing){
	int x;

	//push blocks to wall
	for (int k=0;k<4;k++) {
		for (int y=0;y<4;y++) {
			x=0;
			while (x<3) {
				if (arr[y][x] == 0 && arr[y][x+1] != 0) {
					arr[y][x] = arr[y][x+1];
					arr[y][x+1] = 0;
				}
				x++;
			}
		}
		if( !is_testing ){
			print_all();
			usleep(SPEED);
			draw_map();
		}
	}

	//move and combine blocks
	for (int y=0;y<4;y++) {
		x=0;
		while (x<3) {
			if (arr[y][x] != arr[y][x+1])
				x++;
			else {
				arr[y][x] += arr[y][x+1];
				if(!is_testing)
					score += arr[y][x];
				arr[y][x+1] = 0;

				for (int i=x+1;i<3;i++) {
					arr[y][i] = arr[y][i+1];
					arr[y][i+1] = 0;
				}

				x++;
			}
		}
	}

	if( !is_testing)
		make_random_new_number();
}

void make_random_new_number(){
	int unique_arr[4][4] = {0,};
	int unique_num=0;
	int rand_num;
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			if(num_arr[i][j] == 0)
				unique_arr[i][j] = ++unique_num;
		}
	}

	rand_num = (rand()%unique_num)+1;

	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			if(unique_arr[i][j] == rand_num){
				if(rand()%4 == 3)
					num_arr[i][j] = 4;
				else
					num_arr[i][j] = 2;
			}
		}
	}
}

int can_move(char r){
	int arr[4][4] = {0,};

	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++)
			arr[i][j] = num_arr[i][j];
	}

	//move simulation
	switch(r){
		case 'A':
			up_move(arr, 1);
			break;
		case 'B':
			down_move(arr, 1);
			break;
		case 'C':
			right_move(arr, 1);
			break;
		case 'D':
			left_move(arr, 1);
			break;
		default:
			break;
	}

	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			if(arr[i][j] != num_arr[i][j])
				return 1;
		}
	}

	return 0;
}

int can_move_gameover(){
	int arr[4][4] = {0,};

	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++)
			arr[i][j] = num_arr[i][j];
	}

	//All direction move possibility check
	up_move(arr, 1);
	down_move(arr, 1);
	right_move(arr,1);
	left_move(arr, 1);

	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			if(arr[i][j] != num_arr[i][j])
				return 1;
		}
	}

	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			if(num_arr[i][j] == 0)
				return 1;
		}
	}

	return 0;
}

int print_score(){
	curs_set(0);
	score_win = newwin(7,20,7,60);
	box(score_win, '|', '*');
	wbkgd(score_win, COLOR_PAIR(24));

	mvwprintw(score_win, 2, 5, "Score : %d", score);
	mvwprintw(score_win, 4, 3, "highest : %d", highscore);

	refresh();
	touchwin(score_win);
	wrefresh(score_win);
}

void make_undo_array(){
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			undo_arr[i][j] = num_arr[i][j];
		}
	}
}

void undo(){
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			num_arr[i][j] = undo_arr[i][j];
		}
	}
}

int get_highest_number(){
	int highest=0;

	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++)
			highest = highest<num_arr[i][j] ? num_arr[i][j] : highest;
	}

	return highest;
}

void print_cheers(){
	static int is_first=1;

	if(is_first){
		cheers_win = newwin(5,46,30,5);
		box(cheers_win, '|', '*');
		wbkgd(cheers_win, COLOR_PAIR(25));
	}

	//Cheers depending highest number
	switch( get_highest_number() ){
		case 128:
			mvwprintw(cheers_win,2,3, "You made 128! But it's start...");
			break;
		case 256:
			mvwprintw(cheers_win,2,3, "Wow! I see 256!");
			break;
		case 512:
			mvwprintw(cheers_win,2,3, "Is that 512, TRUE???");
			break;
		case 1024:
			mvwprintw(cheers_win,2,3, "1024! you ALMOST DONE!");
			break;
		default:
			mvwprintw(cheers_win,2,3, "Fighting to make 2048!");
			break;
	}

	refresh();
	touchwin(cheers_win);
	wrefresh(cheers_win);
}

void print_help(){
	help_win = newwin(9, 20, 14, 60);
	box(help_win, ' ', ' ');
	wbkgd(help_win, COLOR_PAIR(26));
	mvwprintw(help_win, 1, 1, " Arrow key : Move");
	mvwprintw(help_win, 4, 1, " u : Undo");
	mvwprintw(help_win, 7, 1, " r : Restart Game");

	refresh();
	touchwin(help_win);
	wrefresh(help_win);
}

void print_title(){
	title_win = newwin(3, 13, 1, 23);
	wbkgd(title_win, COLOR_PAIR(27));
	box(title_win, ' ', ' ');
	mvwprintw(title_win, 1, 2,  "2048 GAME");

	refresh();
	touchwin(title_win);
	wrefresh(title_win);
}

void print_all(){
	print_title();
	print_score();
	print_help();
	print_cheers();
}

