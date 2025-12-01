
/*
window.h : 윈도우 운영체제에서 참조하기 위해 만든 헤더파일,
코드 내부에서 공용으로 쓰이는 함수들을 운영체제에 맞게 정리했습니다.
안정적인 참조를 위해 컴파일 시 -D _WIN32 를 옵션으로 추가해주세요.
*/

#include <windows.h>
#include <conio.h>

#define LF "\r\n"

void clrscr(); // 화면 클리어 함수
void delay(int t); // 마이크로초단위 sleep 함수
void disable_raw_mode(); // main 에서 쓰는관계로 일단 빈 코드로 구현
void enable_raw_mode(); // 22
void gotoxy(int x, int y); // 커서위치 draw 함수

void clrscr() {
    system("cls"); // windows.h 내부 함수 사용하여 구현
	return;
}

void gotoxy(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // 표준 출력 장치 (콘솔 화면 버퍼)에 대한 핸들을 얻기 위해 사용되는 함수
	// HANDLE : 파일, 프로세스, 콘솔 화면 버퍼와 같이 운영체제에서 관리하는 객체의 열린 인스턴스를 나타내는 것
	// UNITY에서 사용하는 instance 나 object 의 개념과 비슷, 일단은 특정 개체를 참조하기 위한 ID라고 이해해도 좋다
	
	COORD pos; // Windows 콘솔 화면의 문자 셀 좌표를 나타내는 구조체 (windows.h)
	
	// COORD 좌표 지정
	pos.X = x;
    pos.Y = y;
	
    SetConsoleCursorPosition(hConsole, pos); // 콘솔 창에서 커서의 위치를 지정 좌표로 이동시키는 함수 (windows.h)
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
