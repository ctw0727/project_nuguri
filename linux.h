
/*
linux.h : 리눅스 운영체제에서 참조하기 위해 만든 헤더파일,
코드 내부에서 공용으로 쓰이는 함수들을 운영체제에 맞게 정리했습니다.
안정적인 참조를 위해 컴파일 시 -D linux 를 옵션으로 추가해주세요.
*/

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define LF "\n" 

// 터미널 설정
struct termios orig_termios;

void clrscr(); // 화면 클리어 함수
void delay(int t); // 마이크로초단위 sleep 함수
void disable_raw_mode(); // 입력된 문자를 사용자 프로그램으로 즉시 전달하는 터미널 RAW 모드 관련 함수 (RAW모드 비활성화)
void enable_raw_mode(); // RAW모드 활성화
int getch(); // linux/MacOS에서 활용하기 위한 getch 함수 구현
int kbhit(); // linux/MacOS에서 활용하기 위한 kbhit 함수 구현
void gotoxy(int x, int y); // 커서위치 draw 함수

void clrscr() {
    printf("\x1b[2J\x1b[1;1H"); // ANSI 이스케이프 함수로 화면 클리어하고 1열 1행으로 커서 이동.
    fflush(stdout); // fflush로 출력버퍼 초기화, ANSI 이스케이프 함수 안정적 실행을 도움.
    return;
}

// 밀리초 단위로 동작
void delay(int t){
	usleep(t*1000); //마이크로초단위로 동작하므로 1000 곱해서 밀리초로 단위 조정
	return;
}

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y, x); // ANSI 이스케이프 함수 이용해 1열 1행으로 커서 이동.
    fflush(stdout); // fflush로 출력버퍼 초기화, ANSI 이스케이프 함수 안정적 실행을 도움.
}

// 터미널 Raw 모드 활성화/비활성화
void disable_raw_mode() ////Raw 모드란? 터미널 원시 모드 - Raw모드에서 시스템은 입력된 문자를 사용자 프로그램으로 즉시 전달
{ 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); 
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int getch() {
    struct termios oldt, newt;
    int ch;
    int oldf;
	
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
	
    newt.c_lflag &= ~(ICANON | ECHO);
	newt.c_cc[VMIN] = 1; // 최소 얼마크기의 데이터가 들어왔을 때 읽을지
	newt.c_cc[VTIME] = 0; // 데이터 수신 대기 시간 (수신을 얼마나 기다릴지)
	//수신된 문자 수가 VMIN 이상일 때 즉시 반환.
	//https://softtone-someday.tistory.com/15
	
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	
    ch = getchar();
	
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
	return ch;
}

int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}
