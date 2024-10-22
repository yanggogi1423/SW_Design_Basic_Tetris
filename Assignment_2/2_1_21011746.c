#include <stdio.h>

#include "block.h"
#include <windows.h>
#include <time.h>
#include <conio.h>

typedef struct Block {
	COORD pos;
	int shape;
	int curShape;
}Block;

void drawBlock(Block* tmp) {
	COORD pos = tmp->pos;
	pos.X *= 2;

	HANDLE tempConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleCursorPosition(tempConsoleOut, pos);

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[tmp->curShape][y][x] == 1) {
				printf("■");
			}
			else {
				printf("  ");
			}
		}
		pos.Y += 1;
		tempConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}
}

void eraseBlock(Block* tmp) {
	COORD pos = tmp->pos;
	pos.X *= 2;

	HANDLE tempConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleCursorPosition(tempConsoleOut, pos);

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			printf("  ");
		}
		pos.Y += 1;
		tempConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}
}

void drawStage(int h, int w) {
	COORD pos = { 0,0 };

	HANDLE tempConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	char col = '|';
	char row = '-';
	char point = '+';

	SetConsoleCursorPosition(tempConsoleOut, pos);

	//	Left Border
	for (int y = 0; y <= h; y++) {
		printf("%c",col);
		pos.Y++;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}

	//	LeftPoint
	printf("%c",point);
	pos.X++;
	SetConsoleCursorPosition(tempConsoleOut, pos);

	//	Bottom Border
	for (int x = 1; x <= w + 1; x++) {
		printf("%c",row);
		pos.X++;
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}

	//	RightPoint
	printf("%c",point);
	pos.Y--;
	SetConsoleCursorPosition(tempConsoleOut, pos);

	//	Right Border
	for (int y = 1; y <= h + 1; y++) {
		printf("%c",col);
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
	drawStage(25, 30);
}

Block* setNewBlock(COORD pos) {
	int idx = (rand() % 7) * 4;

	Block* newBlock = (Block*)malloc(sizeof(Block));
	newBlock->pos = pos;
	newBlock->shape = idx;
	newBlock->curShape = idx;

	return newBlock;
}

Block* moveBlock(Block* tmp, int key) {

	eraseBlock(tmp);

	switch (key)
	{
	case 75:
		tmp->pos.X--;
		break;
	case 77:
		tmp->pos.X++;
		break;
	case 80:
		tmp->pos.Y++;
		break;
	case 72:
		tmp->curShape = (tmp->curShape + 1) % 4 + tmp->shape;
		break;
	}

	drawBlock(tmp);

	return tmp;
}

int main(void) {
	init();

	COORD centerPos = { 6,3 };

	Block* tmp = setNewBlock(centerPos);

	//	Clock()을 통해 시간을 계산
	double preTime = clock();

	while (1) {

		eraseBlock(tmp);
		drawBlock(tmp);

		int key = 0;

		if (_kbhit()) {
			key = _getch();

			if (key == 224) {
				key = _getch();

				moveBlock(tmp, key);
			}
		}

		//	1초마다 Y값이 1씩 증가
		if ((clock() - preTime) / CLOCKS_PER_SEC > 1) {
			eraseBlock(tmp);
			tmp->pos.Y++;
			drawBlock(tmp);

			preTime = clock();	//	Reset
		}

		Sleep(10);
	}

	return 0;
}