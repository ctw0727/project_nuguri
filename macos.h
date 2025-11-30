




#include <termios.h>

#define LF "\r" 

void clrscr() {
    printf("\x1b[2J\x1b[H");
    fflush(stdout);
    return;
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
