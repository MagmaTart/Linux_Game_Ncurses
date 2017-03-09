#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void draw_numbers(WINDOW *, int, int, int, int, int);	//draw all board numbers
void draw_current_memo(int, int, int, int);		//draw current cell's memo
void draw_select_window(int);				//draw difficulty select window
void * draw_time(void *);				//draw time window on thread

void do_memo(int, int, int, int);			//do memo

void recall_cursor(int, int);				//recall cursor to Game board
void m_recall_cursor(int, int);				//recall cursor to Game board from memo window

char getkey();		//get key character
int get_x_coord(int);	//get number's array x coordinate from cursor
int get_y_coord(int);	//get number's array y coordinate from cursor

int check_x_correct(int);		//check correct in that row
int check_y_correct(int);		//check correct in that column
int check_now_block_correct(int, int);	//check correct in that block
int check_blocks_correct();		//check correct in all blocks
int check_correct();			//check corrent in Game board

int count_blanks();		//count the blanks in Game board
void set_original_question();	//set original_question array
void set_array_zero();		//set original_question array to 0
void set_memo_zero();		//set memo_arr array to 0

int num_arr[9][9];			//game number array
int original_question[9][9] = {0,};	//fixed number array
int memo_arr[9][9][2][2] = {0,};	//cell's memo number array

int isFirst=1;
int is_user_complete = 0;	//check user complete game

int main_cur_x;		//cursor coordinate on main game board
int main_cur_y;
int memo_cur_x;		//cursor coordinate on memo board
int memo_cur_y;

int is_memo_editing=0;	//check user memo editing

int game_time=0;	//game play time

WINDOW * time_screen;
WINDOW * memo_screen;

int main(void){
	int st_x, st_y, width, height;
	int console_x, console_y;
	int blank_count=0;
	int now_sel=0;
	int select_complete=0;

	pthread_t time_thread;

	WINDOW * start_win;	//start screen window
	WINDOW * my_win;	//game board window
	WINDOW * util_win;	//blank window
	WINDOW * help_win;	//user help window
	WINDOW * user_status_win;	//Fighting window :)

	FILE * file;

	initscr();
	noecho();

MAINMENU:

	erase();

	console_y = 7;
	console_x = 30;
	start_win = newwin(console_y, console_x, 2, 12);
	box(start_win, '*', '*');

	mvwprintw(start_win, 2, 9, "SUDOKU GAME\n");
	mvwprintw(start_win, 4, 4, "Press any key to start\n");	

	refresh();
	wrefresh(start_win);

	draw_select_window(now_sel);

	while( !select_complete ){
		char n_key;
		switch(n_key = getkey()){
			case 'A':
				//arrow up
				now_sel -= now_sel==0 ? 0 : 1;
				break;
			case 'B':
				//arrow down
				now_sel += now_sel==5 ? 0 : 1;
				break;
			case 10:
				//enter
				switch(now_sel){
					//need to make stages
					//4~5 stages and random choose
					//search stage random make algorithm
					case 0:
						file = fopen("./stage1.txt", "r");
						break;
					case 1:
						file = fopen("./stage2.txt", "r");
						break;
					case 2:
						file = fopen("./stage3.txt", "r");
						break;
					case 3:
						file = fopen("./stage4.txt", "r");
						break;
					case 4:
						file = fopen("./stage5.txt", "r");
						break;
					case 5:
						erase();
						endwin();
						return 0;
					default:
						break;
				}
				select_complete = 1;
			default:
				break;
		}
		draw_select_window(now_sel);
	}

	
	start_color();
	init_color(COLOR_GREEN, 200, 1000, 200);
	init_color(COLOR_WHITE, 1000, 1000, 1000);
	init_color(COLOR_CYAN, 0, 1000, 1000);
	init_color(COLOR_YELLOW, 1000, 1000, 0);
	init_color(COLOR_BLUE, 200, 200, 1000);

	init_pair(1, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(2, COLOR_GREEN, COLOR_MAGENTA);
	init_pair(3, COLOR_CYAN, COLOR_MAGENTA);
	init_pair(4, COLOR_WHITE, COLOR_BLACK);
	init_pair(5, COLOR_YELLOW, COLOR_MAGENTA);
	init_pair(6, COLOR_CYAN, COLOR_MAGENTA);	//clear screen color
	init_pair(7, COLOR_GREEN, COLOR_BLUE);
	init_pair(8, COLOR_CYAN, COLOR_MAGENTA);
	init_pair(9, COLOR_BLUE, COLOR_YELLOW);
	init_pair(10, COLOR_BLACK, COLOR_YELLOW);
	
	now_sel=0;
	select_complete=0;
	erase();

	if(isFirst){
		isFirst=0;

		for(int i=0;i<9;i++){
			for(int j=0;j<9;j++)
				num_arr[i][j] = (int)fgetc(file)-'0';
			fgetc(file);
		}

		//fclose(file);

		set_original_question();	//fixed number initialization

		//draw board

		height = 13;
		width = 25;

		st_y = (LINES - height / 2);
		st_x = (COLS - width / 2);

		my_win = newwin(height, width, 4, 10);
		util_win = newwin(5, 15, 10, 55);
		help_win = newwin(15, 30, 17, 40);
		user_status_win = newwin(4, 30, 20, 10);

		box(my_win,'|' , '-');
		box(util_win, '|', '*');
		box(help_win, '+', '+');

	}else{
		for(int i=0;i<9;i++){
			for(int j=0;j<9;j++)
				num_arr[i][j] = (int)fgetc(file)-'0';
			fgetc(file);
		}

		set_original_question();
	}

	//game draw start

	bkgd(COLOR_PAIR(4));
	wbkgd(my_win, COLOR_PAIR(1));
	wbkgd(util_win, COLOR_PAIR(1));
	wbkgd(help_win, COLOR_PAIR(7));

	refresh();
	touchwin(my_win);
	touchwin(util_win);
	touchwin(help_win);
	touchwin(user_status_win);

	wrefresh(my_win);
	wrefresh(util_win);
	wrefresh(help_win);
	wrefresh(user_status_win);
	draw_current_memo(0,0,0,0);

	//print user help
	mvwprintw(help_win, 2, 3, "Arrow key : move cursor");
	mvwprintw(help_win, 4, 5, "1-9 : Place digit");
	mvwprintw(help_win, 6, 5, "0   : Clear digit");
	mvwprintw(help_win, 8, 5, "m   : Cell MEMO");
	mvwprintw(help_win, 10, 5, "c   : Clear board");
	mvwprintw(help_win, 12, 5, "q   : Quit game");

	wprintw(user_status_win, "Fighting!");

	refresh();
	wrefresh(help_win);
	wrefresh(user_status_win);

	wmove(my_win, 4, 0);
	wprintw(my_win, "+");
	for(int i=0;i<width-2;i++)
		wprintw(my_win, "-");
	wprintw(my_win, "+");

	wmove(my_win, 8, 0);
	wprintw(my_win, "+");
	for(int i=0;i<width-2;i++)
		wprintw(my_win, "-");
	wprintw(my_win, "+");

	wmove(my_win, 0, 8);
	wprintw(my_win, "+");
	for(int i=1;i<height-1;i++){
		wmove(my_win, i, 8);
		wprintw(my_win, "|");
	}
	wmove(my_win, height-1, 8);
	wprintw(my_win, "+");

	wmove(my_win, 0, 16);
	wprintw(my_win, "+");
	for(int i=1;i<height-1;i++){
		wmove(my_win, i, 16);
		wprintw(my_win, "|");
	}
	wmove(my_win, height-1, 16);
	wprintw(my_win, "+");

	wrefresh(my_win);

	//end - draw board

	//start - draw numbers

	draw_numbers(my_win, height, width, -1, 0, 0);

	//end - draw numbers

	//start - draw util WINDOW

	blank_count = count_blanks();	
	mvwprintw(util_win, 2, 3, "Blank : %d", blank_count);
	refresh();
	wrefresh(util_win);

	//game start
	pthread_create(&time_thread, NULL, draw_time, NULL); 	

	wmove(my_win, 1, 2);
	touchwin(my_win);
	wrefresh(my_win);

	main_cur_x = 2;
	main_cur_y = 1;

	//get keyboard input	
	while(1){
		//move
		wmove(my_win, main_cur_y, main_cur_x);
		wprintw(my_win, "");
		touchwin(my_win);
		wrefresh(my_win);

		draw_current_memo( get_y_coord(main_cur_y), get_x_coord(main_cur_x), main_cur_y, main_cur_x);

		char n_key;
		switch(n_key = getkey()){
			case 'A':
				//arrow up
				if(main_cur_y==5 || main_cur_y==9)
					main_cur_y-=2;
				else
					main_cur_y -= main_cur_y==1 ? 0 : 1;

				break;
			case 'B':
				//arrow down
				if(main_cur_y==3 || main_cur_y==7)
					main_cur_y+=2;
				else
					main_cur_y += main_cur_y==11 ? 0 : 1;

				break;
			case 'C':
				//arrow right
				if(main_cur_x==6 || main_cur_x==14)
					main_cur_x+=4;
				else
					main_cur_x += main_cur_x == 22 ? 0 : 2;

				break;
			case 'D':
				//arrow left
				if(main_cur_x==18 || main_cur_x==10)
					main_cur_x-=4;
				else
					main_cur_x -= main_cur_x == 2 ? 0 : 2;

				break;
			case 'c':
				for(int i=0;i<9;i++){
					for(int j=0;j<9;j++)
						num_arr[i][j] = original_question[i][j];
				}

				blank_count = count_blanks();
				werase(util_win);
				box(util_win, '|', '*');
				mvwprintw(util_win, 2, 3, "Blank : %d", blank_count);
				if( original_question[ get_y_coord(main_cur_y) ][ get_x_coord(main_cur_x) ] );
				else
					num_arr[ get_y_coord(main_cur_y) ][ get_x_coord(main_cur_x) ] = 0;
				draw_numbers(my_win, height, width, 0, get_y_coord(main_cur_y), get_x_coord(main_cur_x));

				refresh();
				wrefresh(util_win);
				set_memo_zero();
				break;
			case 'q':
				//quit game
				werase(user_status_win);
				pthread_cancel(time_thread);
				game_time=0;
				for(int a=0;a<9;a++){
					for(int b=0;b<9;b++){
						for(int c=0;c<2;c++){
							for(int d=0;d<2;d++)
								memo_arr[a][b][c][d] = 0;
						}
					}
				}

				set_array_zero();
				set_memo_zero();

				goto MAINMENU;
			case 'm':
				//do memo
				if( !original_question[ get_y_coord(main_cur_y) ][ get_x_coord(main_cur_x) ] )
					do_memo( get_y_coord(main_cur_y), get_x_coord(main_cur_x), main_cur_y, main_cur_x );
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if( original_question[ get_y_coord(main_cur_y) ][ get_x_coord(main_cur_x) ] );
				else
					num_arr[ get_y_coord(main_cur_y) ][ get_x_coord(main_cur_x) ] = (int)n_key-'0';
				draw_numbers(my_win, height, width, (int)n_key-'0', get_y_coord(main_cur_y), get_x_coord(main_cur_x));


				blank_count = count_blanks();
				werase(util_win);
				box(util_win, '|', '*');
				mvwprintw(util_win, 2, 3, "Blank : %d", blank_count);
				refresh();
				wrefresh(util_win);

				if(!blank_count){
					if(check_correct()){
						//time thread cancel
						pthread_cancel(time_thread);
						game_time = 0;
						for(int a=0;a<9;a++){
							for(int b=0;b<9;b++){
								for(int c=0;c<2;c++){
									for(int d=0;d<2;d++)
										memo_arr[a][b][c][d] = 0;
								}
							}
						}

						werase(user_status_win);
						wprintw(user_status_win, "CONGULATULATIONS!\n");
						wprintw(user_status_win, "You solved this SUDOKU!\n");
						wprintw(user_status_win, "Press Any Key to go Main menu.");
						refresh();
						wrefresh(user_status_win);
						char n_key = getkey();
						if(n_key == 'A' || n_key == 'B' || n_key == 'C' || n_key == 'D'){
							werase(user_status_win);	
							fclose(file);
							set_array_zero();
							set_memo_zero();
							goto MAINMENU;
						}else{
							werase(user_status_win);
							fclose(file);
							set_array_zero();
							set_memo_zero();
							goto MAINMENU;
						}
					}else{
						werase(user_status_win);
						mvwprintw(user_status_win, 1,1,"Still not correct...");
						refresh();
						wrefresh(user_status_win);
					}
				}	

				break;
			default:
				//invalid key input
				break;
		}
	}	

	getchar();
	endwin();
	return 0;
}

void draw_numbers(WINDOW * my_win, int height, int width, int now_number, int y_coord, int x_coord){
	int tp_x, tp_y, arr_x, arr_y;

	arr_x=0;
	arr_y=0;

	tp_x = 2;
	tp_y = 1;

	int cur_x;
	int cur_y;

	int is_getted = 0;	

	while(tp_y < height-1){
		tp_x=2;
		arr_x=0;
		if(tp_y==4 || tp_y==8){
			tp_y++;
			continue;
		}

		while(tp_x < width-2){
			wmove(my_win, tp_y, tp_x);

			if(num_arr[arr_y][arr_x]==0){
				waddch(my_win, '.' | COLOR_PAIR(1));
				arr_x++;
			}
			else{
				if(original_question[arr_y][arr_x] && num_arr[arr_y][arr_x] == now_number)
					waddch(my_win, num_arr[arr_y][arr_x++]+'0' | COLOR_PAIR(5));

				else if(original_question[arr_y][arr_x])
					waddch(my_win, num_arr[arr_y][arr_x++]+'0' | COLOR_PAIR(3));

				else if(num_arr[arr_y][arr_x] == now_number)
					waddch(my_win, num_arr[arr_y][arr_x++]+'0' | COLOR_PAIR(5));

				else if(num_arr[arr_y][arr_x] == now_number)
					waddch(my_win, num_arr[arr_y][arr_x++]+'0' | COLOR_PAIR(2));

				else
					waddch(my_win, num_arr[arr_y][arr_x++]+'0' | COLOR_PAIR(2));
			}

			if(tp_x==6 || tp_x==14){
				tp_x+=4;
				continue;
			}
			tp_x+=2;
		}
		tp_y++;
		arr_y++;
	}

	wrefresh(my_win);
}

void set_original_question(){
	for(int i=0;i<9;i++){
		for(int j=0;j<9;j++){
			if( num_arr[i][j] != 0 )
				original_question[i][j] = num_arr[i][j];
		}
	}	
}

void set_array_zero(){
	for(int i=0;i<9;i++){
		for(int j=0;j<9;j++)
			original_question[i][j] = 0;
	}
}

void set_memo_zero(){
	for(int a=0;a<9;a++){
		for(int b=0;b<9;b++){
			for(int c=0;c<2;c++){
				for(int d=0;d<2;d++)
					memo_arr[a][b][c][d] = 0;
			}
		}
	}
}

char getkey(){
	char ch = getch();
	if(ch == '\033'){
		getch();
		return getch();
	}else if(ch>='0' && ch<='9')
		return ch;	//return number
	else if(ch == 10)
		return 10;
	else if( (ch>='A'&&ch<='Z') || (ch>='a'&&ch<='z') )
		return ch; 
	return 'g';
}

int get_x_coord(int cur_x){
	if(cur_x>=18)
		return cur_x/2 - 3;
	else if(cur_x>=10)
		return cur_x/2 - 2;
	else
		return cur_x/2 - 1;
}

int get_y_coord(int cur_y){
	if(cur_y>=9)
		return cur_y-3;
	if(cur_y>=5)
		return cur_y-2;
	else
		return cur_y-1;
}

int check_correct(){
	for(int i=0;i<9;i++){
		if( !check_x_correct(i) )
			return 0;
	}

	for(int i=0;i<9;i++){
		if( !check_y_correct(i) )
			return 0;
	}

	if( !check_blocks_correct() )
		return 0;

	return 1;
}

//all rows check
int check_x_correct(int y){
	int check_arr[9] = {0,0,0,0,0,0,0,0,0};
	for(int i=0;i<9;i++){
		if( !check_arr[ num_arr[y][i]-1 ] )
			check_arr[ num_arr[y][i]-1 ] = 1;
		else
			return 0;
	}
	return 1;
}

//all columns check
int check_y_correct(int x){
	int check_arr[9] = {0,};
	for(int i=0;i<9;i++){
		if( !check_arr[ num_arr[i][x]-1 ] )
			check_arr[ num_arr[i][x]-1 ] = 1;
		else
			return 0;
	}
	return 1;
}

//current block check
int check_now_block_correct(int y, int x){
	int check_arr[9] = {0,};
	for(int i=y;i<=y+2;i++){
		for(int j=x;j<=x+2;j++){
			if( !check_arr[ num_arr[i][j]-1 ] )
				check_arr[ num_arr[i][j]-1 ] = 1;
			else
				return 0;
		}
	}
	return 1;
}

//all blocks check
int check_blocks_correct(){
	if(
			check_now_block_correct(0,0) &&
			check_now_block_correct(0,3) &&
			check_now_block_correct(0,6) &&
			check_now_block_correct(3,0) &&
			check_now_block_correct(3,3) &&
			check_now_block_correct(3,6) &&
			check_now_block_correct(6,0) &&
			check_now_block_correct(6,3) &&
			check_now_block_correct(6,6)
	  )
		return 1;
	else
		return 0;
}

int count_blanks(){
	int count=0;
	for(int y=0;y<9;y++){
		for(int x=0;x<9;x++){
			if( !num_arr[y][x] )
				count++;
		}
	}
	return count;
}

void draw_select_window(int now_sel){
	WINDOW * screen = newwin(15, 50, 10, 2);
	box(screen, '|', '*');

	mvwprintw(screen, 2, 18, "1. Very Easy");
	mvwprintw(screen, 4, 18, "2. Easy");
	mvwprintw(screen, 6, 18, "3. Medium");
	mvwprintw(screen, 8, 18, "4. Hard");
	mvwprintw(screen, 10, 18, "5. Extreme");
	mvwprintw(screen, 12, 18, "6. Exit Game");

	mvwprintw(screen, now_sel*2+2, 15, "V");

	wmove(screen, now_sel*2+2, 13);

	refresh();
	touchwin(screen);
	wrefresh(screen);
}

void * draw_time(void * arg){
	static int is_first_thread_call = 1;
	if( is_first_thread_call ){
		time_screen = newwin(5,30,4,40);
		box(time_screen, '|', '-');
		wbkgd(time_screen, COLOR_PAIR(8)); 
		is_first_thread_call = 0;
	}

	while(1){	
		wmove(time_screen, 2, 9);
		if( (game_time/3600)<10 ){
			wprintw(time_screen, "0");
			wprintw(time_screen, "%d", (game_time/3600) );
		}else
			wprintw(time_screen, "%d", (game_time/3600) );

		wprintw(time_screen, " : ");

		if( ( (game_time%3600)/60 )<10 ){
			wprintw(time_screen, "0");
			wprintw(time_screen, "%d", (game_time%3600)/60);
		}else
			wprintw(time_screen, "%d", (game_time%3600)/60);

		wprintw(time_screen, " : ");

		if( ( (game_time%3600)%60 )<10 ){
			wprintw(time_screen, "0");
			wprintw(time_screen, "%d", (game_time%3600)%60);
		}else
			wprintw(time_screen, "%d", (game_time%3600)%60);

		refresh();
		touchwin(time_screen);
		wrefresh(time_screen);

		//recall the cursor from time_screen
		if( is_memo_editing ){
			m_recall_cursor(memo_cur_y, memo_cur_x);
			refresh();
		}else{
			recall_cursor(main_cur_y, main_cur_x);
			refresh();
		}
		sleep(1);
		game_time++;
	}
}

void draw_current_memo(int y_coord, int x_coord, int current_y_cursor, int current_x_cursor){
	static int is_first_memo_call = 1;
	WINDOW * string_win = newwin(1,10,9,40);
	wprintw(string_win, "CELL MEMO");

	if(is_first_memo_call){
		memo_screen = newwin(5, 9, 10, 40);
		box(memo_screen, '|', '-');
		wbkgd(memo_screen, COLOR_PAIR(10));

		wmove(memo_screen, 2, 0);
		wprintw(memo_screen, "+");
		for(int i=0;i<9-2;i++)
			wprintw(memo_screen, "-");
		wprintw(memo_screen, "+");


		wmove(memo_screen, 0, 4);
		wprintw(memo_screen, "+");
		for(int i=1;i<5-1;i++){
			wmove(memo_screen, i, 4);
			wprintw(memo_screen, "|");
		}
		wmove(memo_screen, 5-1, 4);
		wprintw(memo_screen, "+");
	}

	refresh();
	touchwin(memo_screen);
	touchwin(string_win);
	wrefresh(memo_screen);
	wrefresh(string_win);

	if( !memo_arr[y_coord][x_coord][0][0] )
		mvwaddch(memo_screen, 1, 2, '.' | COLOR_PAIR(9));
	else
		mvwaddch(memo_screen, 1, 2, memo_arr[y_coord][x_coord][0][0]+'0' | COLOR_PAIR(9));

	if( !memo_arr[y_coord][x_coord][0][1] )
		mvwaddch(memo_screen, 1, 6, '.' | COLOR_PAIR(9));
	else
		mvwaddch(memo_screen, 1, 6, memo_arr[y_coord][x_coord][0][1]+'0' | COLOR_PAIR(9));

	if( !memo_arr[y_coord][x_coord][1][0] )
		mvwaddch(memo_screen, 3, 2, '.' | COLOR_PAIR(9));
	else
		mvwaddch(memo_screen, 3, 2, memo_arr[y_coord][x_coord][1][0]+'0' | COLOR_PAIR(9));

	if( !memo_arr[y_coord][x_coord][1][1] )
		mvwaddch(memo_screen, 3, 6, '.' | COLOR_PAIR(9));
	else
		mvwaddch(memo_screen, 3, 6, memo_arr[y_coord][x_coord][1][1]+'0' | COLOR_PAIR(9));

	refresh();
	wrefresh(memo_screen);

	if(current_y_cursor==-1 && current_x_cursor==-1)
		return;
	else{
		recall_cursor(current_y_cursor, current_x_cursor);
		refresh();
	}
}

int m_get_x_coord(int cur_x){
	switch(cur_x){
		case 2:
			return 0;
		case 6:
			return 1;
	}
}

int m_get_y_coord(int cur_y){
	switch(cur_y){
		case 1:
			return 0;
		case 3:
			return 1;
	}
}

void do_memo(int y_coord, int x_coord, int current_y_cursor, int current_x_cursor){
	memo_cur_x=2;
	memo_cur_y=1;
	is_memo_editing=1;

	while(1){
		wmove(memo_screen, memo_cur_y, memo_cur_x);
		wprintw(memo_screen, "");
		touchwin(memo_screen);
		wrefresh(memo_screen);

		char n_key;
		switch(n_key = getkey()){
			case 'A':
				//arrow up
				memo_cur_y -= memo_cur_y==1 ? 0 : 2;
				break;
			case 'B':
				//arrow down
				memo_cur_y += memo_cur_y==3 ? 0 : 2;
				break;
			case 'C':
				//arrow right
				memo_cur_x += memo_cur_x==6 ? 0 : 4;
				break;
			case 'D':
				//arrow left
				memo_cur_x -= memo_cur_x==2 ? 0 : 4;
				break;
			case 'm':
				//finish memo
				recall_cursor(current_y_cursor, current_x_cursor);
				refresh();
				is_memo_editing=0;
				return;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				memo_arr[y_coord][x_coord][ m_get_y_coord(memo_cur_y) ][ m_get_x_coord(memo_cur_x) ] = (int)n_key-'0';
				draw_current_memo(y_coord, x_coord, -1, -1);
				break;
			default:
				break;
		}
	}					
}

void recall_cursor(int s_y_coord, int s_x_coord){
	wmove(stdscr, 4 + s_y_coord, 10 + s_x_coord);
}

void m_recall_cursor(int m_y_coord, int m_x_coord){
	wmove(stdscr, 10+m_y_coord, 40+m_x_coord);
}
