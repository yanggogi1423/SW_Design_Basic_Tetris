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
				cout << "■";
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


	//	커서 깜빡임 제거 -> 현재 표시된 커서의 활성화 해제
	CONSOLE_CURSOR_INFO curCursorInfo;

	//	현재 커서의 상태 정보를 가져오기
	GetConsoleCursorInfo(hConsoleOut, &curCursorInfo);

	//	커서의 보임 표시 끄고 상태 정보를 콘솔에 업데이트
	curCursorInfo.bVisible = 0;
	SetConsoleCursorInfo(hConsoleOut, &curCursorInfo);

	COORD pos1 = { 6,6 }, pos2 = { 0,10 };

	drawBlock(pos1, 4);
	drawBlock(pos2, 12);


	return 0;
}