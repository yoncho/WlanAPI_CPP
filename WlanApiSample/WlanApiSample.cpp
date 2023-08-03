// samplewifi_manager.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include "wifimanager.h"

using namespace std;
DWORD InputParameterToSelectIndex(DWORD sizeOfList, string listName);
string ConvertLPWSTRToString(LPWSTR lpwstr);

int main()
{
    DWORD clientVersion = 2;
    list<string> interfaceNameList;
    string errorDescription;
    DWORD selectedInterfaceIndex = 0;

    //handle open
    if (OpenHandle(clientVersion, &errorDescription))
    {
        //search interface
        if (SearchInterfaces(&errorDescription))
        {
            //interface name list
            interfaceNameList = GetInterfaceNameList(&errorDescription);

            if (interfaceNameList.size() > 0)
            {
                //interfaceNameList 출력
                int i = 0;
                cout << "===== INTERFACE LIST ====" << endl;
                for (const string& str : interfaceNameList) {
                    cout << "[" << i << "] " << str << endl;
                    i++;
                }

                //interface select
                selectedInterfaceIndex = InputParameterToSelectIndex(static_cast<DWORD>(interfaceNameList.size()), "INTERFACE");

                PWLAN_INTERFACE_CAPABILITY interfaceCapavility = GetInterfaceCapavility(selectedInterfaceIndex, &errorDescription);

                if (interfaceCapavility != NULL)
                {
                    cout << "===== INTERFACE INFO =====" << endl;
                    cout << "dwMaxDesiredBssidListSize Type : " << interfaceCapavility->dwMaxDesiredBssidListSize << endl;
                }

                //interface`s network
                if (SearchAvailableNetworks(selectedInterfaceIndex, &errorDescription))
                {
                    list<string> networkNameList = GetNetworkNameList(&errorDescription);

                    if (networkNameList.size() > 0)
                    {
                        int i = 0;
                        cout << "===== WIFI LIST ====" << endl;
                        for (const string& str : networkNameList) {
                            cout << "[" << i << "] " << str << endl;
                            i++;
                        }

                        //network select
                        DWORD selectedNetworkIndex = InputParameterToSelectIndex(static_cast<DWORD>(networkNameList.size()), "NETWORK");

                        PWLAN_AVAILABLE_NETWORK network = GetNetworkInfo(selectedNetworkIndex, &errorDescription);
                        //선택한 network의 ssid 출력
                        cout << ConverterDot11SsidToString(network->dot11Ssid) << endl;
                    }
                }

                //interface`s profile
                if (SearchProfiles(selectedInterfaceIndex, &errorDescription))
                {
                    list<string> profileList = GetProfileNameList(&errorDescription);

                    if (profileList.size() > 0)
                    {
                        int i = 0;
                        cout << "===== PROFILE LIST ====" << endl;
                        for (const string& str : profileList) {
                            cout << "[" << i << "] " << str << endl;
                            i++;
                        }

                        //profile select
                        DWORD selectedProfileIndex = InputParameterToSelectIndex(static_cast<DWORD>(profileList.size()), "PROFILE");

                        //선택한 profile의 xml 출력
                        LPWSTR profileXml;
                        if (GetProfileInfo(selectedInterfaceIndex, selectedProfileIndex, &profileXml, &errorDescription))
                        {
                            cout << "===== PROFILE XML =====" << endl;
                            cout << "Profile XML : \n" << ConvertLPWSTRToString(profileXml) << endl;
                        }

                        //선택한 profile 제거 유무
                        cout << "해당 Profile을 삭제하시겠습니까? (Yes/No) : ";
                        string rescan;
                        getline(std::cin, rescan);
                        for (char& c : rescan) {
                            c = std::toupper(c);
                        }

                        if (rescan == "YES" || rescan == "Y")
                        {
                            if (DeleteProfile(selectedInterfaceIndex, selectedProfileIndex, &errorDescription))
                            {
                                cout << "성공적으로 제거했습니다." << endl;
                            }
                        }

                        //선택한 profile 삭제되었는지 확인
                        if (SearchProfiles(selectedInterfaceIndex, &errorDescription))
                        {
                            list<string> profileList = GetProfileNameList(&errorDescription);

                            if (profileList.size() > 0)
                            {
                                int i = 0;
                                cout << "===== PROFILE LIST ====" << endl;
                                for (const string& str : profileList) {
                                    cout << "[" << i << "] " << str << endl;
                                    i++;
                                }
                            }
                        }

                        //profile 세팅(new create / edit)
                        string ssid = "CRYPT";
                        string password = "crtp12#$";

                        if (SetProfile(selectedInterfaceIndex, ssid, password, &errorDescription))
                        {
                            cout << errorDescription << endl;
                            //profile set 되었는지 확인
                            if (SearchProfiles(selectedInterfaceIndex, &errorDescription))
                            {
                                list<string> profileList = GetProfileNameList(&errorDescription);

                                if (profileList.size() > 0)
                                {
                                    int i = 0;
                                    cout << "===== PROFILE LIST ====" << endl;
                                    for (const string& str : profileList) {
                                        cout << "[" << i << "] " << str << endl;
                                        i++;
                                    }
                                }
                            }
                        }
                        else
                        {
                            cout << errorDescription << endl;
                        }
                    }
                }

                //interface`s connect network
                if (SearchProfiles(selectedInterfaceIndex, &errorDescription))
                {
                    list<string> profileList = GetProfileNameList(&errorDescription);

                    if (profileList.size() > 0)
                    {
                        int i = 0;
                        cout << "===== PROFILE LIST ====" << endl;
                        for (const string& str : profileList) {
                            cout << "[" << i << "] " << str << endl;
                            i++;
                        }

                        //profile select
                        DWORD selectedProfileIndex = InputParameterToSelectIndex(static_cast<DWORD>(profileList.size()), "Profile To Connect");
                        auto startIndex = profileList.begin();
                        advance(startIndex, selectedProfileIndex);

                        //connect network
                        if (ConnectNetwork(selectedInterfaceIndex, selectedProfileIndex, &errorDescription))
                        {
                            cout << "WIFI 연결  성공: " << *startIndex << endl;
                        }
                        else
                        {
                            cout << "WIFI 연결 실패 : " + errorDescription << endl;
                        }

                        //disconnect network
                        if (DisconnectNetwork(selectedInterfaceIndex, &errorDescription))
                        {
                            cout << "WIFI 연결 해제 성공" << endl;
                        }
                        else
                        {
                            cout << "WIFI 연결 해제 실패" << endl;
                        }

                        //close handle
                        if (CloseHandle(&errorDescription))
                        {
                            cout << "handle close 성공" << endl;
                        }
                        else
                        {
                            cout << "handle close 실패" << endl;
                        }
                    }
                }
            }

        }
    }




}


DWORD InputParameterToSelectIndex(DWORD sizeOfList, string listName)
{
    string inputInterface;
    DWORD index;

    while (1) {
        cout << listName << " Index 선택  : ";
        getline(std::cin, inputInterface);
        //입력원에 대한 예외 처리
        try {
            index = stoi(inputInterface);

            if (sizeOfList <= index)
            {
                cout << "Out of range" << endl;
            }
            else
            {
                break;
            }
        }
        catch (const invalid_argument& ex) {
            cout << "Invalid argument: " << ex.what() << endl;
        }
        catch (const out_of_range& ex) {
            cout << "Out of range: " << ex.what() << endl;
        }
    }
    return index;
}


string ConvertLPWSTRToString(LPWSTR lpwstr)
{
    // LPWSTR을 std::wstring으로 변환
    wstring wideString(lpwstr);

    // std::wstring을 UTF-8로 변환하여 std::string으로 저장
    string utf8String(wideString.begin(), wideString.end());

    return utf8String;
}
// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
