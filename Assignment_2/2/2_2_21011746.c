#include <stdio.h>

#include "block.h"
#include <windows.h>
#include <time.h>
#include <conio.h>

#include <limits.h>

#define INT_HEIGHT 25
#define INT_WIDTH 37

//	For Debugging -> 우측 바운더리를 위해 2를 더 함
int STAGE[INT_HEIGHT + 1][INT_WIDTH + 2] = { 0 };

typedef struct Block {
	COORD pos;
//	COORD BR, TL;

	COORD elem[4];

	int shape;
	int curShape;
}Block;

void setCursorPosition(int x, int y);
void drawBlock(Block* selBlock);
void solidifyBlock(Block* selBlock);
void eraseBlock(Block* selBlock);
void drawStage();
void init();
int detectCollision(Block* selBlock);
Block* setNewBlock(COORD pos);
void updateBlock(Block* selBlock);
Block* copyBlock(Block* selBlock);
void rotateBlock(Block* selBlock);
void moveBlock(Block* selBlock, int key);
void getKeyboard(Block* selBlock);
void stageInfo();

int main(void) {
	init();

	COORD centerPos = { INT_WIDTH / 6,3 };

	Block* selBlock = setNewBlock(centerPos);

	//	Clock()을 통해 시간을 계산
	double preTime = clock();

	while (1) {

		if (selBlock != NULL) {
			drawStage();
			drawBlock(selBlock);

			//	For Debugging
			stageInfo();

			//	Keyboard 입력 확인
			getKeyboard(selBlock);

			//	1초마다 Y값이 1씩 증가
			if ((clock() - preTime) / CLOCKS_PER_SEC > 1) {
				Block* tmp = copyBlock(selBlock);
				tmp->pos.Y++;
				updateBlock(tmp);

				if (detectCollision(tmp)) {
					//	Sliding
					getKeyboard(selBlock);
					Sleep(400);

					solidifyBlock(selBlock);
					free(selBlock);
					selBlock = NULL;
					free(tmp);
					continue;
				}
				else {
					eraseBlock(selBlock);
					selBlock->pos.Y++;
					updateBlock(selBlock);
					drawBlock(selBlock);

					preTime = clock();	//	Reset

					free(tmp);
				}
			}


			Sleep(10);
		}
		else {
			selBlock = setNewBlock(centerPos);
			drawBlock(selBlock);
		}
	}

	free(selBlock);
	return 0;
}

void setCursorPosition(int x, int y) {
	HANDLE hConsoleout = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { x,y };
	SetConsoleCursorPosition(hConsoleout, pos);
}

Block* setNewBlock(COORD pos) {
	int idx = (rand() % 7) * 4;

	Block* newBlock = (Block*)malloc(sizeof(Block));
	newBlock->pos = pos;
	newBlock->shape = idx;
	newBlock->curShape = idx;

	//	Bottom Right, Top Left 확인

	updateBlock(newBlock);

	return newBlock;
}

void init() {
	srand(time(NULL));

	//	init
	HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO curCursorInfo;
	GetConsoleCursorInfo(hConsoleOut, &curCursorInfo);
	curCursorInfo.bVisible = 0;
	SetConsoleCursorInfo(hConsoleOut, &curCursorInfo);

	//	Stage 그리기
	drawStage();
}

void drawBlock(Block* selBlock) {
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[selBlock->curShape][y][x] == 1) {
				setCursorPosition(2 * (selBlock->pos.X + x), selBlock->pos.Y + y);
				printf("■");

				//	Area 밖으로 안 나가도록 Check
				if (selBlock->pos.Y + y <= INT_HEIGHT && 2 * (selBlock->pos.X + x) <= INT_WIDTH)
					STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x)] = 2; STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x) + 1] = 2;
			}
		}
	}
}

void solidifyBlock(Block* selBlock) {
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[selBlock->curShape][y][x] == 1) {
				if (selBlock->pos.Y + y <= INT_HEIGHT && 2 * (selBlock->pos.X + x) <= INT_WIDTH)
					STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x)] = 3; STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x) + 1] = 3;
			}
		}
	}
}

void eraseBlock(Block* selBlock) {
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[selBlock->curShape][y][x] == 1) {
				setCursorPosition(2 * (selBlock->pos.X + x), selBlock->pos.Y + y);
				printf("  ");

				STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x)] = 0; STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x) + 1] = 0;
			}
		}
	}
}

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

void rotateBlock(Block* selBlock) {
	//	재설정 이전 모양 확인
	Block* tmp = copyBlock(selBlock);
	tmp->curShape = (tmp->curShape + 1) % 4 + selBlock->shape;
	updateBlock(tmp);

	// 충돌 확인
	int flag = detectCollision(tmp);
	free(tmp);

	if (flag == 1) {	//	벽 충돌
		return;
	}
	else if (flag == 2) {	//	블럭 충돌
		return;
	}

	//	재설정
	selBlock->curShape = (selBlock->curShape + 1) % 4 + selBlock->shape;
	updateBlock(selBlock);
}

void moveBlock(Block* selBlock, int key) {
	//	재설정 이전 모양 확인
	Block* tmp = copyBlock(selBlock);
	updateBlock(tmp);

	//	재설정
	switch (key) {
	case 75:
		tmp->pos.X--;
		updateBlock(tmp);
		if (!detectCollision(tmp)) selBlock->pos.X--; updateBlock(selBlock);
		break;
	case 77:
		tmp->pos.X++;
		updateBlock(tmp);
		if (!detectCollision(tmp)) selBlock->pos.X++; updateBlock(selBlock);
		break;
	case 80:
		tmp->pos.Y++;
		updateBlock(tmp);
		if (!detectCollision(tmp)) selBlock->pos.Y++; updateBlock(selBlock);
		break;
	case 72: rotateBlock(selBlock); break;
	}

	free(tmp);
}

void getKeyboard(Block* selBlock) {
	int key = 0;
	if (_kbhit()) {
		key = _getch();

		if (key == 224) {
			key = _getch();

			eraseBlock(selBlock);
			moveBlock(selBlock, key);
			drawBlock(selBlock);
		}
	}
}

void stageInfo() {
	COORD pos = { INT_WIDTH + 10,0 };
	for (int y = 0; y <= INT_HEIGHT + 1; y++) {
		for (int x = 0; x <= INT_WIDTH + 2; x++) {
			setCursorPosition(pos.X + x, pos.Y + y);
			printf("%d", STAGE[y][x]);
		}
	}
}

void drawStage() {
	COORD pos = { 0,0 };

	HANDLE tempConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleCursorPosition(tempConsoleOut, pos);

	//	Left Border
	for (int y = 0; y <= INT_HEIGHT; y++) {
		printf("┃");
		STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X + 1] = 1;
		pos.Y++;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}

	//	LeftPoint
	printf("┖");
	STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X + 1] = 1;
	pos.X++;
	SetConsoleCursorPosition(tempConsoleOut, pos);

	//	Bottom Border
	for (int x = 1; x <= INT_WIDTH; x++) {
		printf("━");
		STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X + 1] = 1;
		pos.X++;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}

	//	RightPoint
	printf("┛");
	STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X + 1] = 1;
	pos.Y--;
	SetConsoleCursorPosition(tempConsoleOut, pos);

	//	Right Border
	for (int y = 0; y <= INT_HEIGHT; y++) {
		printf("┃");
		STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X + 1] = 1;
		pos.Y--;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}
}

//	RETURN 1 : 벽 충돌 2 : 블럭 충돌
int detectCollision(Block* selBlock) {

	for (int i = 0; i < 4; i++) {
		if (STAGE[selBlock->elem[i].Y][2 * selBlock->elem[i].X] == 3) {	//	블럭 충돌을 우선 파악
			return 1;
		}
		else if (STAGE[selBlock->elem[i].Y][2 * selBlock->elem[i].X] == 1) {
			return 2;
		}
	}

	return 0;
}

Block* copyBlock(Block* selBlock) {
	Block* newBlock = (Block*)malloc(sizeof(Block));
	newBlock->pos = selBlock->pos;
	newBlock->shape = selBlock->shape;
	newBlock->curShape = selBlock->curShape;

	return newBlock;
}
