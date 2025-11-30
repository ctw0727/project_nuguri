




#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define LF "\n" 

// 터미널 설정
struct termios orig_termios;

void clrscr();
void delay(int t);
void disable_raw_mode();
void enable_raw_mode();
int getch();
int kbhit();

void clrscr() {
    printf("\x1b[2J\x1b[H");
    fflush(stdout);
    return;
}

// 밀리초 단위로 동작
void delay(int t){
	usleep(t*1000);
	return;
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
