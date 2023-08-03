#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H
#include <Windows.h>
#include <wlanapi.h>
#include <iostream>
#include <list>
#include <string>
#include <codecvt>
#pragma comment(lib, "wlanapi.lib")
using namespace std;

extern HANDLE handle;

bool OpenHandle(DWORD clientVersion, string* errorDescription);
bool SearchInterfaces(string* errorDescription);
bool SearchAvailableNetworks(DWORD interfaceIndex, string* errorDescription);
bool SearchProfiles(DWORD interfaceIndex, string* errorDescription);
bool GetProfileInfo(DWORD interfaceIndex, DWORD profileIndex, LPWSTR* profileXml, string* errorDescription);
bool DeleteProfile(DWORD interfaceIndex, DWORD profileIndex, string* errorDescription);
bool SetProfile(DWORD interfaceIndex, string ssid, string password, string* errorDescription);
bool ConnectNetwork(DWORD interfaceIndex, DWORD profileIndex, string* errorDescription);
bool DisconnectNetwork(DWORD interfaceIndex, string* errorDescription);
bool CloseHandle(string* errorDescription);
list<string> GetInterfaceNameList(string* errorDescription);
list<string> GetNetworkNameList(string* errorDescription);
list<string> GetProfileNameList(string* errorDescription);
string ConverterWcharToString(WCHAR* wcharData);
string ConverterDot11SsidToString(const DOT11_SSID& ssid);
string ConvertWlanErrToString(long errorCode);
string ConverterReasonCodeToString(DWORD reasonCode);
PCHAR ConverterStringToPchar(string stringData);
LPWSTR CreateProfileXML(string ssid, string password);
LPWSTR ConvertStringToLPWSTR(const std::string& str);
PWLAN_AVAILABLE_NETWORK GetNetworkInfo(DWORD networkIndex, string* errorDescription);
PWLAN_INTERFACE_CAPABILITY GetInterfaceCapavility(DWORD interfaceIndex, string* errorDescription);

#endif
