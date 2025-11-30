




#include <windows.h>
#include <conio.h>

#define LF "\r\n"

void clrscr();
void delay(int);
void disable_raw_mode();
void enable_raw_mode();

void clrscr() {
    system("cls");
	return;
}

// 밀리초 단위로 동작
void delay(int t){
	Sleep(t);
	return;
}

void disable_raw_mode(){
	return;
}

void enable_raw_mode(){
	return;
}
