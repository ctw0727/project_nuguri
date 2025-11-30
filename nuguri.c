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
#define MAX_STAGES 2
#define MAX_ENEMIES 15 // 최대 적 개수 증가
#define MAX_COINS 30   // 최대 코인 개수 증가

// 방향키 판단을 위한 define (-32 catch 한 이후 판단)
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77

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

int player_x, player_y;
int stage = 0;
int score = 0;

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
void setMapMemory(int width, int height);
void getMapSize();
void readBanner(char* str);
void opening();


int main() {
    srand(time(NULL));
    enable_raw_mode();
    
    opening();
    
    load_maps();
    init_stage();

    char c = '\0';
    int game_over = 0;

    while (!game_over && stage < MAX_STAGES) {
        if (kbhit()) {
            c = getchar();
            if (c == 'q') {
                game_over = 1;
                continue;
            }
            if (c == -32) {
                c = getchar();
            }
        } else {
            c = '\0';
        }

        update_game(c);
        draw_game();
        delay(90);

        if (map[stage][player_y][player_x] == 'E') {
            stage++;
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
    FILE *file = fopen("map.txt", "r");
    if (!file) {
        perror("map.txt 파일을 열 수 없습니다.");
        exit(1);
    }
    int s = 0, r = 0;
    char line[MAP_WIDTH + 2]; // 버퍼 크기는 MAP_WIDTH에 따라 자동 조절됨
    while (s < MAX_STAGES && fgets(line, sizeof(line), file)) {
        if ((line[0] == '\n' || line[0] == '\r') && r > 0) {
            s++;
            r = 0;
            continue;
        }
        if (r < MAP_HEIGHT) {
            // 윈도우즈에서는 "\r\n" 사용하는 경우가 많다고 하는데, os마다 고려해서 예외처리가 필요할 거 같습니다.
            // 그래서 그냥 운영체제별로 개행 #define LF 로 해놓음, 문자열 형태 window: \r\n  macos: \r  linux: \n
            // 참고: https://teck10.tistory.com/296
            
            line[strcspn(line, LF)] = 0; ////strcspn(char* str1, char* str2) - str2에 들어있는 '문자들' 중에서 str1에 들어있는 '문자'와 일치하는 것이 있다면 첫번째로 일치하는 문자까지 읽어들인 수를 리턴
            // ㄴ 근데 이거 필요한 거 맞음???
            
            strncpy(map[s][r], line, MAP_WIDTH + 1); ////strncpy(char* str1, char* str2, int count) - string2의 count자를 string1에 복사
            // 그냥 문자열 마지막 NULL 안줘도 되는 거 아님?? 어차피 MAP_WIDTH까지밖에 탐색(출력) 안 하잖음
            r++;
        }
    }
    fclose(file);
}

// 현재 스테이지 초기화
void init_stage() {
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
}

// 게임 화면 그리기
void draw_game() {
    clrscr();
    printf("Stage: %d | Score: %d\n", stage + 1, score);
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
        for(int x=0; x< MAP_WIDTH; x++){
            printf("%c", display_map[y][x]);
        }
        printf("\n");
    }
}

// 게임 상태 업데이트
void update_game(char input) {
    move_player(input);
    move_enemies();
    check_collisions();
}

// 플레이어 이동 로직
void move_player(char input) {
    int next_x = player_x, next_y = player_y;
    char floor_tile = (player_y + 1 < MAP_HEIGHT) ? map[stage][player_y + 1][player_x] : '#';
    char current_tile = map[stage][player_y][player_x];

    on_ladder = (current_tile == 'H');

    switch (input) {
        case LEFT:   next_x--; break;
        case RIGHT:  next_x++; break;
        case UP:     if (on_ladder) next_y--; break;
        case DOWN:   if (on_ladder && (player_y + 1 < MAP_HEIGHT) && map[stage][player_y + 1][player_x] != '#') next_y++; break;
        case ' ':
            if (!is_jumping && (floor_tile == '#' || on_ladder)) {
                is_jumping = 1;
                velocity_y = -2;
            }
            break;
    }

    if (next_x >= 0 && next_x < MAP_WIDTH && map[stage][player_y][next_x] != '#') player_x = next_x;
    
    if (on_ladder && (input == 'w' || input == 's')) {
        if(next_y >= 0 && next_y < MAP_HEIGHT && map[stage][next_y][player_x] != '#') {
            player_y = next_y;
            is_jumping = 0;
            velocity_y = 0;
        }
    } 
    else {
        if (is_jumping) {
            next_y = player_y + velocity_y;
            if(next_y < 0) next_y = 0;
            velocity_y++;

            if (velocity_y < 0 && next_y < MAP_HEIGHT && map[stage][next_y][player_x] == '#') {
                velocity_y = 0;
            } else if (next_y < MAP_HEIGHT) {
                player_y = next_y;
            }
            
            if ((player_y + 1 < MAP_HEIGHT) && map[stage][player_y + 1][player_x] == '#') {
                is_jumping = 0;
                velocity_y = 0;
            }
        } else {
            if (floor_tile != '#' && floor_tile != 'H') {
                 if (player_y + 1 < MAP_HEIGHT) player_y++;
                 else init_stage();
            }
        }
    }
    
    if (player_y >= MAP_HEIGHT) init_stage();
}


// 적 이동 로직
void move_enemies() {
    for (int i = 0; i < enemy_count; i++) {
        int next_x = enemies[i].x + enemies[i].dir;
        if (next_x < 0 || next_x >= MAP_WIDTH || map[stage][enemies[i].y][next_x] == '#' || (enemies[i].y + 1 < MAP_HEIGHT && map[stage][enemies[i].y + 1][next_x] == ' ')) {
            enemies[i].dir *= -1;
        } else {
            enemies[i].x = next_x;
        }
    }
}

// 충돌 감지 로직
void check_collisions() {
    for (int i = 0; i < enemy_count; i++) {
        if (player_x == enemies[i].x && player_y == enemies[i].y) {
            score = (score > 50) ? score - 50 : 0;
            init_stage();
            return;
        }
    }
    for (int i = 0; i < coin_count; i++) {
        if (!coins[i].collected && player_x == coins[i].x && player_y == coins[i].y) {
            coins[i].collected = 1;
            score += 20;
        }
    }
}

void setMapMemory(int width, int height) {
    int i =0, j = 0;
    MAP_HEIGHT = height;
    MAP_WIDTH = width;
    map = (char***)malloc(sizeof(char**) * 2); //MAX_STAGE
    for(i = 0; i < 2; i++){
        map[i] = (char**)malloc(sizeof(char*) * height);  //MAP_HEIGHT
        for(j = 0; j < MAP_HEIGHT; j++){
            map[i][j] = (char*)malloc(sizeof(char) * width); //MAP_WIDTH
        }
    }
}

void getMapSize() {
    int width;
    int height = 0;
    char buffer[45];

    FILE *file = fopen("map.txt", "r");
    if (!file) {
        perror("map.txt 파일을 열 수 없습니다.");
        exit(1);
    }
    while(fscanf(file,"%s",buffer) != EOF) height++;
        width = sizeof(buffer) / sizeof(char);

    setMapMemory(width, height);
    fclose(file);
}

void mallocFree() {
    int i = 0, j = 0;
    for(i = 0; i < 2; i++){
        for(j = 0; j < MAP_HEIGHT; j++){
            free(map[i][j]);
        }
        free(map[i]);
    }
    free(map);
}

// 너구리 배너 띄우려고 만든 코드
void readBanner(char* str){
    FILE *file = fopen(str, "r");
    if (!file) {
		perror("파일을 열 수 없습니다.");
		exit(1);
    }
    int h = 0, r = 0;
    char C;
    char line[45];
	
	while (h<11 && fgets(line, sizeof(line), file)) {
		printf(line);
		h++;
	}
	fclose(file);
}

// 시작화면
void opening(){
	int select = 0;
	char c;
	while(1){
		clrscr();
		readBanner("banner.txt");
		printf(LF LF "         press Enter to select" LF LF);
		
		if (select==0) printf("         > START        EXIT");
		else printf("           START      > EXIT");
		
		c = getch();
		if (c==-32) c = getchar();
		
		if (c==LEFT && select!=0) select--;
		else if (c==RIGHT && select!=1) select++;
		else if (c== LF[0]) break;
	}
	if (select == 0) return;
	else exit(1);
}
