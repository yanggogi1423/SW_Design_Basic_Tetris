#include <stdio.h>

#include "block.h"
#include <windows.h>
#include <time.h>
#include <conio.h>

#include <limits.h>

#define INT_HEIGHT 25
#define INT_WIDTH 36

//	For Debugging
int STAGE[INT_HEIGHT + 1][INT_WIDTH + 1] = { 0 };

typedef struct Block {
	COORD pos;
	COORD BR, TL;

	COORD elem[4];

	int shape;
	int curShape;
}Block;

void getKeyboard(Block* selBlock);
void updateBlock(Block* selBlock);
void updateBlock(Block* selBlock);
int getValue(Block* selBlock, int mode);

void setCursorPosition(int x, int y) {
	HANDLE hConsoleout = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { x,y };
	SetConsoleCursorPosition(hConsoleout, pos);
}

void drawBlock(Block* selBlock) {
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[selBlock->curShape][y][x] == 1) {
				setCursorPosition(2 * (selBlock->pos.X + x), selBlock->pos.Y + y);
				printf("■");

				if (selBlock->pos.Y + y <= INT_HEIGHT && 2 * (selBlock->pos.X + x) <= INT_WIDTH)
					STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x)] = 2; STAGE[selBlock->pos.Y + y][2 * (selBlock->pos.X + x) + 1] = 2;
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
		STAGE[pos.Y][pos.X] = 1; STAGE[pos.Y][pos.X - 1] = 1;
		pos.Y--;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}
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

//	RETURN 1 : 하단 벽&하단 블럭, 2 : 왼쪽 벽, 3 : 오른쪽 벽, 4 : 사이드 블럭
int detectCollision(Block* selBlock) {
	
	//	아래
	int maxY = getValue(selBlock, 2);

	for (int i = 0; i < 4; i++) {
		if (maxY == selBlock->elem[i].Y) {
			if (STAGE[maxY + 1][2 * selBlock->elem[i].X] == 1) {
				return 1;
			}
		}
	}

	//	왼쪽
	int minX = getValue(selBlock, 1);

	for (int i = 0; i < 4; i++) {
		if (minX == selBlock->elem[i].X) {
			if (STAGE[selBlock->elem[i].Y][2 * (minX - 1)] == 1) {
				return 4;
			}
		}
	}

	//	오른쪽
	int maxX = getValue(selBlock, 0);

	for (int i = 0; i < 4; i++) {
		if (maxX == selBlock->elem[i].X) {
			if (STAGE[selBlock->elem[i].Y][2 * (maxX + 1)] == 1) {
				return 4;
			}
		}
	}

	//	STAGE 벗어남
	if (selBlock->BR.Y >= INT_HEIGHT) {
		getKeyboard(selBlock);
		return 1;
	}
	else if (selBlock->TL.X - 1 <= 0) {
		return 2;
	}
	else if (selBlock->BR.X + 1 >= INT_WIDTH / 2) {
		return 3;
	}

	return 0;
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

	selBlock->BR.X = getValue(selBlock, 0);
	selBlock->TL.X = getValue(selBlock, 1);
	selBlock->BR.Y = getValue(selBlock, 2);
	selBlock->TL.Y = getValue(selBlock, 3);
}

int getValue(Block* selBlock, int mode) {
	//	0 : maxX, 1 : minX, 2 : maxY, 3 : minY
	int dst = 0;

	switch (mode) {
	case 0:
		dst = INT_MIN;
		for (int i = 0; i < 4; i++)
			if (dst < selBlock->elem[i].X)
				dst = selBlock->elem[i].X;
		break;
	case 1:
		dst = INT_MAX;
		for (int i = 0; i < 4; i++)
			if (dst > selBlock->elem[i].X)
				dst = selBlock->elem[i].X;
		break;
	case 2:
		dst = INT_MIN;
		for (int i = 0; i < 4; i++)
			if (dst < selBlock->elem[i].Y)
				dst = selBlock->elem[i].Y;
		break;
	case 3:
		dst = INT_MAX;
		for (int i = 0; i < 4; i++)
			if (dst > selBlock->elem[i].Y)
				dst = selBlock->elem[i].Y;
		break;
	}

	return dst;
}

Block* copyBlock(Block* selBlock) {
	Block* newBlock = (Block*)malloc(sizeof(Block));
	newBlock->pos = selBlock->pos;
	newBlock->shape = selBlock->shape;
	newBlock->curShape = selBlock->curShape;

	updateBlock(newBlock);

	return newBlock;
}

void rotateBlock(Block* selBlock) {
	//	재설정 이전 모양 확인
	Block* tmp = copyBlock(selBlock);

	tmp->curShape = (tmp->curShape + 1) % 4 + selBlock->shape;
	updateBlock(tmp);

	int tmpFlag = detectCollision(tmp);

	int flag = detectCollision(selBlock);

	for (int i = 0; i < 2; i++) {
		if (flag == 1) {	//	하단 이탈
			selBlock->pos.Y = INT_HEIGHT;
		}
		else if (flag == 2) {	//	좌측 이탈
			selBlock->pos.X = 1;
		}
		else if (flag == 3) {	//	우측 이탈
			selBlock->pos.X -= (INT_WIDTH / 2 - selBlock->BR.X);
		}
		else {
			break;
		}
	}

	//	재설정
	selBlock->curShape = (selBlock->curShape + 1) % 4 + selBlock->shape;
	updateBlock(selBlock);

	// 충돌 확인
}

void moveBlock(Block* selBlock, int key) {
	switch (key) {
	case 75:
		if (detectCollision(selBlock) != 2 && detectCollision(selBlock) != 4) selBlock->pos.X--; updateBlock(selBlock);
		break;
	case 77:
		if (detectCollision(selBlock) != 3 && detectCollision(selBlock) != 4) selBlock->pos.X++; updateBlock(selBlock);
		break;
	case 80:
		if (detectCollision(selBlock) != 1) selBlock->pos.Y++; updateBlock(selBlock);
		break;
	case 72: rotateBlock(selBlock); break;
	}


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
		for (int x = 0; x <= INT_WIDTH + 1; x++) {
			setCursorPosition(pos.X + x, pos.Y + y);
			printf("%d", STAGE[y][x]);
		}
	}
}

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
				if (detectCollision(selBlock) == 1) {
					getKeyboard(selBlock);
					Sleep(400);
					free(selBlock);
					selBlock = NULL;

					continue;
				}
				eraseBlock(selBlock);
				selBlock->pos.Y++;
				drawBlock(selBlock);

				updateBlock(selBlock);
				preTime = clock();	//	Reset
			}

			Sleep(10);
		}
		else {
			selBlock = setNewBlock(centerPos);
			drawBlock(selBlock);
		}
	}

	return 0;
}