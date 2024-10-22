#include <iostream>
using namespace std;

#include "block.h"
#include <windows.h>

void drawBlock(COORD pos, int idx) {
	pos.X *= 2;

	HANDLE tempConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleCursorPosition(tempConsoleOut, pos);

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (blockModel[idx][y][x] == 1) {
				cout << "��";
			}
			else {
				cout << "  ";
			}
		}
		pos.Y += 1;
		tempConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleCursorPosition(tempConsoleOut, pos);
	}
}

int main(void) {

	//	HANDLE : 
	HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);


	//	Ŀ�� ������ ���� -> ���� ǥ�õ� Ŀ���� Ȱ��ȭ ����
	CONSOLE_CURSOR_INFO curCursorInfo;

	//	���� Ŀ���� ���� ������ ��������
	GetConsoleCursorInfo(hConsoleOut, &curCursorInfo);

	//	Ŀ���� ���� ǥ�� ���� ���� ������ �ֿܼ� ������Ʈ
	curCursorInfo.bVisible = 0;
	SetConsoleCursorInfo(hConsoleOut, &curCursorInfo);

	COORD pos1 = { 6,6 }, pos2 = { 0,10 };

	drawBlock(pos1, 4);
	drawBlock(pos2, 12);


	return 0;
}