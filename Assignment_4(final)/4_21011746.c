/*	SW 설계 기초 과제
	Tetris 21011746 양현석 (최종 수정 : 20240929)
	
	C언어로 테트리스 게임을 구현하는 과제입니다.
	조작법은 방향키(상키는 회전), 스페이스(맨 아래로 내리기), ESC(일시정지), TAB(다음 블럭과 Switch)입니다.

	원래 없던 기능인 일시정지와 Switch 기능, 블럭 색상을 넣어 플레이어의 편의성을 증대시키고, 플레이 경험을 상향시키도록 했습니다.
	더하여 예상 블럭 위치를 보여주는 기능을 넣어서 한 층 더 개선된 플레이가 가능하도록 했습니다.

	'Down'와 'Space' 키로 블럭을 내리면 y 한 칸 당 1점씩 증가하게 했고, 한 줄 지울 시 정해진 공식을 통해 점수를 부여하였습니다.
	한 번에 지우는 줄 수에 따라 점수가 지수 함수꼴로 올라가게 다음과 같이 공식을 부여했습니다. "int(exp((double)cnt * 0.9) * 20" (cnt는 줄 수)
	이어 300점 단위로 레벨이 올라가는 시스템을 도입했습니다. 레벨이 올라갈 때마다 블럭이 내려오는 속도가 점점 줄어듭니다.
	다음 공식에 의해 블럭이 내려오는 속도가 정해집니다. "(float)3 / (LV + 5)" (LV은 0 ~ 8)

	또한 블럭 구조체를 이용해서 함수들의 인자를 간소화 하고, 재활용할 수 있도록 구현했습니다.

	옛날 오락실 게임기에서 레퍼런스를 얻어 다양한 이벤트를 넣어서 사용자에게 알림을 줍니다.(레벨 업, 콤보 수 등)
	게임 시작과 게임 오버 시에도 같은 방식으로 사용자에게 이벤트를 부여하여, 인터페이스적으로 훨씬 깔끔하게 보일 수 있습니다.
	
	점수로 플레이어 간의 경쟁뿐만 아니라 플레이 타임을 넣어서 누가 더 오래 플레이 했나로 경쟁할 수도 있을 것입니다.(다양한 플레이 가능)

	이상입니다. 
*/

#include <stdio.h>

#include "block.h"
#include <windows.h>
#include <time.h>
#include <conio.h>

//	수학적 계산을 위해서 사용
#include <limits.h>
#include <math.h>

//	Macro
#define INT_HEIGHT 25
#define INT_WIDTH 38
#define INT_MENU_X 41
#define INT_MENU_WIDTH 15

//	Block을 객체화시키는 구조체
typedef struct Block {
	//	Block의 위치
	COORD pos;

	//	칠해지는 부분의 좌표
	COORD elem[4];

	//	Original Shape
	int shape;
	int curShape;

	//   GameOver를 판정하기 위한 변수
	int flag;

	//	블럭 색상
	int color;
}Block;


//   Method
/*
*	블럭을 생성하고, 복사하고, 구조체 변수들을 업데이트하는 파트입니다.
*	setBlock은 주어진 pos로 newBlock의 pos를 초기화, shape과 curShape, color는 rand()로 초기화, flag은 0으로 초기화(한 번이라도 움직이면 1로 변경)
*	copyBlock은 인자로 들어온 구조체의 내용을 그대로 복사한 새로운 구조체를 반환합니다.
*/
Block* setNewBlock(COORD pos);
Block* copyBlock(Block* selBlock);
void updateBlock(Block* selBlock);

/*
*	블럭을 그려주고, 굳히고, 지우는 파트입니다. drawBlock과 drawBuffBlock은 기본적으로 인자로 들어온 블럭을 정해진 위치와 정해진 색으로 칠합니다.
*	eraseBlock은 인자로 들어온 블럭의 위치에서 블럭을 지우고 배열 STAGE에서의 블럭의 좌표를 0으로 수정. 블럭을 굳히는 solidifyBlock은 배열 STAGE에서의 
*	블럭의 좌표를 인자의 color로 홀수 x좌표를 수정, 짝수는 0으로 수정합니다. drawBlock에서 2차원 배열 STAGE에서의 블럭의 좌표를 인자의 color로 짝수 x좌표를 수정합니다.(블럭이 없다면 0 -> 칠해지면 색상 값 2 ~ 15)
*/
void drawBlock(Block* selBlock);
void drawBuffBlock(Block* selBlock);
void solidifyBlock(Block* selBlock);
void eraseBlock(Block* selBlock);

/*
*	블럭의 예상 경로를 확인하기 위한 파트입니다.
*	drawExtra은 selBlock의 예상되는 최종 위치에 블럭을 그려줍니다.
*	eraseExtraBlock은 블럭의 위치가 갱신될 때마다 예상 블럭을 지워줍니다.
*/
void drawExtraBlock(Block* selBlock);
void eraseExtraBlock(Block* selBlock);

/*
*	블럭을 회전시키고, 움직이고, 충돌 판정을 내리는 파트입니다.
*	rotateBlock과 moveBlock은 각각 다음 도형, 다음 위치를 임시 구조체에 설정한 후 detectCollision을 통해 판정하여 수행합니다.(충돌 시 회전, 이동 불가)
*	detectCollision에서는 인자의 위치에 해당하는 배열 STAGE의 홀수 x좌표의 값을 확인 후, 블럭 충돌 시 1, 벽 충돌 시 2, 충돌이 없으면 0을 반환합니다.
*/
void rotateBlock(Block* selBlock);
void moveBlock(Block* selBlock, int key);
int detectCollision(Block* selBlock);

/*
*	전체적인 부분을 관장하는 파트입니다.
*	eraseAll은 화면의 모든 글자를 지웁니다. (게임 종료 시 깔끔하게 보이기 위해 존재) removeFilledBlock은 줄 채워짐을 확인하고, 
*	지운 후 그에 맞는 extraScore을 SCORE에 더해줍니다.(STAGE 홀수 x의 값에 0이나 1이 있으면 채워지지 않은 것)
*/
void eraseAll();
void removeFilledBlock();

/*
*	전체적인 디자인을 담당하는 파트입니다. drawStage와 drawBuffStage은 가시되는 바운더리를 생성하고, drawStage에서는 STAGE도 초기화합니다.
*	drawUserInfo에서는 점수, 레벨, 시간, 특수 키 설명, 버전을 플레이어에게 보여줍니다.
*	stageInfo는 Debugging용으로 사용합니다. (STAGE의 상태를 보여줌)
*/
void drawStage();
void drawBuffStage();
void drawUserInfo();
void stageInfo();

/*
*	초기 설정과 필수 기능을 담당하는 파트입니다. setCursorPosition은 원하는 x,y 좌표로 cursor을 이동시킵니다.
*	init은 기본적인 초기 세팅을 합니다.(커서 깜빡임 제거, rand의 시드 초기화, Stage 생성)
*	getKeyBoard는 키보드 입력을 받고, 해당되는 기능의 함수를 호출합니다.(방향키, 스페이스, ESC, TAB)
*/
void setCursorPosition(int x, int y);
void init();
void getKeyboard();

/*
*	이벤트를 담당하는 파트입니다. levelUpdate는 전역변수의 LV의 상태를 확인하고 변동이 있다면 "Level Up"으로 간주하여 1을 반환(아니라면 0을 반환)
*	levelUpdate가 1을 반환하면 levelEvent와 deleteEvent를 번갈아 실행(문구 깜빡임 효과). gameStart는 게임이 바로 실행되지 않고, 플레이어가
*	아무 키를 눌러야 게임이 시작되도록 합니다.(문구 깜빡임 효과) gameOver도 마찬가지로 아무 키를 눌러야 게임이 종료되도록 합니다.(최종 점수와 문구 깜빡임)
*/
int levelUpdate();
void levelEvent();
void deleteEvent();
void gameStart();
int gameOver();


//	Global variable Field

//   For Debugging -> 우측 바운더리를 위해 2를 더 함
int STAGE[INT_HEIGHT + 1][INT_WIDTH + 2] = { 0 };
//   Score
int SCORE = 0;
//	Level
int LV = 0;
float TIME = 0.6f;
//	Play Time
double STARTTIME = 0;

Block* selBlock, * buffBlock;

//   Main
int main(void) {
	init();

	COORD centerPos = { INT_WIDTH / 4 - 1,1 };
	COORD buffPos = { INT_WIDTH / 2 + 4,2 };

	selBlock = setNewBlock(centerPos);
	buffBlock = setNewBlock(buffPos);

	//	플레이 타임 측정
	double preTime = clock();

	drawUserInfo();
	drawStage();

	gameStart();
	//	조금의 딜레이 형성
	Sleep(500);

	//	게임 시작 시간 check
	STARTTIME = clock();

	while (1) {
		//	Level Update
		if (levelUpdate()) {
			for (int i = 0; i < 3; i++) {
				levelEvent();
				Sleep(500);
				deleteEvent();
				Sleep(400);
			}
		}

		drawUserInfo();

		if (selBlock != NULL) {
			drawBuffStage();

			drawExtraBlock(selBlock);
			drawBlock(selBlock);
			drawBuffBlock(buffBlock);

			//   Keyboard 입력 확인
			getKeyboard();

			//   TIME초마다 Y 값이 1씩 증가
			if ((clock() - preTime) / CLOCKS_PER_SEC > TIME) {
				Block* tmp = copyBlock(selBlock);
				tmp->pos.Y++;
				updateBlock(tmp);

				if (detectCollision(tmp)) {
					if (selBlock->flag == 0) {
						eraseExtraBlock(tmp);
						free(selBlock);
						free(tmp);
						if (gameOver()) {	//	종료하려면 무조건 진입
							eraseAll();
							setCursorPosition(0, 0);
							return 0;
						}
					}

					//   Sliding 구현
					getKeyboard();
					Sleep(200);

					//	Block Solidify
					eraseExtraBlock(selBlock);
					drawBlock(selBlock);
					solidifyBlock(selBlock);
					free(selBlock);
					selBlock = NULL;
					free(tmp);

					continue;
				}
				else {
					//   움직임 판정 -> 게임 오버가 되지 않도록 함
					selBlock->flag = 1;

					eraseBlock(selBlock);
					eraseExtraBlock(selBlock);
					selBlock->pos.Y++;
					updateBlock(selBlock);

					drawExtraBlock(selBlock);
					drawBlock(selBlock);

					preTime = clock();   //   time Reset

					free(tmp);
				}
			}

			Sleep(10);
		}

		else {	//	selBlock이 없을 경우
			//	Level Update
			if (levelUpdate()) {
				for (int i = 0; i < 3; i++) {
					levelEvent();
					Sleep(500);
					deleteEvent();
					Sleep(400);
				}
			}

			removeFilledBlock();
			//   조금의 딜레이 형성
			Sleep(10);

			//   블럭 교체
			selBlock = buffBlock;
			eraseBlock(buffBlock);
			selBlock->pos = centerPos;

			buffBlock = setNewBlock(buffPos);

			drawExtraBlock(selBlock);
			drawBlock(selBlock);
			drawBlock(buffBlock);
		}
	}

	free(selBlock);
	return 0;
}

//	x, y 좌표 입력 시 cursor의 위치를 조정함
void setCursorPosition(int x, int y) {
	HANDLE hConsoleout = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { x,y };
	SetConsoleCursorPosition(hConsoleout, pos);
}

//	새로운 Block을 입력된 pos로 생성해서 주소를 반환하는 함수
Block* setNewBlock(COORD pos) {
	int idx = (rand() % 7) * 4;

	Block* newBlock = (Block*)malloc(sizeof(Block));
	if (newBlock == NULL) {
		newBlock = setNewBlock(pos);
	}
	newBlock->pos = pos;
	newBlock->shape = idx;
	newBlock->curShape = idx;
	newBlock->flag = 0;

	newBlock->color = (rand() % 14) + 2;	//	색상 : 2 ~ 15

	updateBlock(newBlock);

	return newBlock;
}

//	기본적인 초기 설정을 진행(커서 깜빡임 제거, rand의 시드 초기화, Stage 생성)
void init() {
	srand(time(NULL));

	//   init
	HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO curCursorInfo;
	GetConsoleCursorInfo(hConsoleOut, &curCursorInfo);
	curCursorInfo.bVisible = 0;
	SetConsoleCursorInfo(hConsoleOut, &curCursorInfo);

	//   Stage 그리기
	drawStage();
	drawBuffStage();
}

//	입력된 selBlock을 정해진 색상으로 정해진 위치에 그리는 함수
void drawBlock(Block* selBlock) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), selBlock->color);
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[selBlock->curShape][y][x] == 1) {
				setCursorPosition(2 * (selBlock->pos.X + x), selBlock->pos.Y + y);
				printf("■");

				//   Area 밖으로 안 나가도록 Check
				if (selBlock->pos.Y + y <= INT_HEIGHT && 2 * (selBlock->pos.X + x) + 1 < INT_WIDTH + 2) {
					STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x)] = selBlock->color;
				}
			}
		}
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
}

//	selBlock의 예상 위치를 보여주는 함수
void drawExtraBlock(Block* selBlock) {
	Block* tmp = copyBlock(selBlock);

	while (1) {
		if (detectCollision(tmp)) {
			tmp->pos.Y--;
			updateBlock(tmp);
			break;
		}
		tmp->pos.Y++;
		updateBlock(tmp);
	}
	
	//	Draw
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), tmp->color);
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[tmp->curShape][y][x] == 1) {
				setCursorPosition(2 * (tmp->pos.X + x), tmp->pos.Y + y);
				printf("□");
			}
		}
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

	free(tmp);
}


//	입력된 selBlock(buffBlock)을 buffStage에 정해진 색상으로 그리는 함수
void drawBuffBlock(Block* selBlock) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), selBlock->color);
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[selBlock->curShape][y][x] == 1) {
				setCursorPosition(2 * (selBlock->pos.X + x), selBlock->pos.Y + y);
				printf("■");
			}
		}
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
}

//	고정되야하는 selBlock을 입력으로 받아 굳히는 함수
void solidifyBlock(Block* selBlock) {
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[selBlock->curShape][y][x] == 1) {
				if (selBlock->pos.Y + y <= INT_HEIGHT && 2 * (selBlock->pos.X + x) + 1 < INT_WIDTH + 2) {
					STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x)] = 0;
					STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x) + 1] = selBlock->color;
				}
			}

		}
	}
}

//	입력받은 selBlock을 지워버리는 함수
void eraseBlock(Block* selBlock) {
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[selBlock->curShape][y][x] == 1) {
				setCursorPosition(2 * (selBlock->pos.X + x), selBlock->pos.Y + y);
				printf("  ");
				if (selBlock->pos.Y + y <= INT_HEIGHT && 2 * (selBlock->pos.X + x) + 1 < INT_WIDTH + 2) {
					STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x)] = 0;
					STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x) + 1] = 0;
				}
			}
		}
	}
}

//	입력받은 selBlock을 지워버리는 함수(문자만 삭제)
void eraseExtraBlock(Block* selBlock) {
	Block* tmp = copyBlock(selBlock);

	while (1) {
		if (detectCollision(tmp)) {
			tmp->pos.Y--;
			updateBlock(tmp);
			break;
		}
		tmp->pos.Y++;
		updateBlock(tmp);
	}

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[tmp->curShape][y][x] == 1) {
				setCursorPosition(2 * (tmp->pos.X + x), tmp->pos.Y + y);
				printf("  ");
			}
		}
	}

	free(tmp);
}

//	Collision을 정확하게 체크하기 위해 입력 받은 selBlock에서 칠해지는 좌표를 저장하는 함수
void updateBlock(Block* selBlock) {
	int idx = 0;

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[selBlock->curShape][y][x] == 1) {
				selBlock->elem[idx].X = selBlock->pos.X + x;
				selBlock->elem[idx].Y = selBlock->pos.Y + y;
				idx++;
			}
		}
	}
}

//	입력 받은 selBlock을 임시 변수 tmp로 충돌을 확인 후 회전시키는 함수
void rotateBlock(Block* selBlock) {
	//   재설정 이전 모양 확인
	Block* tmp = copyBlock(selBlock);
	tmp->curShape = (tmp->curShape + 1) % 4 + selBlock->shape;
	updateBlock(tmp);

	// 충돌 확인
	int flag = detectCollision(tmp);
	free(tmp);

	if (flag == 1) {   //   벽 충돌
		return;
	}
	else if (flag == 2) {   //   블럭 충돌
		return;
	}

	//   재설정
	selBlock->curShape = (selBlock->curShape + 1) % 4 + selBlock->shape;
	updateBlock(selBlock);
}

//	입력 받은 selBlock과 키보드를 확인하며 위치 이동이나 회전을 관장하는 함수
void moveBlock(Block* selBlock, int key) {
	//   재설정 이전 모양 확인
	Block* tmp = copyBlock(selBlock);
	updateBlock(tmp);

	//   재설정
	switch (key) {
	case 75:	//	Left
		tmp->pos.X--;
		updateBlock(tmp);
		if (!detectCollision(tmp)) { selBlock->pos.X--; updateBlock(selBlock); selBlock->flag = 1; }
		break;
	case 77:	//	Right
		tmp->pos.X++;
		updateBlock(tmp);
		if (!detectCollision(tmp)) { selBlock->pos.X++; updateBlock(selBlock); selBlock->flag = 1; }
		break;
	case 80:	//	Down
		tmp->pos.Y++;
		updateBlock(tmp);
		if (!detectCollision(tmp)) {
			selBlock->pos.Y++;
			updateBlock(selBlock);
			selBlock->flag = 1;
			SCORE += 1;
			drawUserInfo();
		}
		break;
	case 72:	//	Up
		rotateBlock(selBlock); 
		break;	
	case 32:	//	Space
		while (1) {
			if (detectCollision(tmp)) {
				if (selBlock->pos.Y != tmp->pos.Y - 1) {
					selBlock->flag = 1;
				}
				selBlock->pos.Y = tmp->pos.Y - 1;
				updateBlock(selBlock);
				break;
			}
			tmp->pos.Y++;
			updateBlock(tmp);

			//	점수 무제한 증가 버그 방지
			if (!detectCollision(tmp)) {
				SCORE += 1;
				drawUserInfo();
			}
		}
		break;
	}

	free(tmp);
}

//	키보드 입력을 받고, 이에 대한 대응을 나누어 시행하는 함수
void getKeyboard() {
	int key = 0;

	if (_kbhit()) {
		key = _getch();

		if (key == VK_SPACE) {	//	아래로 끝까지 이동
			eraseBlock(selBlock);
			eraseExtraBlock(selBlock);
			moveBlock(selBlock, key);
			drawBlock(selBlock);
			drawExtraBlock(selBlock);
		}
		else if (key == 224) {	//	방향키 입력 -> moveBlock으로 연결
			key = _getch();

			eraseBlock(selBlock);
			eraseExtraBlock(selBlock);
			moveBlock(selBlock, key);
			drawBlock(selBlock);
			drawExtraBlock(selBlock);
		}
		else if (key == VK_ESCAPE) {	//	일시정지
			while (1) {
				setCursorPosition(INT_WIDTH / 5 * 2, INT_HEIGHT / 2);
				printf("<Pause>");
				if (_kbhit()) {
					_getch();
					setCursorPosition(INT_WIDTH / 5 * 2, INT_HEIGHT / 2);
					printf("          ");
					break;
				}
				Sleep(1000);

				setCursorPosition(INT_WIDTH / 5 * 2, INT_HEIGHT / 2);
				printf("           ");
				if (_kbhit()) {
					_getch();
					break;
				}
				Sleep(1000);
			}
		}
		else if (key == VK_TAB) {	//	buffBlock과 Switch
			eraseExtraBlock(selBlock);
			eraseBlock(selBlock);
			eraseBlock(buffBlock);
			//	Swap
			COORD posTmp = selBlock->pos;
			selBlock->pos = buffBlock->pos;
			buffBlock->pos = posTmp;

			Block* tmp = selBlock;
			selBlock = buffBlock;
			buffBlock = tmp;

			drawExtraBlock(selBlock);
			drawBlock(selBlock);
			drawBuffBlock(buffBlock);
		}
	}
}

//	디버깅을 위해 Stage 상태를 확인하는 함수
void stageInfo() {
	COORD pos = { INT_WIDTH * 2,0 };
	for (int y = 0; y <= INT_HEIGHT; y++) {
		for (int x = 0; x <= INT_WIDTH + 1; x++) {
			setCursorPosition(pos.X + x, pos.Y + y);
			printf("%d", STAGE[y][x]);
		}
	}
}

//	플레이 Stage를 그리는 함수
void drawStage() {
	COORD pos = { 0,0 };

	HANDLE tempConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleCursorPosition(tempConsoleOut, pos);

	//   Left Border
	for (int y = 0; y < INT_HEIGHT; y++) {
		printf("┃");
		STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X + 1] = 1;
		pos.Y++;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}

	//   LeftPoint
	printf("┖");
	STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X + 1] = 1;
	pos.X++;
	SetConsoleCursorPosition(tempConsoleOut, pos);

	//   Bottom Border
	for (int x = 1; x < INT_WIDTH; x++) {
		printf("━");
		STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X + 1] = 1;
		pos.X++;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}

	//   RightPoint
	printf("┛");
	STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X + 1] = 1;
	pos.Y--;
	SetConsoleCursorPosition(tempConsoleOut, pos);

	//   Right Border
	for (int y = 0; y < INT_HEIGHT; y++) {
		printf("┃");
		STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X + 1] = 1;
		pos.Y--;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}
}

//	buffBlock이 보여지는 Buff Stage를 그리는 함수
void drawBuffStage() {
	COORD pos = { INT_MENU_X,0 };
	int Y = 6;

	HANDLE tempConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleCursorPosition(tempConsoleOut, pos);

	//   RightPoint
	printf("┏");
	pos.Y++;
	SetConsoleCursorPosition(tempConsoleOut, pos);

	//   Left Border
	for (int y = 0; y <= Y; y++) {
		printf("┃");
		pos.Y++;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}

	//   LeftPoint
	printf("┖");
	pos.X++;
	SetConsoleCursorPosition(tempConsoleOut, pos);

	//   Bottom Border
	for (int x = 1; x <= INT_MENU_WIDTH; x++) {
		printf("━");
		pos.X++;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}

	//   RightPoint
	printf("┛");
	pos.Y--;
	SetConsoleCursorPosition(tempConsoleOut, pos);

	//   Right Border
	for (int y = 0; y <= Y; y++) {
		printf("┃");
		pos.Y--;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}

	printf("┓");
	pos.X--;
	SetConsoleCursorPosition(tempConsoleOut, pos);


	//   Upper Border
	for (int x = 1; x <= INT_MENU_WIDTH; x++) {
		printf("━");
		pos.X--;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}
}

//	점수, 레벨, 플레이 타임, 기능 키들과 수정 날짜를 보여주는 함수
void drawUserInfo() {
	setCursorPosition(INT_MENU_X, 10);
	printf("                            ");   //   Erase
	setCursorPosition(INT_MENU_X, 10);
	printf("Current Score | %d", SCORE);

	setCursorPosition(INT_MENU_X, 12);
	printf("                            ");   //   Erase
	setCursorPosition(INT_MENU_X, 12);
	printf("Current Level | %d / 9", LV + 1);
	if (LV == 8) printf(" <MAX LEVEL>");

	setCursorPosition(INT_MENU_X, 14);
	printf("                            ");   //   Erase
	setCursorPosition(INT_MENU_X, 14);
	printf("Play Time | %02d:%02d", (int)((clock() - STARTTIME) / CLOCKS_PER_SEC / 60), ((int)((clock() - STARTTIME) / CLOCKS_PER_SEC) % 60));

	setCursorPosition(INT_MENU_X, 17);
	printf("                            ");   //   Erase
	setCursorPosition(INT_MENU_X, 17);
	printf("Tab | Switch Block");

	setCursorPosition(INT_MENU_X, 19);
	printf("                            ");   //   Erase
	setCursorPosition(INT_MENU_X, 19);
	printf("ESC | Pause");

	setCursorPosition(INT_MENU_X, 24);
	printf("                                    ");   //   Erase
	setCursorPosition(INT_MENU_X, 24);
	printf("▶Tetris◀ Latest update 20240929");
}

//   입력 받은 selBlock이 다른 블럭이나 Stage와의 충돌이 있는지 확인하고, 충돌 시 1, 2 없을 시에는 0을 반환
int detectCollision(Block* selBlock) {

	for (int i = 0; i < 4; i++) {
		if (STAGE[selBlock->elem[i].Y][2 * selBlock->elem[i].X + 1] > 1) {   //   블럭 충돌을 우선 파악
			return 1;
		}
		else if (STAGE[selBlock->elem[i].Y][2 * selBlock->elem[i].X + 1] == 1) {
			return 2;
		}
	}

	return 0;
}

//	selBlock의 다음 단계(이동, 회전 등)을 파악하기 위해 새로운 Block 구조체를 할당받고 반환
Block* copyBlock(Block* selBlock) {
	Block* newBlock = (Block*)malloc(sizeof(Block));
	if (newBlock == NULL) {
		newBlock = copyBlock(selBlock);
	}
	newBlock->pos = selBlock->pos;
	newBlock->shape = selBlock->shape;
	newBlock->curShape = selBlock->curShape;
	newBlock->flag = selBlock->flag;
	newBlock->color = selBlock->color;

	updateBlock(newBlock);
	return newBlock;
}

//	채워진 줄을 확인하고 지운 후, 그에 맞는 점수를 부여하는 함수
void removeFilledBlock() {
	int cnt = 0;

	for (int y1 = INT_HEIGHT; y1 >= 1; y1--) {
		int flag = 1;

		for (int x1 = 3; x1 < INT_WIDTH; x1 += 2) {
			//   1이나 0이 있으면 채워지지 않은 것.
			if (STAGE[y1][x1] == 1 || STAGE[y1][x1] == 0) {
				flag = 0;
				break;
			}
		}

		if (flag == 1) {
			cnt++;

			for (int y2 = y1; y2 >= 1; y2--) {
				for (int x2 = 1; x2 < INT_WIDTH / 2; x2++) {
					STAGE[y2][2 * x2 + 1] = STAGE[y2 - 1][2 * x2 + 1];

					setCursorPosition(2 * x2, y2);
					if (STAGE[y2][2 * x2 + 1] == 0) {
						printf("  ");
					}
					else {
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), STAGE[y2][2 * x2 + 1]);
						printf("■");
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
					}
				}
				y1++;
			}
		}
	}

	//   점수 환산 공식
	if (cnt > 0) {
		int extraScore = int(exp((double)cnt * 0.9) * 20);

		if (extraScore > 0)
			SCORE += extraScore;
		drawUserInfo();

		//	플레이어가 몇 줄을 채웠는지 Display
		for (int i = 0; i < 2; i++) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), cnt + 1);
			setCursorPosition(INT_WIDTH / 5 * 2, INT_HEIGHT / 2);
			printf("Combo X %d!!", cnt);
			Sleep(1000);

			setCursorPosition(INT_WIDTH / 5 * 2, INT_HEIGHT / 2);
			printf("              ");
			Sleep(800);

			//	원래 블럭을 다시 display
			int y = INT_HEIGHT / 2;
			for (int x = 1; x < INT_WIDTH / 2; x++) {
				setCursorPosition(2 * x, y);
				if (STAGE[y][2 * x + 1] == 0) {
					printf("  ");
				}
				else {
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), STAGE[y][2 * x + 1]);
					printf("■");
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				}
			}
		}
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	}


	drawUserInfo();
}

//	Level이 올랐는지 판단하고, Time값을 수정 후 flag을 값을 반환(1: 레벨 변동 있음, 0: 없음)
int levelUpdate() {

	int flag = 0;
	if (LV != SCORE / 300 && SCORE / 300 <= 8) flag = 1;

	//	300점 당 Level +1 -> 0.6s ~ 0.23s (난이도가 초반에 빠르게 오름)
	LV = SCORE / 300;

	if (LV >= 9) LV = 8;

	TIME = (float)3 / (LV + 5);

	return flag;
}

//	Level up을 하면 화면에 문구를 띄우는 함수
void levelEvent() {
	//	화면에 Level UP 이벤트를 줌
	setCursorPosition(INT_WIDTH / 5 * 2, INT_HEIGHT / 2);

	printf("Level Up!");
}

//	level event에서 등장한 문구를 삭제하고, 원래 있던 블럭을 다시 보여주는 함수
void deleteEvent() {
	setCursorPosition(INT_WIDTH / 5 * 2, INT_HEIGHT / 2);
	printf("            ");

	int y = INT_HEIGHT / 2;

	//	원래 블럭을 다시 display
	for (int x = 1; x < INT_WIDTH / 2; x++) {
		setCursorPosition(2 * x, y);
		if (STAGE[y][2 * x + 1] == 0) {
			printf("  ");
		}
		else {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), STAGE[y][2 * x + 1]);
			printf("■");
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		}
	}
}

//	화면의 전체를 지우는 함수
void eraseAll() {
	COORD pos = { 0, 0 };
	for (int y = 0; y < 100; y++) {
		for (int x = 0; x < 100; x++) {
			setCursorPosition(2 * (pos.X + x), pos.Y + y);
			printf("  ");
		}
	}
}

//	게임 시작할 때, 개선된 플레이어 경험을 위해 아무 키를 눌러야 시작되도록 하는 함수
void gameStart() {
	while (1) {
		setCursorPosition(INT_WIDTH / 8 * 1, INT_HEIGHT / 2);
		printf("Press any key to start the game.");
		if (_kbhit()) {
			setCursorPosition(INT_WIDTH / 8 * 1, INT_HEIGHT / 2);
			printf("                                ");
			//	space bar에 의한 오류 방지
			_getch();
			break;
		}
		Sleep(1000);

		setCursorPosition(INT_WIDTH / 8 * 1, INT_HEIGHT / 2);
		printf("                                ");
		if (_kbhit()) {
			//	space bar에 의한 오류 방지
			_getch();
			break;
		}
		Sleep(800);
	}

	return;
}

//	게임 오버되었을 때, 문구와 최종 점수를 띄우고 키를 누르면 1을 반환하여 함수를 종료
int gameOver() {
	while (1) {
		setCursorPosition(INT_WIDTH / 5 * 2, INT_HEIGHT / 2);
		printf("Game Over");

		setCursorPosition(INT_WIDTH / 7 * 2, INT_HEIGHT / 2 + 1);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), LV + 1);
		printf("Final Score : %d", SCORE);

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		setCursorPosition(INT_WIDTH / 8 * 1, INT_HEIGHT / 2 + 2);
		printf("Press any key to quit the game.");
		if (_kbhit()) {
			break;
		}
		Sleep(2000);

		setCursorPosition(INT_WIDTH / 5 * 2, INT_HEIGHT / 2);
		printf("                   ");
		setCursorPosition(INT_WIDTH / 7 * 2, INT_HEIGHT / 2 + 1);
		printf("                   ");
		setCursorPosition(INT_WIDTH / 8 * 1, INT_HEIGHT / 2 + 2);
		printf("                                ");

		//	원래 블럭을 다시 display
		for (int y = INT_HEIGHT / 2; y <= INT_HEIGHT / 2 + 2; y++) {
			for (int x = 1; x < INT_WIDTH / 2; x++) {
				setCursorPosition(2 * x, y);
				if (STAGE[y][2 * x + 1] == 0) {
					printf("  ");
				}
				else {
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), STAGE[y][2 * x + 1]);
					printf("■");
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				}
			}
		}

		if (_kbhit()) {
			break;
		}

		Sleep(800);
	}

	return 1;
}