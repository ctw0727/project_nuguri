#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include "window.h"
#endif

#ifdef linux
#include "linux.h"
#endif

#ifdef __MACH__
#include "macos.h"
#endif


// 맵 및 게임 요소 정의 (수정된 부분)
//#define MAP_WIDTH 40  // 맵 너비를 40으로 변경
//#define MAP_HEIGHT 20
//#define MAX_STAGES 2
#define MAX_ENEMIES 15 // 최대 적 개수 증가
#define MAX_COINS 30   // 최대 코인 개수 증가

// 방향키 판단을 위한 define (-32 catch 한 이후 판단)
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77


// 운영체제 분리 필요
#ifdef _WIN32
    #include <windows.h>   // Beep() 함수 사용
#else
    #include <unistd.h>    // usleep 같은 거 이미 쓰고 있을 수도 있어서 (맥/리눅스)
#endif

// 구조체 정의
typedef struct {
    int x, y;
    int dir; // 1: right, -1: left
} Enemy;

typedef struct {
    int x, y;
    int collected;
} Coin;

// 전역 변수
//char map[MAX_STAGES][MAP_HEIGHT][MAP_WIDTH + 1];
int MAP_HEIGHT;
int MAP_WIDTH;
char*** map;

char DEBUGGING = 0;

// 추가된 부분
int MAX_STAGES;
int* TEMP_HEIGHT;
int* TEMP_WIDTH;

#define MAX_HP 3   // 기본 최대 HP
// 추가된 부분

int player_x, player_y;
int stage = 0;
int score = 0;

int hp = MAX_HP; // 플레이어 체력(초기값은 MAX_HP)

// 플레이어 상태
int is_jumping = 0;
int velocity_y = 0;
int on_ladder = 0;

// 게임 객체
Enemy enemies[MAX_ENEMIES];
int enemy_count = 0;
Coin coins[MAX_COINS];
int coin_count = 0;

// 함수 선언
void load_maps();
void init_stage();
void draw_game();
void update_game(char input);
void move_player(char input);
void move_enemies();
void check_collisions();
void setMapMemory();
//void setMapMemory(int s, int width, int height);
void getMapSize();
void readBanner(char* str, int height);
void opening();
void mallocFree();

void DBG(char* str); //debugging print

//추가된
void setStage();
void beep();


int main() {
    srand(time(NULL));
    enable_raw_mode();
	
    opening();
	  getMapSize();
    
    //추가된
    setStage();
    getMapSize();
    MAP_HEIGHT = TEMP_HEIGHT[stage];
    MAP_WIDTH = TEMP_WIDTH[stage];
    //추가된
  
  
    load_maps();
    init_stage();

    char c = '\0';
    int game_over = 0;
	
    while (!game_over && stage < MAX_STAGES) {
		
        if (kbhit()) {
            c = getch();
            if (c == 'q') {
                game_over = 1;
                continue;
            }
            if (c == -32) { // 화살표 입력 받기위한 (화살표 입력하면 -32가 먼저 입력된 후 72,80 등 고유 숫자가 입력됨.)
                c = getch();
            }
        } else {
            c = '\0';
        }
        update_game(c);
        draw_game();

        if (map[stage][player_y][player_x] == 'E') {
            mallocFree(stage);
            stage++;
            getMapSize(stage);
            load_maps();
            score += 100;
            if (stage < MAX_STAGES) {
                init_stage();
            } else {
                game_over = 1;
                printf("\x1b[2J\x1b[H");
                printf("축하합니다! 모든 스테이지를 클리어했습니다!\n");
                printf("최종 점수: %d\n", score);
            }
        }
    }

    disable_raw_mode();
    return 0;
}



// 맵 파일 로드
void load_maps() {
	if(DEBUGGING) DBG("load_maps() started");
	if(DEBUGGING) delay(500);
	
    FILE *file = fopen("map.txt", "r");
    if (!file) {
        perror("map.txt 파일을 열 수 없습니다.");
        exit(1);
    }
	
    int s = 0, r = 0;
    char line[MAP_WIDTH + 2]; // 버퍼 크기는 MAP_WIDTH에 따라 자동 조절됨
    while (s < MAX_STAGES && fgets(line, sizeof(line), file)) {
		//if(DEBUGGING) DBG("in while");
		//if(DEBUGGING) delay(30);
		
        if ((line[0] == '\n') && r > 0) { // map.txt 파일은 LF만 사용하므로, \n만 확인해도 됨.
            s++;
            r = 0;
			if(DEBUGGING) DBG("in load_maps() stage plused");
			if(DEBUGGING) delay(500);
            continue;
        }
        if (r < MAP_HEIGHT) {
            // 윈도우즈에서는 "\r\n" 사용하는 경우가 많다고 하는데, os마다 고려해서 예외처리가 필요할 거 같습니다.
            // 그래서 그냥 운영체제별로 개행 #define LF 로 해놓음, 문자열 형태 window: \r\n  macos: \r  linux: \n
            // 참고: https://teck10.tistory.com/296
            
            //line[strcspn(line, LF)] = 0;
			//strcspn(char* str1, char* str2) - str2에 들어있는 '문자들' 중에서 str1에 들어있는 '문자'와 일치하는 것이 있다면 첫번째로 일치하는 문자까지 읽어들인 수를 리턴
            strncpy(map[s][r], line, MAP_WIDTH);
			//strncpy(char* str1, char* str2, int count) - string2의 count자를 string1에 복사
            
            r++;
        }
    }
    fclose(file);
	
	if(DEBUGGING) DBG("load_maps() ended");
	if(DEBUGGING) delay(500);
}

// 현재 스테이지 초기화
void init_stage() {
	if(DEBUGGING) DBG("init_stage(); started");
	if(DEBUGGING) delay(300);
	
    enemy_count = 0;
    coin_count = 0;
    is_jumping = 0;
    velocity_y = 0;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = map[stage][y][x];
            if (cell == 'S') {
                player_x = x;
                player_y = y;
            } else if (cell == 'X' && enemy_count < MAX_ENEMIES) {
                enemies[enemy_count] = (Enemy){x, y, (rand() % 2) * 2 - 1};
                enemy_count++;
            } else if (cell == 'C' && coin_count < MAX_COINS) {
                coins[coin_count++] = (Coin){x, y, 0};
            }
        }
    }
	if(DEBUGGING) DBG("init_stage(); ended");
	if(DEBUGGING) delay(300);
}

// 게임 화면 그리기
void draw_game() {
	if(DEBUGGING) DBG("draw_game(); started");
	if(DEBUGGING) delay(300);
    clrscr();
    printf("Stage: %d | Score: %d\n", stage + 1, score);
    printf("HP: %d\n", hp); // 플레이어 체력 표시
    printf("조작: ← → (이동), ↑ ↓ (사다리), Space (점프), q (종료)\n");
	
    char display_map[MAP_HEIGHT][MAP_WIDTH + 1];
    for(int y=0; y < MAP_HEIGHT; y++) {
        for(int x=0; x < MAP_WIDTH; x++) {
            char cell = map[stage][y][x];
            if (cell == 'S' || cell == 'X' || cell == 'C') {
                display_map[y][x] = ' ';
            } else {
                display_map[y][x] = cell;
            }
        }
    }
    
    for (int i = 0; i < coin_count; i++) {
        if (!coins[i].collected) {
            display_map[coins[i].y][coins[i].x] = 'C';
        }
    }

    for (int i = 0; i < enemy_count; i++) {
        display_map[enemies[i].y][enemies[i].x] = 'X';
    }

    display_map[player_y][player_x] = 'P';

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for(int x = 0; x < MAP_WIDTH; x++){
            printf("%c", display_map[y][x]);
        }
        printf("\n");
    }
	
	if(DEBUGGING) DBG("draw_game(); ended");
	if(DEBUGGING) delay(300);
}

// 게임 상태 업데이트
void update_game(char input) {
    if(DEBUGGING) DBG("update_game(); started");
	if(DEBUGGING) delay(300);
	
	move_player(input);
    move_enemies();
    check_collisions();
	
	if(DEBUGGING) DBG("update_game(); ended");
	if(DEBUGGING) delay(300);
}

// 플레이어 이동 로직
void move_player(char input) {
	if(DEBUGGING) DBG("move_player(); started");
	if(DEBUGGING) delay(300);
	
    int next_x = player_x, next_y = player_y;
    char floor_tile = (player_y + 1 < MAP_HEIGHT) ? map[stage][player_y + 1][player_x] : '#';
	// 삼항연산자, (조건) ? (참일때 값) : (거짓일 때 값)
	// floor_tile : 플레이어 발 아래의 블록이 무엇인지
	
    char current_tile = map[stage][player_y][player_x];
	
    on_ladder = (current_tile == 'H');

    switch (input) {
        case LEFT:   next_x--; break;
        case RIGHT:  next_x++; break;
        case UP:     if (on_ladder) next_y--; break;
        case DOWN:   if (on_ladder && (player_y + 1 < MAP_HEIGHT) && map[stage][player_y + 1][player_x] != '#') next_y++; break;
        case ' ': // 점프
            if (!is_jumping && (floor_tile == '#' || on_ladder)) {
                is_jumping = 1;
                velocity_y = -2;
            }
            break;
    }

    if (next_x >= 0 && next_x < MAP_WIDTH && map[stage][player_y][next_x] != '#') player_x = next_x; // 땅 위에서 좌우이동
    
    if (on_ladder && (input == UP || input == DOWN)) { //사다리 위아래 이동
		if (next_y >= 0 && input == UP && map[stage][next_y][player_x] == '#'){ //사다리 끝에서 위로 이동했을 때
			player_y = next_y-1;
            is_jumping = 0;
            velocity_y = 0;
		}else if (next_y >= 0 && next_y < MAP_HEIGHT && map[stage][next_y][player_x] != '#') {
            player_y = next_y;
            is_jumping = 0;
            velocity_y = 0;
        }
    } else { // 사다리에서 위아래로 이동 중이 아닐때
        if (is_jumping) { // 점프중에~
            next_y = player_y + velocity_y;
            if(next_y < 0) next_y = 0; // 0보다 작아지지 않게
            velocity_y++; // 중력가속도(클수록 아래로)

            if (velocity_y > 0 && (next_y > MAP_HEIGHT || map[stage][next_y][player_x] == '#')) { // 행선지가 땅일때, 땅보다 아래로 향할 때 (수정됨)
				velocity_y = 0;
            } else if (next_y < MAP_HEIGHT) { // 행선지가 땅에 닿지 않을 때
                player_y = next_y;
            }
			
            if ((player_y + 1 > MAP_HEIGHT) || map[stage][player_y + 1][player_x] == '#') { // 이미 땅에 닿아있을 때
                is_jumping = 0;
                velocity_y = 0;
				next_y = MAP_HEIGHT-1;
            }
        } else { // 점프중이 아니면~
            if (floor_tile != '#' && floor_tile != 'H') { // 땅 위도 사다리 위도 아닐 때 (허공일 때)
                 if (player_y + 1 < MAP_HEIGHT) player_y++; // 맵을 벗어나지 않으면 아래로 한 칸 이동
                 else init_stage();
            }
        }
    }
    
    if (player_y >= MAP_HEIGHT) init_stage();
	
	if(DEBUGGING) DBG("move_player(); ended");
	if(DEBUGGING) delay(300);
}

// 적 이동 로직
void move_enemies() {
	if(DEBUGGING) DBG("move_enemies(); started");
	if(DEBUGGING) delay(300);
	
    for (int i = 0; i < enemy_count; i++) {
        int next_x = enemies[i].x + enemies[i].dir;
        if (next_x < 0 || next_x >= MAP_WIDTH || map[stage][enemies[i].y][next_x] == '#' || (enemies[i].y + 1 < MAP_HEIGHT && map[stage][enemies[i].y + 1][next_x] == ' ')) {
            enemies[i].dir *= -1;
        } else {
            enemies[i].x = next_x;
        }
    }
	
	if(DEBUGGING) DBG("move_enemies(); ended");
	if(DEBUGGING) delay(300);
}

// 충돌 감지 로직
void check_collisions() {
	if(DEBUGGING) DBG("check_collisions(); started");
	if(DEBUGGING) delay(300);
	
    for (int i = 0; i < enemy_count; i++) {
    if (player_x == enemies[i].x && player_y == enemies[i].y) {
        // 적과 충돌: 체력 감소
        hp--;
        // 점수 패널티는 유지(원하면 제거 가능)
        score = (score > 50) ? score - 50 : 0;

        if (hp <= 0) {
            // 게임 오버 처리: 화면 정리 후 종료
            printf("\x1b[2J\x1b[H");
            printf("게임 오버! HP가 모두 소진되었습니다.\n");
            printf("최종 점수: %d\n", score);
            disable_raw_mode();
            exit(0);
        } else {
            // 체력이 남아있으면 현재 스테이지 재시작 (플레이어 위치 초기화)
            init_stage();
            return;
        }
    }
}

    for (int i = 0; i < coin_count; i++) {
        if (!coins[i].collected && player_x == coins[i].x && player_y == coins[i].y) {
            beep();
            coins[i].collected = 1;
            score += 20;
        }
    }
	
	if(DEBUGGING) DBG("check_collisions(); ended");
	if(DEBUGGING) delay(300);
}


//추가된
void setStage() {
    char temp;
    int check_stage = 0, sum_stage = 1;

    FILE* file = fopen("map.txt", "r");
    if (!file) {
        perror("map.txt 파일을 열 수 없습니다.");
        exit(1);
    }
    while (fscanf(file, "%c", &temp) != EOF) {
        switch (temp) {
        case '\n':
        case '\r':
            if (check_stage == 1) {
                sum_stage++;
            }
            check_stage = 1;
            break;
        default:
            check_stage = 0;
            break;
        }
    }
    MAX_STAGES = sum_stage;
    map = (char***)malloc(sizeof(char**) * MAX_STAGES);
    TEMP_HEIGHT = (int*)malloc(sizeof(int) * MAX_STAGES);
    TEMP_WIDTH = (int*)malloc(sizeof(int) * MAX_STAGES);
    fclose(file);
}
//추가된

// 맵 전역변수에 동적 메모리 할당
void setMapMemory() {
	if(DEBUGGING) DBG("setMapMemory(); started");
	if(DEBUGGING) delay(300);
	
    int i =0, j = 0;
	
	//전역 변수 활용하게 수정
    map = (char***)malloc(sizeof(char**) * MAX_STAGES); // MAX_STAGES
    for(i = 0; i < MAX_STAGES; i++){ 
        map[i] = (char**)malloc(sizeof(char*) * MAP_HEIGHT);  //MAP_HEIGHT
        for(j = 0; j < MAP_HEIGHT; j++){
            map[i][j] = (char*)malloc(sizeof(char) * MAP_WIDTH); //MAP_WIDTH
        }
    }
	
	if(DEBUGGING) DBG("setMapMemory(); ended");
	if(DEBUGGING) delay(300);
}

void setMapMemory(int s, int width, int height) {
    int i = 0;
    TEMP_HEIGHT[s] = height;
    TEMP_WIDTH[s] = width;
    map[s] = (char**)malloc(sizeof(char*) * TEMP_HEIGHT[s]);  //MAP_HEIGHT
    for (i = 0; i < TEMP_HEIGHT[s]; i++) {
        map[s][i] = (char*)malloc(sizeof(char) * TEMP_WIDTH[s]); //MAP_WIDTH
    }
}

// 맵 사이즈 계산
void getMapSize() {
	if(DEBUGGING) DBG("getMapSize(); started");
	if(DEBUGGING) delay(300);
	  
    /* 추가된 변수들
    int temp_width = 0, width = 0, height = 0;
    int temp_stage = 0, check_stage = 0;
    char temp;
    */
    
    int height = 0;
    char buffer[45];

    FILE* file = fopen("map.txt", "r");
    if (!file) {
        perror("map.txt 파일을 열 수 없습니다.");
        exit(1);
    }
	
	while(fgets(buffer, sizeof(buffer), file) != NULL){
		if (buffer[0] == '\n'){
			if (MAP_HEIGHT < height) MAP_HEIGHT = height;
			height=0;
		}else height++;
	}
	if (MAP_HEIGHT < height) MAP_HEIGHT = height;
    MAP_WIDTH = strlen(buffer);
	
	/*
	height 267?? 정도 나옴, 이렇게 쓰면 안됨.
    while(fscanf(file,"%s",buffer) != EOF) height++;
        width = sizeof(buffer) / sizeof(char);
		ㄴ 이렇게하면 buffer 사이즈만큼 설정됨, 45로 잡힌다는 거임
	*/
    setMapMemory();
    
    /* 로직 참조할 부분
    while (fscanf(file, "%c", &temp) != EOF) {
        switch (temp) {
        case '\n':
        case '\r':
            if (check_stage == 1) {
                setMapMemory(temp_stage, width, height);
                height = 0;
                temp_stage++;
                continue;
            }
            check_stage = 1;
            height++;
            width = temp_width;
            temp_width = 0;
            break;
        default:
            check_stage = 0;
            temp_width++;
            break;
        }
    }
    setMapMemory(temp_stage, width, height + 1);
    */
    fclose(file);
	
	//width 45
	if(DEBUGGING) printf("///MAP_WIDTH: %d, MAP_HEIGHT: %d   ", MAP_WIDTH, MAP_HEIGHT);
	if(DEBUGGING) DBG("getMapSize(); ended");
	if(DEBUGGING) delay(300);
}

void mallocFree() {
    int i, j;
    free(TEMP_HEIGHT);
    free(TEMP_WIDTH);
    for (i = 0; i < MAX_STAGES; i++) {
        for (j = 0; j < TEMP_HEIGHT[i]; j++) {
            free(map[i][j]);
        }
        free(map[i]);
    }
    free(map);
}

// 너구리 배너 띄우려고 만든 코드
void readBanner(char* str, int height){
    FILE *file = fopen(str, "r");
    if (!file) {
		perror("파일을 열 수 없습니다.");
		exit(1);
    }
    int h = 0, r = 0;
    char C;
    char line[50];
	
	while (h<height && fgets(line, sizeof(line), file)) {
		printf(line);
		h++;
	}
	fclose(file);
}

// 엔딩화면
void ending(){
	clrscr();
	readBanner("endAni1.txt", 20);
	printf(LF LF "             >> YOU WIN <<" LF LF);
	return;
}

// 시작화면
void opening(){
	int select = 0;
	int d = 0;
	char c;
	
	clrscr();
	readBanner("banner.txt", 11);
	printf(LF LF "         press Enter to select" LF LF);
	printf("           START        EXIT");
	
	while(1){
		
		int x;
		if (select==0) x=9;
		else x=22;
		
		gotoxy(x,14);
		printf(">");
		gotoxy(0,16);
		
		c = getch();
		if (c==-32){ //화살표 입력을 받았을 때
			c = getch();
			if (c==LEFT && select!=0) {select--; gotoxy(x,14); printf(" ");}
			else if (c==RIGHT && select!=1) {select++; gotoxy(x,14); printf(" ");}
		} else if (c=='d'){ // 디버깅 모드 진입
			if (d==0) {d = 1;}
			else d = 0;
		} else if (c== LF[0]) break;
		
		if (d==1){
			gotoxy(0,12);
			printf("        Debuging mode activated" LF LF);
			printf("           START        ENDING");
		} else{
			gotoxy(0,12);
			printf("         press Enter to select" LF LF);
			printf("           START        EXIT");
		}
	}
	
	if (d==0){
		if (select == 0) return;
		else exit(1);
	}else if (d==1){
		if (select == 0){ //debug start
			DEBUGGING = 1;
			return;
		} else{ //ending
			ending();
			return;
		}
	}
}

void DBG(char* str){
	gotoxy(0,30);
	printf("//%s   ", str);
	return;
}

// 따로 운영체제별 분리 필요할 것으로 보임
void beep(void) {
    #ifdef _WIN32
        //윈도우
        Beep(750, 100);
    #else
        //리눅스
        printf("\a");
        fflush(stdout);
    #endif
    }

