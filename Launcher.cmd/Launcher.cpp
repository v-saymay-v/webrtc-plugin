// Launcher.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"
#include <iostream>
#include <string>
#include <vector>
#include <codecvt>
//#include <conio.h>
#include <stdlib.h>
#include <processthreadsapi.h>
#include <windows.h>

BOOL execute(std::string program, std::string commandLine)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
	std::wstring wprogram = cv.from_bytes(program);
	std::wstring wcommand = cv.from_bytes(commandLine);
/*
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	return CreateProcess(wprogram.c_str(), (LPTSTR)wcommand.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
*/
	return (ShellExecute(NULL, L"open", (LPTSTR)wcommand.c_str(), NULL, NULL, SW_SHOWNORMAL) != NULL);
}

int main()
{
    //std::cout << "Hello World!\n"; 
	std::string s;
	std::string ss;
	std::vector< std::string > vecLines;

	CHAR full[_MAX_PATH];
	GetCurrentDirectoryA(_MAX_PATH, full);
#ifdef _DEBUG
	strcat_s(full, "\\..\\Broadcast\\bin\\Debug\\Broadcast.exe");
#else
	strcat_s(full, "\\Broadcast.exe");
#endif
	s = full;
	ss = s;

	while (std::cin.gcount() > 0) {
		// read in every line of stdin   
		std::string line;
		getline(std::cin, line);

		std::vector< std::string >::const_iterator it;
		for (it = vecLines.begin(); it != vecLines.end(); ++it) {
			s += std::string(" ");
			s += *it;
		}
	}
	MessageBoxA(NULL, ss.c_str(), s.c_str(), MB_OK);
	std::cerr << ss << ":" << s;
	BOOL bResult = execute(ss, s);
	if (!bResult) {
		int err = GetLastError();
		std::cerr << "can not start process:" << err;
	}
	char ch;
	std::cin.get(ch);
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
