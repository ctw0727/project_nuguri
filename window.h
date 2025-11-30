




#include <windows.h>
#include <conio.h>

#define LF "\r\n"

void clrscr();
void delay(int);

void clrscr() {
    system("cls");
	return;
}

// 밀리초 단위로 동작
void delay(int t){
	Sleep(t);
	return;
}
