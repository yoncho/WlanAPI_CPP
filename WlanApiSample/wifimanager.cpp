#include "wifimanager.h"

#include <windows.h>
#include <string>
#include <wlanapi.h>
#include <list>
#include <iostream>
#include <codecvt>
#pragma comment(lib, "wlanapi.lib")
using namespace std;

/*
* wifi-manager.cpp create by yoncho [23.07.24]
* wlanapi.h documents url : https://learn.microsoft.com/en-us/windows/win32/api/wlanapi/
*/

HANDLE handle = NULL;
DWORD negotiatedVersion = 0;
DWORD* flags = NULL;
DWORD* grantedAccess = NULL;
PVOID reserved = NULL;
PDOT11_SSID dot11Ssid = NULL;
PWLAN_RAW_DATA wlanRawData = NULL;
PWLAN_INTERFACE_INFO_LIST interfaceList = NULL;
PWLAN_PROFILE_INFO selectedProfile = NULL;
PWLAN_AVAILABLE_NETWORK_LIST availableNetworkList = NULL;
PWLAN_INTERFACE_INFO selectedInterface = 0;
PWLAN_PROFILE_INFO_LIST profileList = NULL;
PWLAN_INTERFACE_CAPABILITY interfaceCapavility = NULL;


//#1 Open Handle 
bool OpenHandle(DWORD clientVersion, string* errorDescription)
{
	/* WlanOpenHandle()`s info
	* - [INPUT][DWORD] clientVersion :
	*	value
	*		1 : Client version for Windows XP with SP3 and Wireless LAN API for Windows XP with SP2.
	*		2 : Client version for Windows Vista and Windows Server 2008
	*	 Description
	*		value 2를 사용.
	* - [INPUT][PVOID] reserved : input NULL, reserved parameter
	*	Description
	*		향후 사용을 위해 예약되었습니다. NULL로 설정해야 합니다.
	* - [OUTPUT][PDWORD] negotiatedVersion :
	*	Description
	*		이 세션에서 사용될 WLAN API의 버전입니다. 이 값은 일반적으로 클라이언트와 서버 모두에서 지원하는 가장 높은 버전입니다.
	* - [OUTPUT][PHANDLE] clientHandle
	*	Description
	*		Open handle success일 경우, handle에 값 지정
	* Return
	* - [OUTPUT][DWORD]
	*	value
	*		[PASS] ERROR_SUCCESS (0L) : open handle success
	*		[FAIL] ERROR_INVALID_PARAMETER (87L) : parameter error
	*		[FAIL] ERROR_NOT_ENOUGH_MEMORY (8L) : memory error
	*		[FAIL] RPC_STATUS (not define) : various error codes.
	*		[FAIL] ERROR_REMOTE_SESSION_LIMIT_EXCEEDED (1220L) : session error
	*/
	long result = WlanOpenHandle(clientVersion, reserved, &negotiatedVersion, &handle);
	*errorDescription = ConvertWlanErrToString(result);
	return result == ERROR_SUCCESS ? true : false;
}

//#2 Search Interface List 
// - save interface list in local parameter (interfaceList)
bool SearchInterfaces(string* errorDescription)
{
	/* WlanEnumInterfaces()`s info
	* - [INPUT][HANDLE] handle :
	*	 Description
	*		OpenHandle()을 통해 설정된 handle 값 사용.
	* - [INPUT][PVOID] reserved : input NULL, reserved parameter
	*	Description
	*		향후 사용을 위해 예약되었습니다. NULL로 설정해야 합니다.
	* - [OUTPUT][PWLAN_INTERFACE_INFO_LIST] *ppInterfaceList :
	*	Description
	*		Interface 정보를 포함한 List가 반환.
	* Return
	* - [OUTPUT][DWORD]
	*	value
	*		[PASS] ERROR_SUCCESS (0L) : open handle success
	*		[FAIL] ERROR_INVALID_PARAMETER (87L) : parameter error
	*		[FAIL] ERROR_INVALID_HANDLE (6L) : handle error
	*		[FAIL] ERROR_NOT_ENOUGH_MEMORY (8L) : memory error
	*		[FAIL] RPC_STATUS (not define) : various error codes.
	*		[FAIL] ERROR_REMOTE_SESSION_LIMIT_EXCEEDED (1220L) : session error
	*/
	DWORD result = WlanEnumInterfaces(handle, reserved, &interfaceList);
	*errorDescription = ConvertWlanErrToString(result);
	return result == ERROR_SUCCESS ? true : false;
}

//#3 Interface Name List 
// - return string type list (ex. Intel(R) Wi-Fi 6 AX201 160MHz)
list<string> GetInterfaceNameList(string* errorDescription)
{
	list<string> result = {};
	if (interfaceList->dwNumberOfItems > 0)
	{
		for (DWORD i = 0; i < interfaceList->dwNumberOfItems; ++i) {
			result.push_back(ConverterWcharToString(interfaceList->InterfaceInfo[i].strInterfaceDescription));
		}
	}
	return result;
}

//#4 Search Interface Capavility
PWLAN_INTERFACE_CAPABILITY GetInterfaceCapavility(DWORD interfaceIndex, string* errorDescription)
{
	selectedInterface = (WLAN_INTERFACE_INFO*)&interfaceList->InterfaceInfo[interfaceIndex];

	DWORD result = WlanGetInterfaceCapability(handle, &selectedInterface->InterfaceGuid, reserved, &interfaceCapavility);
	*errorDescription = ConvertWlanErrToString(result);
	return interfaceCapavility;
}

//#5 Search Available Network List in Selected Interface (User select)
// - save network list in local parameter (interfaceList)
bool SearchAvailableNetworks(DWORD interfaceIndex, string* errorDescription)
{
	selectedInterface = (WLAN_INTERFACE_INFO*)&interfaceList->InterfaceInfo[interfaceIndex];

	DWORD result = WlanScan(handle, &selectedInterface->InterfaceGuid, dot11Ssid, wlanRawData, reserved);
	if (result == ERROR_SUCCESS)
	{
		result = WlanGetAvailableNetworkList(handle, &selectedInterface->InterfaceGuid, WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES, NULL, &availableNetworkList);
	}
	*errorDescription = ConvertWlanErrToString(result);
	return result == ERROR_SUCCESS ? true : false;
}

//#6 Newtork Name List 
// - return string type list (ex. CRYPT)
list<string> GetNetworkNameList(string* errorDescription)
{
	list<string> result = {};
	if (availableNetworkList->dwNumberOfItems > 0)
	{
		for (DWORD i = 0; i < availableNetworkList->dwNumberOfItems; ++i) {
			result.push_back(ConverterDot11SsidToString(availableNetworkList->Network[i].dot11Ssid));
		}
	}
	return result;
}

//#7 Detail in Selected Network (User selected)
PWLAN_AVAILABLE_NETWORK GetNetworkInfo(DWORD networkIndex, string* errorDescription)
{
	return &availableNetworkList->Network[networkIndex];
}

//#8 Search Profile List in Selected Interface (User selected)
// - save profile list in local parameter (profileList)
bool SearchProfiles(DWORD interfaceIndex, string* errorDescription)
{
	selectedInterface = (WLAN_INTERFACE_INFO*)&interfaceList->InterfaceInfo[interfaceIndex];

	DWORD result = WlanGetProfileList(handle, &selectedInterface->InterfaceGuid, reserved, &profileList);
	*errorDescription = ConvertWlanErrToString(result);
	return result == ERROR_SUCCESS ? true : false;
}

//#9 Profile Name List 
// - return string type list (ex. CRYPT)
list<string> GetProfileNameList(string* errorDescription)
{
	list<string> result = {};
	if (profileList->dwNumberOfItems > 0)
	{
		for (DWORD i = 0; i < profileList->dwNumberOfItems; ++i) {
			result.push_back(ConverterWcharToString(profileList->ProfileInfo[i].strProfileName));
		}
	}
	return result;
}

//#10 Detail(XML) in Selected Profile(User selected)
// - return LPWSTR* profileXML
bool GetProfileInfo(DWORD interfaceIndex, DWORD profileIndex, LPWSTR* profileXml, string* errorDescription)
{
	selectedInterface = (WLAN_INTERFACE_INFO*)&interfaceList->InterfaceInfo[interfaceIndex];
	selectedProfile = (WLAN_PROFILE_INFO*)&profileList->ProfileInfo[profileIndex];
	DWORD result = WlanGetProfile(handle, &selectedInterface->InterfaceGuid, selectedProfile->strProfileName, reserved, profileXml, flags, grantedAccess);
	*errorDescription = ConvertWlanErrToString(result);
	return result == ERROR_SUCCESS ? true : false;
}


//#11 Delete Profile (User selected)
// - delete profile (*현재 연결중인 wifi의 profile을 제거할 시 자동으로 연결이 해제됨.)
bool DeleteProfile(DWORD interfaceIndex, DWORD profileIndex, string* errorDescription)
{
	selectedInterface = (WLAN_INTERFACE_INFO*)&interfaceList->InterfaceInfo[interfaceIndex];
	selectedProfile = (WLAN_PROFILE_INFO*)&profileList->ProfileInfo[profileIndex];
	DWORD result = WlanDeleteProfile(handle, &selectedInterface->InterfaceGuid, selectedProfile->strProfileName, reserved);
	*errorDescription = ConvertWlanErrToString(result);
	return result == ERROR_SUCCESS ? true : false;
}

//#12 Set Profile (User input)
bool SetProfile(DWORD interfaceIndex, string ssid, string password, string* errorDescription)
{
	selectedInterface = (WLAN_INTERFACE_INFO*)&interfaceList->InterfaceInfo[interfaceIndex];
	DWORD reasonCode = NULL;
	LPWSTR profileXML = CreateProfileXML(ssid, password);
	DWORD result = WlanSetProfile(handle, &selectedInterface->InterfaceGuid, WLAN_PROFILE_USER, profileXML, NULL, true, NULL, &reasonCode);
	//TODO : wchar_t -> string 변환 (utf16 -> utf8) 해야함.  
	// *errorDescription = ConverterReasonCodeToString(reasonCode);
	*errorDescription = ConvertWlanErrToString(result);
	return result == ERROR_SUCCESS ? true : false;
}

//#13 Connect Network by Profile 
bool ConnectNetwork(DWORD interfaceIndex, DWORD profileIndex, string* errorDescription)
{
	selectedInterface = (WLAN_INTERFACE_INFO*)&interfaceList->InterfaceInfo[interfaceIndex];
	selectedProfile = (WLAN_PROFILE_INFO*)&profileList->ProfileInfo[profileIndex];

	//Connection Parameter Setting
	LPCWSTR ssid = selectedProfile->strProfileName;
	WLAN_CONNECTION_PARAMETERS connectionParams;
	ZeroMemory(&connectionParams, sizeof(WLAN_CONNECTION_PARAMETERS));
	connectionParams.wlanConnectionMode = wlan_connection_mode_profile;
	connectionParams.strProfile = ssid;
	connectionParams.pDot11Ssid = nullptr;
	connectionParams.pDesiredBssidList = nullptr;
	connectionParams.dot11BssType = dot11_BSS_type_infrastructure;
	connectionParams.dwFlags = 1;

	DWORD result = WlanConnect(handle, &selectedInterface->InterfaceGuid, &connectionParams, reserved);
	*errorDescription = ConvertWlanErrToString(result);
	return result == ERROR_SUCCESS ? true : false;
}

//#14 Disconnect Network
bool DisconnectNetwork(DWORD interfaceIndex, string* errorDescription)
{
	selectedInterface = (WLAN_INTERFACE_INFO*)&interfaceList->InterfaceInfo[interfaceIndex];

	DWORD result = WlanDisconnect(handle, &selectedInterface->InterfaceGuid, reserved);
	*errorDescription = ConvertWlanErrToString(result);
	return result == ERROR_SUCCESS ? true : false;
}

//#15 Close Handle
bool CloseHandle(string* errorDescription)
{
	DWORD result = WlanCloseHandle(handle, reserved);
	*errorDescription = ConvertWlanErrToString(result);
	return result == ERROR_SUCCESS ? true : false;
}

//Converter
string ConverterWcharToString(WCHAR* wcharData)
{
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wcharData);
}

PCHAR ConverterStringToPchar(string stringData)
{
	const char* cstr = stringData.c_str();
	return const_cast<char*>(cstr);
}

string ConverterDot11SsidToString(const DOT11_SSID& ssid)
{
	string strSsid(reinterpret_cast<const char*>(ssid.ucSSID), ssid.uSSIDLength);
	return strSsid;
}

LPWSTR ConvertStringToLPWSTR(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	LPWSTR lpwstr = new WCHAR[size_needed];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, lpwstr, size_needed);
	return lpwstr;
}

string ConvertWlanErrToString(long errorCode)
{
	switch (errorCode)
	{
	case ERROR_SUCCESS:
		return "Success [0]";
	case ERROR_ACCESS_DENIED:
		return "ERROR [5] : Access denied";
	case ERROR_INVALID_HANDLE:
		return "ERROR [6] : Invalid handle";
	case ERROR_NOT_ENOUGH_MEMORY:
		return "ERROR [8] : Not enough memory";
	case ERROR_NOT_SUPPORTED:
		return "ERROR [50] : Not supported";
	case ERROR_INVALID_PARAMETER:
		return "ERROR [87] : Invalid parameter";
	case ERROR_ALREADY_EXISTS:
		return "ERROR [183] : Already exists";
	case ERROR_NOT_FOUND:
		return "ERROR [1168] : Not found";
	case ERROR_NO_MATCH:
		return "ERROR [1169] : No match";
	case ERROR_BAD_PROFILE:
		return "ERROR [1206] : Bad profile";
	case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED:
		return "ERROR [1220] : Remote session limit exceeded";
	default:
		return "";
	}
}

string ConverterReasonCodeToString(DWORD reasonCode)
{
	string result = NULL;
	wchar_t buffer[1024] = { 0 };
	DWORD bufferSize = sizeof(buffer) / sizeof(wchar_t);

	DWORD data = WlanReasonCodeToString(reasonCode, bufferSize, buffer, NULL);
	size_t length = wcslen(buffer);
	std::wstring wstr(buffer, buffer + length);
	//TODO wstring to string 변환 해야함.
	// reasonCode를 WlanReasonCodeToString()으로 변환한 wchar_t buffer에 reason이 표시됨.
	// 하지만 wstring to string 변환 작업을 하지 못 함.
	return result;
}

LPWSTR CreateProfileXML(string ssid, string password)
{
	string xmlString =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n"
		"    <name>" + ssid + "</name>\n"
		"    <SSIDConfig>\n"
		"        <SSID>\n"
		"            <name>" + ssid + "</name>\n"
		"        </SSID>\n"
		"    </SSIDConfig>\n"
		"    <connectionType>ESS</connectionType>\n"
		"    <connectionMode>auto</connectionMode>\n"
		"    <MSM>\n"
		"        <security>\n"
		"            <authEncryption>\n"
		"                <authentication>WPA2PSK</authentication>\n"
		"                <encryption>AES</encryption>\n"
		"                <useOneX>false</useOneX>\n"
		"            </authEncryption>\n"
		"            <sharedKey>\n"
		"                <keyType>passPhrase</keyType>\n"
		"                <protected>false</protected>\n"
		"                <keyMaterial>" + password + "</keyMaterial>\n"
		"            </sharedKey>\n"
		"        </security>\n"
		"    </MSM>\n"
		"</WLANProfile>";

	return ConvertStringToLPWSTR(xmlString);
}
