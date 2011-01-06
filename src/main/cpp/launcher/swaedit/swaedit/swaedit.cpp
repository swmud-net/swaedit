// swaedit.cpp : Defines the entry point for the application.
//

#include "swaedit.h"

using namespace std;

static ofstream logFile("swaedit_exe.log");
static CHAR currentDir[MAX_PATH] = { 0 };

LPCSTR findJava()
{
	if (!_access(JAVAW6_PF, 0))
		return JAVAW6_PF;
	else if (!_access(JAVAW6_PF_86, 0))
		return JAVAW6_PF_86;
	else if (!_access(JAVA6_PF, 0))
		return JAVA6_PF;
	else if (!_access(JAVA6_PF_86, 0))
		return JAVA6_PF_86;
	else
		return NULL;
}

bool launchSwaedit(LPCSTR javaExe)
{
	const int NUM_CURRDIR_INJECTIONS = 7;

	bool launched = false;

	if (*currentDir)
	{
		logFile << "current directory: " << currentDir << endl;

		const size_t LEN = strlen(currentDir)*NUM_CURRDIR_INJECTIONS+1024;
		LPSTR lpCommandLine = new CHAR[LEN];
		strcpy_s(lpCommandLine, LEN, " -Djava.library.path=\"");
		
		strcat_s(lpCommandLine, LEN, currentDir);
		strcat_s(lpCommandLine, LEN, "\\lib\" -cp \"");

		strcat_s(lpCommandLine, LEN, currentDir);
		strcat_s(lpCommandLine, LEN, "\\lib\\gluegen-rt.jar;");

		strcat_s(lpCommandLine, LEN, currentDir);
		strcat_s(lpCommandLine, LEN, "\\lib\\jogl.all-noawt.jar;");

		strcat_s(lpCommandLine, LEN, currentDir);
		strcat_s(lpCommandLine, LEN, "\\lib\\nativewindow.all-noawt.jar;");

		strcat_s(lpCommandLine, LEN, currentDir);
		strcat_s(lpCommandLine, LEN, "\\lib\\newt.all-noawt.jar;");

		strcat_s(lpCommandLine, LEN, currentDir);
		strcat_s(lpCommandLine, LEN, "\\lib\\qtjambi.jar;");

		strcat_s(lpCommandLine, LEN, currentDir);
		strcat_s(lpCommandLine, LEN, "\\bin\" pl.swmud.ns.swaedit.gui.SWAEdit");

		logFile << "running: " << javaExe << lpCommandLine << endl;

		PROCESS_INFORMATION pInfo;
		STARTUPINFOA sInfo;
		memset(&pInfo, 0, sizeof(PROCESS_INFORMATION));
		memset(&sInfo, 0, sizeof(STARTUPINFOA));
		sInfo.cb = sizeof(STARTUPINFOA);

		if (CreateProcessA(javaExe, lpCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &sInfo, &pInfo))
		{
			WaitForSingleObject(pInfo.hProcess, 3000);
			DWORD exitCode;
			if (GetExitCodeProcess(pInfo.hProcess, &exitCode))
			{
				logFile << "exit code: " << exitCode << endl;
				if (exitCode == STILL_ACTIVE || exitCode == ERROR_SUCCESS)
				{
					launched = true;
				}
			}
		}
		else
		{
			logFile << "process not created: " << GetLastError() << endl;
		}

		delete lpCommandLine;
	}

	return launched;
}

LPCSTR getJava()
{
	OPENFILENAMEA ofn;
	CHAR szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);

	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "javaw.exe\0javaw.exe\0java.exe\0java.exe\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	
	LPSTR javaExe = NULL;
	if (GetOpenFileNameA(&ofn))
	{
		const size_t LEN = strlen(ofn.lpstrFile)+1;
		javaExe = new CHAR[LEN];
		strcpy_s(javaExe, LEN, ofn.lpstrFile);
	}
	if (!SetCurrentDirectoryA(currentDir))
	{
		logFile << "unable to get current directory" << endl;
	}

	return javaExe;
}

void logDirTree(LPCSTR dir)
{
	const size_t LEN = MAX_PATH*2+1;
	CHAR dirName[LEN] = { 0 };
	strcpy_s(dirName, LEN, dir);
	strcat_s(dirName, LEN, "\\*");
	WIN32_FIND_DATAA fData;
	HANDLE fh = FindFirstFileA(dirName, &fData);

	if (fh != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (strcmp(fData.cFileName, ".") && strcmp(fData.cFileName, ".."))
			{
				if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					logFile << dir << "\\" << fData.cFileName << "\\ " << fData.nFileSizeLow << " " << fData.nFileSizeHigh << endl;
					strcpy_s(dirName, LEN, dir);
					strcat_s(dirName, LEN, "\\");
					strcat_s(dirName, LEN, fData.cFileName);
					logDirTree(dirName);
				}
				else
				{
					logFile << dir << "\\" << fData.cFileName << " " << fData.nFileSizeLow << " " << fData.nFileSizeHigh << endl;
				}
			}
		} while (FindNextFileA(fh, &fData) != 0);
		FindClose(fh);
	}
}

bool swaeditFound()
{
	LPCSTR swaeditPaths[] = {
		"\\bin\\pl\\swmud\\ns\\swaedit\\gui\\SWAEdit.class",
		"\\bin\\pl\\swmud\\ns\\swaedit\\core\\JAXBOperations.class",
		"\\data\\exits.xml",
		"\\schemas\\area.xsd",
		"\\bin\\pl\\swmud\\ns\\swmud\\_1_0\\area\\Area.class",
		"\\lib\\com_trolltech_qt_gui.dll",
		"\\lib\\qtjambi.dll",
		"\\lib\\qtjambi.jar",
		0
	};

	LPSTR path;
	bool allExist = true;
	for (int i = 0; swaeditPaths[i]; ++i)
	{
		const size_t LEN = strlen(currentDir)+strlen(swaeditPaths[i])+1;
		path = new CHAR[LEN];
		strcpy_s(path, LEN, currentDir);
		strcat_s(path, LEN, swaeditPaths[i]);
		if (_access(path, 0))
		{
			allExist = false;
			logFile << "cannot find swaedit REQUIRED file: " << path << endl;
		}
		delete path;
	}

	return allExist;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	if (!GetCurrentDirectoryA(MAX_PATH, currentDir))
	{
		logFile << "unable to get current directory" << endl;
	}

	bool allOk = false;
	bool freeJavaExe = false;
	bool shouldReport = true;

	if (swaeditFound())
	{
		LPCSTR javaExe = findJava();
		
		if (!javaExe)
		{
			if (MessageBoxA(NULL, "Unable to find Java. Do you want to find it manually?", "Java not found!", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
			{
				if (javaExe = getJava())
				{
					freeJavaExe = true;
				}
				else
				{
					logFile << "java binary selection cancelled" << endl;
					shouldReport = false;
				}
			}
			else
			{
				logFile << "manual search not accepted" << endl;
				shouldReport = false;
			}
		}

		if (javaExe && launchSwaedit(javaExe))
		{
			allOk = true;
		}
		else
		{
			logFile << "swaedit not launched" << endl;
			if (shouldReport)
			{
				logDirTree(currentDir);
			}
		}

		if (javaExe && freeJavaExe)
		{
			delete javaExe;
		}
	}

	logFile.flush();
	logFile.close();

	if (allOk)
	{
		return EXIT_SUCCESS;
	}
	else
	{
		if (shouldReport)
		{
			MessageBoxA(NULL, "Check the swaedit_exe.log for details. Maybe the problem is obvious?\nIf not, send swaedit_exe.log to the swmud.pl team.",
				"Unable to launch swaedit!", MB_OK | MB_ICONERROR);
		}
		return EXIT_FAILURE;
	}

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
}
