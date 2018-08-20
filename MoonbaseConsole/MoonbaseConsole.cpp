// TODO:
// Add a "wrap in ring of water" option to make "non-toroidal" maps.
// Review string sizes and make them more rational.
// Run length encode the WIZ resource.
//
// Version 1.0.6 was ONLY given to POKK (cedricbas@hotmail.com) who disappeared right after I gave it to him...
// Version 1.0.14 Corrected third level crater tile coding error C0 C1 instead of C1 C2
//                Note that there are two crater related differences between these maps and editor-generated maps.
//                   1) We allow craters adjacent to the right and bottom edges of medium and high ground.
//                   2) We correctly handle craters adjacent to the right and bottom edges of low ground where
//                      terrain on the wrapped edge of the map interferes. (The editor makes mangled craters in
//                      this case.)
//                The biggest two map sizes aren't picked randomly anymore (they are nonstandard).
//                Changed energy pool allocation for Spiff style maps
//                Set custom icon for ALT-TAB
//                Removed requirement to replace moon001.map
//                Added sliders to the UI
//
//                Internal Changes:
//                Changed data types of arrays to reduce memory footprint.
//                Removed constructors with debug code in them.
//                Commented out the calibration map stuff.
// Version 1.0.15 Stopped random maps from having zero energy and terrain.
//                Added saving and restoring of settings.
// Version 2.0.16 Limited random ranges of water, energy, and terrain to be 2-4
//                Added check of HEGAMES.INI when registry entry doesn't exist.
//                Updates to both map generators
//                GameInfo version updated to 5
//                Default map number is now set to 1 on launch so don't have to scroll for map
//                IP Address is now saved in the ini file
//                Auto IP detection (started but not finished...)
// Version 2.0.17 Totally redid most of the user interface
//                Added Katton's custom .WIZ generator
// Version 2.0.18 Finished the host game UI.
// Version 2.0.19 Auto unchecked the random boxes when the sliders move.
//                New KattonGen tweaks
// Version 2.0.20 Fixed text alignment on sliders
// Version 2.0.21 Fixed random checkbox on tileset (didn't actually do anything before).
//                New version of Spiff generator from 12/3
// Version 2.0.22 Released source to Spiff and Katton for finalization.
// Version 2.1.23 Updated minor version number
//                Changed YorkDink to YorkdinK
//                updated GAMENUM_VERSION checking so network games work
// Version 2.1.24 Added Bwappo's THM
//                changed credits back to a button..
// Version 2.1.25 Fixed silly bug where wiz not created on client machines.
//         RELEASE
// Version 2.1.26 Fixed bug that broke replays when a standard map was played.
//                Fixed bug where random map not saved on first replay.
// Version 2.2	  Updated domains for finding public IP address
// 		  Added ability to change game options - anything you can change within
// 		  Moonbase Commander's Game Options menu, you can change here too.
// 		  Reverted functionality to find install location, you must put MoonbaseConsole.exe in the same location as Moonbase.exe for this to work.
// 		  Modified UI a little bit.



#include "StdAfx.h"
//#include <atlbase.h>
#include <commctrl.h>
#include <time.h>
#include <wininet.h>
#include <winsock2.h>
#include <shellapi.h>
#include "resource.h"
#include "mbcmif.h"
#include "MBCMapImage.h"
#include "Base64Encode.h"
#include "SpiffGen.h"
#include "KattonGen.h"
#include "GameInfo.h"
#include <stdlib.h>
#include <ctype.h>
#include <set>


// 1.0.15 == 4 (public release)
// 2.0.16 == 5 (leaked release)
// 2.1    == 6 (public release)
// 2.2    == 7 (public release)
#define GAMENUM_VERSION 7

#define NAME_CHAR_LIMIT 10
// ini keys
#define INI_KEY_NAME                        "name"
#define INI_KEY_DAMAGE_BAR                  "0-0"
#define INI_KEY_RANGE_RADIUS                "0-1"
#define INI_KEY_CENTER_UNIT                 "0-2"
#define INI_KEY_NEXT_UNIT                   "0-3"
#define INI_KEY_PREVIOUS_UNIT               "0-4"
#define INI_KEY_CLOSEST_LAUNCHER            "0-5"
#define INI_KEY_SHOW_ATTACKED               "0-6"
#define INI_KEY_CHAT_TOGGLE                 "0-7"
#define INI_KEY_AUTO_SCROLL                 "0-8"
#define INI_KEY_AUTO_HUB_SELECT             "0-9"
#define INI_KEY_SMART_CAMERA                "0-10"
#define INI_KEY_AUTO_RETURN_CAMERA          "0-11"
#define INI_KEY_SFX_VOLUME                  "0-12"
#define INI_KEY_MUSIC_VOLUME                "0-13"
#define INI_KEY_VOICE_VOLUME                "0-14"
#define INI_KEY_INTERFACE_VOLUME            "0-15"
#define INI_KEY_MUSIC_QUALITY               "0-16"
#define INI_KEY_COMMENTARY                  "0-17"

// Default game options
// TODO: Might put all defaults here for neatness' sake
// The codes for each of these are ascii-codes
#define DEFAULT_PLAYER_NAME                 "Commander"
#define DEFAULT_CONTROL_DAMAGE_BAR          100
#define DEFAULT_CONTROL_RANGE_RADIUS        114
#define DEFAULT_CONTROL_CENTER_UNIT         99
#define DEFAULT_CONTROL_NEXT_UNIT           110
#define DEFAULT_CONTROL_PREVIOUS_UNIT       112
#define DEFAULT_CONTROL_CLOSEST_LAUNCHER    108
#define DEFAULT_CONTROL_SHOW_ATTACKED       97
#define DEFAULT_CONTROL_CHAT_TOGGLE         9
#define DEFAULT_CONTROL_AUTO_SCROLL         FALSE
#define DEFAULT_CONTROL_AUTO_HUB_SELECT     FALSE
#define DEFAULT_CONTROL_SMART_CAMERA        FALSE
#define DEFAULT_CONTROL_AUTO_RETURN_CAMERA  FALSE
#define DEFAULT_CONTROL_SFX_VOLUME          255
#define DEFAULT_CONTROL_MUSIC_VOLUME        96
#define DEFAULT_CONTROL_VOICE_VOLUME        196
#define DEFAULT_CONTROL_INTERFACE_VOLUME    196
#define DEFAULT_CONTROL_MUSIC_QUALITY       FALSE
#define DEFAULT_CONTROL_COMMENTARY          TRUE


char szMoonbasePathG[MAX_PATH];
char szMoonbaseIniFileG[MAX_PATH];
PROCESS_INFORMATION piG;
HWND hwndDlgModelessG = NULL; 
HWND changeKeyHwnd;
HICON hIconG = NULL;
HINSTANCE hInstanceG = NULL;
int nReplayNumberHighestG;
CGameInfo giG;
DWORD dwIPAddressG = NULL;

// Game settings variables
char name[NAME_CHAR_LIMIT];
int damageBarPreference;
int rangeRadiusPreference;
int centerUnitPreference;
int nextUnitPreference;
int previousUnitPreference;
int closestLauncherPreference;
int showAttackedPreference;
int chatTogglePreference;
int changedKey;
std::set<int> preferenceSet;

BOOL GetMoonbaseCommanderPath (void)
   {
   /*CRegKey rk;
   if (rk.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\Humongous Entertainment\\Moonbase\\Path") == ERROR_SUCCESS)
      {
      DWORD csz = sizeof(szMoonbasePathG);
      rk.QueryValue(szMoonbasePathG, "", &csz);
      }
   else*/
      {
      // Failed to find the registry key, try HEGAMES.INI

      //char szPath[MAX_PATH];
      //GetWindowsDirectory(szPath, MAX_PATH);
      //// assume GWD succeeded, why would it fail?
      //strcat(szPath, "\\HEGAMES.INI");

      //GetPrivateProfileString("Moonbase", "GameResourcePath", "", szMoonbasePathG, MAX_PATH, szPath);
      
      strcpy(szMoonbasePathG, ".\\"); // Just make it current directory for now 
      }


   strcpy(szMoonbaseIniFileG, szMoonbasePathG);
   strcat(szMoonbaseIniFileG, "moonbase.ini");
	//    I had issues finding the path using the above code, probably because I don't
   // have atlbase.h to look at the registry. I had to use the hacks below to get it to run.
   // Hopefully moving to MinGW might fix this. 
   // It works if you put it in the Moonbase Commander directory though
	   //strcpy(szMoonbasePathG, "C:\\GOG Games\\Moonbase Commander\\");
	   //strcpy(szMoonbaseIniFileG, "C:\\GOG Games\\Moonbase Commander\\Moonbase.ini");

   return TRUE;
   }

int makewiz(int energy, int terrain, int water)
   {
   unsigned short wiz [139][139];
   int i, j;

   HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_WIZ1), "WIZ");
   HGLOBAL hgbl = LoadResource(NULL, hrsrc);
   BYTE *pwiz = (BYTE *)LockResource(hgbl);

   unsigned short *pImage = (unsigned short *)pwiz;
   pImage += 0x0224; // dealing with shorts, so two bytes per
   for (j = 0; j < 139; ++j)
      for (i = 0; i < 139; ++i)
         wiz[i][j] = *pImage++;
   
   for (j=0; j < energy*9; j++)
      {
      for (i = 30; i < 51; i++)
         {
         wiz[i][91 - j] = ((255) / 8 ) + ((int) (130 - (j * 100 / (energy*9))))/8*32 + ((int) (80 + j * 80 / (energy*9))/8*1024);
         }
      }
   
   for (j=0; j < terrain*9; j++)
      {
      for (i = 61; i < 82; i++)
         {
         wiz[i][91 - j] = ((255) / 8 ) + ((int) (130 - (j * 100 / (terrain*9))))/8*32 + ((int) (80 + j * 80 / (terrain*9))/8*1024);
         }
      }
   
   
   for (j=0; j < water*9; j++)
      {
      for (i = 92; i < 113; i++)
         {
         wiz[i][91 - j] = ((255) / 8 ) + ((int) (130 - (j * 100 / (water*9))))/8*32 + ((int) (80 + j * 80 / (water*9))/8*1024);		
         }
      }
   
   char sz[MAX_PATH];
   strcpy(sz, szMoonbasePathG);
   strcat(sz, "\\map\\moon001.wiz");
   FILE *pf;
   pf = fopen(sz, "wb");
   fwrite(pwiz, 1, SizeofResource(NULL, hrsrc), pf);
   fseek(pf, 0x0448, SEEK_SET);
   for  (j = 0; j < 139; j++) {
      for  (i = 0; i < 139; i++) {
         fwrite(&wiz[i][j], 1, 2, pf);
         }
      }
   fclose(pf);
      
   hrsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_THM1), "THM");
   hgbl = LoadResource(NULL, hrsrc);
   BYTE *pthm = (BYTE *)LockResource(hgbl);
   
   strcpy(sz, szMoonbasePathG);
   strcat(sz, "\\map\\moon001.thm");
   pf = fopen(sz, "wb");
   fwrite(pthm, 1, SizeofResource(NULL, hrsrc), pf);
   fclose(pf);
      
   return 0;
   }

void BackupSystemMapFile (void)
   {
   WIN32_FIND_DATA FindFileData;
   HANDLE hFind;
   char szBackup[MAX_PATH];
   char szFile[MAX_PATH];

   strcpy(szFile, szMoonbasePathG);
   strcat(szFile, "map\\moon001.map");
   strcpy(szBackup, szFile);
   strcat(szBackup, ".bak");

   hFind = FindFirstFile(szBackup, &FindFileData);
   if (hFind == INVALID_HANDLE_VALUE) 
      {
      MoveFile(szFile, szBackup);

      strcpy(szFile, szMoonbasePathG);
      strcat(szFile, "map\\moon001.thm");
      strcpy(szBackup, szFile);
      strcat(szBackup, ".bak");
      MoveFile(szFile, szBackup);

      strcpy(szFile, szMoonbasePathG);
      strcat(szFile, "map\\moon001.wiz");
      strcpy(szBackup, szFile);
      strcat(szBackup, ".bak");
      MoveFile(szFile, szBackup);
      }
   }


void RestoreSystemMapFile (void)
   {
   WIN32_FIND_DATA FindFileData;
   HANDLE hFind;
   char szBackup[MAX_PATH];
   char szFile[MAX_PATH];

   strcpy(szFile, szMoonbasePathG);
   strcat(szFile, "map\\moon001.map");
   strcpy(szBackup, szFile);
   strcat(szBackup, ".bak");

   hFind = FindFirstFile(szBackup, &FindFileData);
   if (hFind != INVALID_HANDLE_VALUE) 
      {
      DeleteFile(szFile);
      MoveFile(szBackup, szFile);

      strcpy(szFile, szMoonbasePathG);
      strcat(szFile, "map\\moon001.thm");
      DeleteFile(szFile);
      strcpy(szBackup, szFile);
      strcat(szBackup, ".bak");
      MoveFile(szBackup, szFile);

      strcpy(szFile, szMoonbasePathG);
      strcat(szFile, "map\\moon001.wiz");
      DeleteFile(szFile);
      strcpy(szBackup, szFile);
      strcat(szBackup, ".bak");
      MoveFile(szBackup, szFile);
      }
   }


void PublishGameNumber (CGameInfo& gi, HWND hwnd)
   {
   char sz[1024];

   SecureBase64Encode((unsigned char *)&gi, sizeof(CGameInfo), sz, 1024);

   OpenClipboard(hwnd);
   // TODO: Check that it opened successfully
   EmptyClipboard();
   HGLOBAL hgbl; //eeeeeewwwwww
   hgbl = GlobalAlloc(GMEM_MOVEABLE, strlen(sz) + 1);
   char *psz = (char *)GlobalLock(hgbl);
   strcpy(psz, sz);
   SetClipboardData(CF_TEXT, hgbl);
   CloseClipboard();
   }

void MakeMap(MBCMIF& mif, CGameInfo& gi)
   {
   MBCMapImage mi;

   mif.GenerateMap(mi);

   *((CGameInfo *)&mi.aSpace1) = gi;

   char sz[MAX_PATH];
   strcpy(sz, szMoonbasePathG);
   strcat(sz, "map\\moon001.map");

   FILE *pf;
   pf = fopen(sz, "wb");
   if (pf)
      {
      if (sizeof(MBCMapImage) != fwrite(&mi, 1, sizeof(MBCMapImage), pf))
         throw CGenericError("Could not write entire map file");
      fclose(pf);
      }
   else
      {
      char szError[MAX_PATH + 64];
      sprintf(szError, "Could not open \"%s\" for writing", sz);
      throw CGenericError(szError);
      }
   }

void CleanUpMoonbaseINI (void)
   {
   WritePrivateProfileString("System", "HostIP", NULL, szMoonbaseIniFileG);
   WritePrivateProfileString("System", "JoinIP", NULL, szMoonbaseIniFileG);
   }

void EnableDisableHostButton (HWND hwndDlg)
   {
   DWORD dwAddress;
   SendMessage(GetDlgItem(hwndDlg, IDC_IPADDRESS1), IPM_GETADDRESS, 0, (LPARAM)&dwAddress);

   EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_HOST_GAME), (dwAddress > 0));
   }

void EnableDialog (HWND hwnd, bool bEnable)
   {
   EnableWindow(GetDlgItem(hwnd, IDC_BTN_HOST_GAME), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_BUTTON1), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_BUTTON2), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_BUTTON3), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_BUTTON4), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_SINGLE_GENERATE), bEnable);

   EnableWindow(GetDlgItem(hwnd, IDC_SLIDER1), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_SLIDER2), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_SLIDER3), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_SLIDER4), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_SLIDER5), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_SLIDER6), bEnable);

   EnableWindow(GetDlgItem(hwnd, IDC_CHECK1), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_CHECK2), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_CHECK3), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_CHECK4), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_CHECK5), bEnable);
   EnableWindow(GetDlgItem(hwnd, IDC_CHECK6), bEnable);
   
   EnableWindow(hwnd, bEnable);
   }

int FindHighestReplayNumber (void)
   {
   char szMapDir[MAX_PATH];
   
   strcpy(szMapDir, szMoonbasePathG);
   strcat(szMapDir, "user\\Rep???.rep");

   WIN32_FIND_DATA fd;
   HANDLE hFind;
   int nReplayHighest = 0;
   BOOL bFinished;

   hFind = FindFirstFile(szMapDir, &fd);
   bFinished = (INVALID_HANDLE_VALUE == hFind);

   while (!bFinished)
      {
      int nReplay = atoi(fd.cFileName + 3);
      if (nReplay > nReplayHighest)
         nReplayHighest = nReplay;

      bFinished = !FindNextFile(hFind, &fd);
      }

   FindClose(hFind);
   
   return nReplayHighest;
   }

bool SpawnMoonbaseCommander (HWND hwnd, PROCESS_INFORMATION& pi, bool bSaveWaitInfo)
   {
   char szExe[1024];
   strcpy(szExe, szMoonbasePathG);
   strcat(szExe, "moonbase.exe");

   STARTUPINFO si;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);

   if (pi.hProcess)
      CloseHandle(pi.hProcess);
   if (pi.hThread)
      CloseHandle(pi.hThread);
   ZeroMemory( &pi, sizeof(pi) );

   bool bRet;
   bRet = CreateProcess(szExe, NULL, NULL, NULL, FALSE, 0, NULL, szMoonbasePathG, &si, &pi) ? true : false;
   // GetLastError

   if (bSaveWaitInfo)
      {
      nReplayNumberHighestG = FindHighestReplayNumber();

      EnableDialog(hwnd, false);
      }
   else
      {
      CloseHandle(piG.hProcess);
      CloseHandle(piG.hThread);
      ZeroMemory(&piG, sizeof(piG));
      }

   return bRet;
   }

void HostGame(HWND hwnd, CGameInfo& gi)
   {
   CleanUpMoonbaseINI();

   char sz[24];
   sprintf(sz, "%d.%d.%d.%d", gi.nIP1, gi.nIP2, gi.nIP3, gi.nIP4);
   WritePrivateProfileString("System", "HostIP", sz, szMoonbaseIniFileG);

   WritePrivateProfileString("user", "map", "1", szMoonbaseIniFileG);

   SpawnMoonbaseCommander(hwnd, piG, true);
   }

void JoinGame(HWND hwnd, CGameInfo& gi)
   {
   CleanUpMoonbaseINI();

   char sz[24];
   sprintf(sz, "%d.%d.%d.%d", gi.nIP1, gi.nIP2, gi.nIP3, gi.nIP4);
   WritePrivateProfileString("System", "JoinIP", sz, szMoonbaseIniFileG);

   SpawnMoonbaseCommander(hwnd, piG, true);
   }

char *FindNumber (char *psz)
   {
   return strpbrk(psz, "0123456789");
   }

// expects psz to point to a digit
bool IsIPAddress (char *psz)
   {
   int cDigits;

   if (!psz)
      return false;

   cDigits = 0;

   while (psz && isdigit(*psz) && cDigits < 4)
      {
      ++cDigits;
      ++psz;
      }

   if (cDigits > 3 && cDigits > 0 || !psz)
      return false;

   if (*psz != '.')
      return false;

   ++psz;

   cDigits = 0;

   while (psz && isdigit(*psz) && cDigits < 4)
      {
      ++cDigits;
      ++psz;
      }

   if (cDigits > 3 || !psz)
      return false;

   if (*psz != '.')
      return false;

   ++psz;

   cDigits = 0;

   while (psz && isdigit(*psz) && cDigits < 4)
      {
      ++cDigits;
      ++psz;
      }

   if (cDigits > 3 && cDigits > 0 || !psz)
      return false;

   if (*psz != '.')
      return false;

   ++psz;

   cDigits = 0;

   while (psz && isdigit(*psz) && cDigits < 4)
      {
      ++cDigits;
      ++psz;
      }

   if (cDigits > 3 && cDigits > 0)
      return false;

   return true;
   }

char *FindIPAddressInString (char *psz)
   {
   psz = FindNumber(psz);
   while (psz && !IsIPAddress(psz))
      {
      while (psz && isdigit(*psz))
         ++psz;
      psz = FindNumber(psz);
      }

   return psz;
   }

DWORD DetectIPAddress (char *pszURL)
   {
   DWORD dwRet = 0;
   HINTERNET hINet, hFile;
   hINet = InternetOpen("InetURL/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
   
   if ( !hINet )
      {
      // TODO: Good error handling
//      fprintf(stderr, "InternetOpen failed\n");
//      exit(-1);
      }

   hFile = InternetOpenUrl( hINet, pszURL, NULL, 0, 0, 0 ) ;

   if ( hFile )
      {
      CHAR buffer[32768];
      DWORD dwRead;
      char *psz = buffer;
      DWORD cch = 0;
      int cPasses = 0;
      
      while (InternetReadFile( hFile, psz, 32767 - cch, &dwRead)) 
         {
         if (dwRead == 0)
            break;

         psz += dwRead;
         cch += dwRead;

         ++cPasses;
         }

      if (cch > 0)
         {
         buffer[cch] = '\0';
         int nIP1, nIP2, nIP3, nIP4;
         char *psz;
         psz = FindIPAddressInString(buffer);
         if (psz)
            {
            sscanf(psz, "%d.%d.%d.%d", &nIP1, &nIP2, &nIP3, &nIP4);
            dwRet = MAKEIPADDRESS(nIP1, nIP2, nIP3, nIP4);
            }
         }

      InternetCloseHandle( hFile );
      }

   InternetCloseHandle( hINet );
   
   return dwRet;
   }


DWORD DetectLANIPAddress (void)
   {
   DWORD dwRet = 0;
   WSADATA WSAData;

   // Initialize winsock dll
   if(::WSAStartup(MAKEWORD(1, 0), &WSAData))
     return 0;// Error handling

   // Get local host name
   char szHostName[128] = "";

   if(::gethostname(szHostName, sizeof(szHostName)))
     return 0;// Error handling -> call 'WSAGetLastError()'

   // Get local IP addresses
   struct sockaddr_in SocketAddress;
   struct hostent     *pHost        = 0;

   pHost = ::gethostbyname(szHostName);
   if(!pHost)
     return 0;// Error handling -> call 'WSAGetLastError()'

//   char aszIPAddresses[10][16]; // maximum of ten IP addresses

   int iCnt = 0; // Just get the first one
//   for(int iCnt = 0; ((pHost->h_addr_list[iCnt]) && (iCnt < 10)); ++iCnt)
//      {
      memcpy(&SocketAddress.sin_addr, pHost->h_addr_list[iCnt], pHost->h_length);
      dwRet = htonl(*((DWORD *)&SocketAddress.sin_addr));
//      strcpy(aszIPAddresses[iCnt], inet_ntoa(SocketAddress.sin_addr));
//      }

   // Cleanup
   WSACleanup();

   return dwRet;
   }

/**
 * These are the only keys aside from alphanumeric characters
 * that are allowed in the game.
 */
const char * getKeyStringFromInt(int key) {
    if (key == 32){
        return "SPC";
    }
    if (key == 8){ 
        return "BK";
    }
    if (key == 13){
        return "ENT";
    }
    if (key == 9){
        return "TAB";
    }
    return NULL;

}
BOOL CALLBACK ChangeNameDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
   { 
   switch (message) 
      { 
      case WM_INITDIALOG:
         {
             // Set the name in the edittext to the current name
             SendMessage(GetDlgItem(hwndDlg, IDC_EDIT2), WM_SETTEXT, (WPARAM) TRUE, (LPARAM) name);
             // Select the name
             SendMessage(GetDlgItem(hwndDlg, IDC_EDIT2), EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
             // Set a limit on the number of characters in the edittext 
             SendMessage(GetDlgItem(hwndDlg, IDC_EDIT2), EM_SETLIMITTEXT, (WPARAM) NAME_CHAR_LIMIT, (LPARAM) -1);

             return TRUE; 
         }
  
      case WM_COMMAND: 
         switch (LOWORD(wParam)) 
            {
            case IDOK:
               {
                   char enteredName[NAME_CHAR_LIMIT];

                   GetDlgItemText(hwndDlg, IDC_EDIT2, enteredName, NAME_CHAR_LIMIT);

                   // If the entered name is nothing, we just treat it as a cancel
                   if (!enteredName[0]){
                       EndDialog(hwndDlg, FALSE);
                   }else{
                       strncpy(name, enteredName, NAME_CHAR_LIMIT);

                       EndDialog(hwndDlg, TRUE);
                   }
               
                   return TRUE;
               }

            case IDCANCEL:
               {
               EndDialog(hwndDlg, FALSE);

               return TRUE;
               }
               
            default:
               return FALSE;
            }
      }
   return FALSE; 
   } 

BOOL CALLBACK ChangeKeyDlgProc(HWND m_hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
    switch (uMsg)
    {
    case WM_INITDIALOG:
	changeKeyHwnd = m_hwnd;
	return TRUE;
    case WM_COMMAND: 
	if (LOWORD(wParam) == IDCANCEL){
	    EndDialog(m_hwnd, FALSE);
	    changeKeyHwnd = NULL;

	    return TRUE;
	}
    }
    return FALSE;
}

BOOL CALLBACK GameOptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch(message){
      case WM_INITDIALOG:
            {
                // Name can only be NAME_CHAR_LIMIT characters long
                GetPrivateProfileString("user", INI_KEY_NAME, DEFAULT_PLAYER_NAME, name, sizeof(name), szMoonbaseIniFileG);
                SendMessage(GetDlgItem(hwndDlg, IDC_TXT1), WM_SETTEXT, (WPARAM) TRUE, (LPARAM) name);

                const char * keyString;
                // Damage bar preference:
                damageBarPreference = GetPrivateProfileInt("Options", INI_KEY_DAMAGE_BAR, DEFAULT_CONTROL_DAMAGE_BAR, szMoonbaseIniFileG);
                keyString = getKeyStringFromInt(damageBarPreference);
                SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON2), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &damageBarPreference : (LPARAM) keyString));

                // Range radius
                rangeRadiusPreference = GetPrivateProfileInt("Options", INI_KEY_RANGE_RADIUS, DEFAULT_CONTROL_RANGE_RADIUS, szMoonbaseIniFileG);
                keyString = getKeyStringFromInt(rangeRadiusPreference);
                SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON3), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &rangeRadiusPreference : (LPARAM) keyString));

                // Center unit
                centerUnitPreference = GetPrivateProfileInt("Options", INI_KEY_CENTER_UNIT, DEFAULT_CONTROL_CENTER_UNIT, szMoonbaseIniFileG);
                keyString = getKeyStringFromInt(centerUnitPreference);
                SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON4), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &centerUnitPreference : (LPARAM) keyString));

                // Next unit
                nextUnitPreference = GetPrivateProfileInt("Options", INI_KEY_NEXT_UNIT, DEFAULT_CONTROL_NEXT_UNIT, szMoonbaseIniFileG);
                keyString = getKeyStringFromInt(nextUnitPreference);
                SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON5), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &nextUnitPreference : (LPARAM) keyString));

                // Previous unit
                previousUnitPreference = GetPrivateProfileInt("Options", INI_KEY_PREVIOUS_UNIT, DEFAULT_CONTROL_PREVIOUS_UNIT, szMoonbaseIniFileG);
                keyString = getKeyStringFromInt(previousUnitPreference);
                SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON6), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &previousUnitPreference : (LPARAM) keyString));

                // Closest launcher
                closestLauncherPreference = GetPrivateProfileInt("Options", INI_KEY_CLOSEST_LAUNCHER, DEFAULT_CONTROL_CLOSEST_LAUNCHER, szMoonbaseIniFileG);
                keyString = getKeyStringFromInt(closestLauncherPreference);
                SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON7), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &closestLauncherPreference : (LPARAM) keyString));

                // Show attacked
                showAttackedPreference = GetPrivateProfileInt("Options", INI_KEY_SHOW_ATTACKED, DEFAULT_CONTROL_SHOW_ATTACKED, szMoonbaseIniFileG);
                keyString = getKeyStringFromInt(showAttackedPreference);
                SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON8), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &showAttackedPreference : (LPARAM) keyString));

                // Chat toggle
                chatTogglePreference = GetPrivateProfileInt("Options", INI_KEY_CHAT_TOGGLE, DEFAULT_CONTROL_CHAT_TOGGLE, szMoonbaseIniFileG);
                keyString = getKeyStringFromInt(chatTogglePreference);
                SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON9), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &chatTogglePreference : (LPARAM) keyString));
		
		// We load up this set to avoid setting duplicates when settings keys for these
		preferenceSet.clear();
		preferenceSet.insert(damageBarPreference);
		preferenceSet.insert(rangeRadiusPreference);
		preferenceSet.insert(centerUnitPreference);
		preferenceSet.insert(nextUnitPreference);
		preferenceSet.insert(previousUnitPreference);
		preferenceSet.insert(closestLauncherPreference);
		preferenceSet.insert(showAttackedPreference);
		preferenceSet.insert(chatTogglePreference);

                // Auto scroll
                BOOL autoScrollPreference = GetPrivateProfileInt("Options", INI_KEY_AUTO_SCROLL, DEFAULT_CONTROL_AUTO_SCROLL, szMoonbaseIniFileG);
                CheckDlgButton(hwndDlg, IDC_CHECK1, autoScrollPreference);

                // Auto hub select
                BOOL autoHubSelectPreference = GetPrivateProfileInt("Options", INI_KEY_AUTO_HUB_SELECT, DEFAULT_CONTROL_AUTO_HUB_SELECT, szMoonbaseIniFileG);
                CheckDlgButton(hwndDlg, IDC_CHECK2, autoHubSelectPreference);

                // Smart camera
                BOOL smartCameraPreference = GetPrivateProfileInt("Options", INI_KEY_SMART_CAMERA, DEFAULT_CONTROL_SMART_CAMERA, szMoonbaseIniFileG);
                CheckDlgButton(hwndDlg, IDC_CHECK3, smartCameraPreference);

                // Auto return camera
                BOOL autoReturnCameraPreference = GetPrivateProfileInt("Options", INI_KEY_AUTO_RETURN_CAMERA, DEFAULT_CONTROL_AUTO_RETURN_CAMERA, szMoonbaseIniFileG);
                CheckDlgButton(hwndDlg, IDC_CHECK4, autoReturnCameraPreference);

                // SFX volume
                int sfxVolumePreference = GetPrivateProfileInt("Options", INI_KEY_SFX_VOLUME, DEFAULT_CONTROL_SFX_VOLUME, szMoonbaseIniFileG);
                SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 255));
                SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_SETPOS, TRUE, sfxVolumePreference);  

                // Music volume
                int musicVolumePreference = GetPrivateProfileInt("Options", INI_KEY_MUSIC_VOLUME, DEFAULT_CONTROL_MUSIC_VOLUME, szMoonbaseIniFileG);
                SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER2), TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 255));
                SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER2), TBM_SETPOS, TRUE, musicVolumePreference);  

                // Voice volume
                int voiceVolumePreference = GetPrivateProfileInt("Options", INI_KEY_VOICE_VOLUME, DEFAULT_CONTROL_VOICE_VOLUME, szMoonbaseIniFileG);
                SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER3), TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 255));
                SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER3), TBM_SETPOS, TRUE, voiceVolumePreference);  

                // Interface volume
                int interfaceVolumePreference = GetPrivateProfileInt("Options", INI_KEY_INTERFACE_VOLUME, DEFAULT_CONTROL_INTERFACE_VOLUME, szMoonbaseIniFileG);
                SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER4), TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 255));
                SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER4), TBM_SETPOS, TRUE, interfaceVolumePreference);  

                // Commentary
                BOOL commentaryPreference = GetPrivateProfileInt("Options", INI_KEY_COMMENTARY, DEFAULT_CONTROL_COMMENTARY, szMoonbaseIniFileG);
                CheckDlgButton(hwndDlg, IDC_CHECK5, commentaryPreference);

                // Music quality (FALSE == high, TRUE == low)
                BOOL musicQualityPreference = GetPrivateProfileInt("Options", INI_KEY_MUSIC_QUALITY, DEFAULT_CONTROL_MUSIC_QUALITY, szMoonbaseIniFileG);
                CheckRadioButton(hwndDlg, IDR_BUTTON1, IDR_BUTTON2, musicQualityPreference ? IDR_BUTTON1 : IDR_BUTTON2);

                return TRUE;
            }
        case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                    case IDC_BUTTON1:
                        {
                            // Change name
                            if (DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG5), hwndDlg, ChangeNameDlgProc, 0)){
                                // Reset shown name
                                SendMessage(GetDlgItem(hwndDlg, IDC_TXT1), WM_SETTEXT, (WPARAM) TRUE, (LPARAM) name);
                            }
                            break;
                        }
                    case IDC_BUTTON2:
                        {
                            // Change Damage Bar control
				OutputDebugString("Changing Damage");
				preferenceSet.erase(damageBarPreference);
                            if (DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG6), hwndDlg, ChangeKeyDlgProc, 0)){
				    damageBarPreference = changedKey;
				    const char * keyString = getKeyStringFromInt(damageBarPreference);
				    SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON2), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &damageBarPreference : (LPARAM) keyString));
			    } else {
				preferenceSet.insert(damageBarPreference);
			    }
                            break;
                        }
		    case IDC_BUTTON3:
                        {
                            // Change Radius Range control
				preferenceSet.erase(rangeRadiusPreference);
                            if (DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG6), hwndDlg, ChangeKeyDlgProc, 0)){
				    rangeRadiusPreference = changedKey;
				    const char * keyString = getKeyStringFromInt(rangeRadiusPreference);
				    SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON3), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &rangeRadiusPreference : (LPARAM) keyString));
			    } else {
				preferenceSet.insert(rangeRadiusPreference);
                            }
                            break;
                        }
		    case IDC_BUTTON4:
			{
				// Change Center Unit control
					preferenceSet.erase(centerUnitPreference);
				if (DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG6), hwndDlg, ChangeKeyDlgProc, 0)){
					centerUnitPreference = changedKey;
					const char * keyString = getKeyStringFromInt(centerUnitPreference);
					SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON4), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &centerUnitPreference : (LPARAM) keyString));
				} else {
					preferenceSet.insert(centerUnitPreference);
				}
				break;
			}
		    case IDC_BUTTON5:
			{
				// Change Next Unit control
					preferenceSet.erase(nextUnitPreference);
				if (DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG6), hwndDlg, ChangeKeyDlgProc, 0)){
					nextUnitPreference = changedKey;
					const char * keyString = getKeyStringFromInt(nextUnitPreference);
					SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON5), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &nextUnitPreference : (LPARAM) keyString));
				} else {
					preferenceSet.insert(nextUnitPreference);
				}
				break;
			}
		    case IDC_BUTTON6:
			{
				// Change Previous Unit control
					preferenceSet.erase(previousUnitPreference);
				if (DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG6), hwndDlg, ChangeKeyDlgProc, 0)){
					previousUnitPreference = changedKey;
					const char * keyString = getKeyStringFromInt(previousUnitPreference);
					SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON6), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &previousUnitPreference : (LPARAM) keyString));
				} else {
					preferenceSet.insert(previousUnitPreference);
				}
				break;
			}
		    case IDC_BUTTON7:
			{
				// Change Closest Launcher control
					preferenceSet.erase(closestLauncherPreference);
				if (DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG6), hwndDlg, ChangeKeyDlgProc, 0)){
					closestLauncherPreference = changedKey;
					const char * keyString = getKeyStringFromInt(closestLauncherPreference);
					SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON7), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &closestLauncherPreference : (LPARAM) keyString));
				} else {
					preferenceSet.insert(closestLauncherPreference);
				}
				break;
			}
		    case IDC_BUTTON8:
			{
				// Change Show Attacked control
					preferenceSet.erase(showAttackedPreference);
				if (DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG6), hwndDlg, ChangeKeyDlgProc, 0)){
					showAttackedPreference = changedKey;
					const char * keyString = getKeyStringFromInt(showAttackedPreference);
					SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON8), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &showAttackedPreference : (LPARAM) keyString));
				} else {
					preferenceSet.insert(showAttackedPreference);
				}
				break;
			}
		    case IDC_BUTTON9:
			{
				// Change Chat Toggle control
					preferenceSet.erase(chatTogglePreference);
				if (DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG6), hwndDlg, ChangeKeyDlgProc, 0)){
					chatTogglePreference = changedKey;
					const char * keyString = getKeyStringFromInt(chatTogglePreference);
					SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON9), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &chatTogglePreference : (LPARAM) keyString));
				} else {
					preferenceSet.insert(chatTogglePreference);
				}
				break;
			}
                    case IDC_BUTTON10: 
                        {
                            // Reset
                            strncpy(name, DEFAULT_PLAYER_NAME, NAME_CHAR_LIMIT);
                            SendMessage(GetDlgItem(hwndDlg, IDC_TXT1), WM_SETTEXT, (WPARAM) TRUE, (LPARAM) name);

                            const char * keyString;

                            damageBarPreference =  DEFAULT_CONTROL_DAMAGE_BAR;
                            keyString = getKeyStringFromInt(damageBarPreference);
                            SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON2), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &damageBarPreference : (LPARAM) keyString));

                            rangeRadiusPreference =  DEFAULT_CONTROL_RANGE_RADIUS;
                            keyString = getKeyStringFromInt(rangeRadiusPreference);
                            SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON3), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &rangeRadiusPreference : (LPARAM) keyString));

                            centerUnitPreference =  DEFAULT_CONTROL_CENTER_UNIT;
                            keyString = getKeyStringFromInt(centerUnitPreference);
                            SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON4), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &centerUnitPreference : (LPARAM) keyString));

                            nextUnitPreference = DEFAULT_CONTROL_NEXT_UNIT;
                            keyString = getKeyStringFromInt(nextUnitPreference);
                            SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON5), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &nextUnitPreference : (LPARAM) keyString));

                            previousUnitPreference = DEFAULT_CONTROL_PREVIOUS_UNIT;
                            keyString = getKeyStringFromInt(previousUnitPreference);
                            SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON6), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &previousUnitPreference : (LPARAM) keyString));
                            
                            closestLauncherPreference = DEFAULT_CONTROL_CLOSEST_LAUNCHER;
                            keyString = getKeyStringFromInt(closestLauncherPreference);
                            SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON7), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &closestLauncherPreference : (LPARAM) keyString));

                            showAttackedPreference = DEFAULT_CONTROL_SHOW_ATTACKED;
                            keyString = getKeyStringFromInt(showAttackedPreference);
                            SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON8), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &showAttackedPreference : (LPARAM) keyString));
                            
                            chatTogglePreference = DEFAULT_CONTROL_CHAT_TOGGLE;
                            keyString = getKeyStringFromInt(chatTogglePreference);
                            SendMessage(GetDlgItem(hwndDlg, IDC_BUTTON9), WM_SETTEXT, (WPARAM) FALSE, (keyString == NULL ? (LPARAM) &chatTogglePreference : (LPARAM) keyString));

                            CheckDlgButton(hwndDlg, IDC_CHECK1, DEFAULT_CONTROL_AUTO_SCROLL);

                            CheckDlgButton(hwndDlg, IDC_CHECK2, DEFAULT_CONTROL_AUTO_HUB_SELECT);

                            CheckDlgButton(hwndDlg, IDC_CHECK3, DEFAULT_CONTROL_SMART_CAMERA);

                            CheckDlgButton(hwndDlg, IDC_CHECK4, DEFAULT_CONTROL_AUTO_RETURN_CAMERA);

                            SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_SETPOS, TRUE, DEFAULT_CONTROL_SFX_VOLUME);  
                            
                            SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER2), TBM_SETPOS, TRUE, DEFAULT_CONTROL_MUSIC_VOLUME);  
                            
                            SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER3), TBM_SETPOS, TRUE, DEFAULT_CONTROL_VOICE_VOLUME);  
                            
                            SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER4), TBM_SETPOS, TRUE, DEFAULT_CONTROL_INTERFACE_VOLUME);  

                            CheckDlgButton(hwndDlg, IDC_CHECK5, DEFAULT_CONTROL_COMMENTARY);

                            CheckRadioButton(hwndDlg, IDR_BUTTON1, IDR_BUTTON2, DEFAULT_CONTROL_MUSIC_QUALITY ? IDR_BUTTON1 : IDR_BUTTON2);
                            break;
                        }
                    case IDOK:
                        {
                            WritePrivateProfileString("user", INI_KEY_NAME, name, szMoonbaseIniFileG);

                            char sz[4];

                            // Damage bar
                            wsprintf(sz, "%d", damageBarPreference);
                            WritePrivateProfileString("Options", INI_KEY_DAMAGE_BAR, sz, szMoonbaseIniFileG);

                            // Range radius
                            wsprintf(sz, "%d", rangeRadiusPreference);
                            WritePrivateProfileString("Options", INI_KEY_RANGE_RADIUS, sz, szMoonbaseIniFileG);

                            // Center unit
                            wsprintf(sz, "%d", centerUnitPreference);
                            WritePrivateProfileString("Options", INI_KEY_CENTER_UNIT, sz, szMoonbaseIniFileG);

                            // Next unit
                            wsprintf(sz, "%d", nextUnitPreference);
                            WritePrivateProfileString("Options", INI_KEY_NEXT_UNIT, sz, szMoonbaseIniFileG);

                            // Previous unit
                            wsprintf(sz, "%d", previousUnitPreference);
                            WritePrivateProfileString("Options", INI_KEY_PREVIOUS_UNIT, sz, szMoonbaseIniFileG);

                            // Closest launcher
                            wsprintf(sz, "%d", closestLauncherPreference);
                            WritePrivateProfileString("Options", INI_KEY_CLOSEST_LAUNCHER, sz, szMoonbaseIniFileG);

                            // Show attacked
                            wsprintf(sz, "%d", showAttackedPreference);
                            WritePrivateProfileString("Options", INI_KEY_SHOW_ATTACKED, sz, szMoonbaseIniFileG);

                            // Chat toggle
                            wsprintf(sz, "%d", chatTogglePreference);
                            WritePrivateProfileString("Options", INI_KEY_CHAT_TOGGLE, sz, szMoonbaseIniFileG);

                            // Auto scroll
                            wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK1));
                            WritePrivateProfileString("Options", INI_KEY_AUTO_SCROLL, sz, szMoonbaseIniFileG);

                            // Auto hub-select
                            wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK2));
                            WritePrivateProfileString("Options", INI_KEY_AUTO_HUB_SELECT, sz, szMoonbaseIniFileG);

                            // Smart camera
                            wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK3));
                            WritePrivateProfileString("Options", INI_KEY_SMART_CAMERA, sz, szMoonbaseIniFileG);

                            // Auto-return camera
                            wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK4));
                            WritePrivateProfileString("Options", INI_KEY_AUTO_RETURN_CAMERA, sz, szMoonbaseIniFileG);

                            // SFX volume
                            wsprintf(sz, "%d", SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_GETPOS, 0, 0));
                            WritePrivateProfileString("Options", INI_KEY_SFX_VOLUME, sz, szMoonbaseIniFileG);

                            // Music volume
                            wsprintf(sz, "%d", SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER2), TBM_GETPOS, 0, 0));
                            WritePrivateProfileString("Options", INI_KEY_MUSIC_VOLUME, sz, szMoonbaseIniFileG);

                            // Voice volume
                            wsprintf(sz, "%d", SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER3), TBM_GETPOS, 0, 0));
                            WritePrivateProfileString("Options", INI_KEY_VOICE_VOLUME, sz, szMoonbaseIniFileG);

                            // Interface volume
                            wsprintf(sz, "%d", SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER4), TBM_GETPOS, 0, 0));
                            WritePrivateProfileString("Options", INI_KEY_INTERFACE_VOLUME, sz, szMoonbaseIniFileG);

                            // Music quality (Low is TRUE, High is FALSE)
                            wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDR_BUTTON1));
                            WritePrivateProfileString("Options", INI_KEY_MUSIC_QUALITY, sz, szMoonbaseIniFileG);
                            
                            // Commentary
                            wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK5));
                            WritePrivateProfileString("Options", INI_KEY_COMMENTARY, sz, szMoonbaseIniFileG);

                            EndDialog(hwndDlg, FALSE);
                            break;
                        }
                    case IDCANCEL:
                        {
                            EndDialog(hwndDlg, FALSE);
                            break;
                        }
                }
                // Act on commands
                return TRUE;
            }
    }
    return FALSE;
}

BOOL CALLBACK JoinDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
   { 
   switch (message) 
      { 
      case WM_INITDIALOG:
         {
         // TODO: Auto paste from clipboard?

         EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

         return TRUE; 
         }
  
      case WM_COMMAND: 
         switch (LOWORD(wParam)) 
            {
            case IDC_EDIT2:
               {
               if (HIWORD(wParam) == EN_CHANGE)
                  {
                  CGameInfo gi;
                  char sz[1024];

                  // Get the game number
                  GetDlgItemText(hwndDlg, IDC_EDIT2, sz, 1024);

                  EnableWindow(GetDlgItem(hwndDlg, IDOK), SecureBase64Decode((unsigned char *)&gi, sizeof(CGameInfo), sz));
                  }

               return TRUE;
               }

            case IDOK:
               {
               // TODO: Make this whole thing more robust. Idiots will be using it...
               // TODO: Leading and trailing spaces?

               MBCMIF mif;
               char sz[1024];

               // Get the game number
               GetDlgItemText(hwndDlg, IDC_EDIT2, sz, 1024);

               // This check can probably be made an assert, the UI enable/disable code should make sure we get good keys.
               if (!SecureBase64Decode((unsigned char *)&giG, sizeof(CGameInfo), sz))
                  {
                  MessageBox(hwndDlg, "Invalid game number.", "Moonbase Console Error", MB_OK);
                  return TRUE;
                  }

               if (giG.nVersion != GAMENUM_VERSION)
                  {
                  MessageBox(hwndDlg, "This program accepts version 5 game numbers, but your host gave you a different game number. Make sure that you are both using the same version of Moonbase Console.", "Moonbase Console Error", MB_OK);
                  return TRUE;
                  }

               EndDialog(hwndDlg, TRUE);

               return TRUE;
               }

            case IDCANCEL:
               {
               EndDialog(hwndDlg, FALSE);

               return TRUE;
               }
               
            default:
               return FALSE;
            }
      }
   return FALSE; 
   } 

BOOL CALLBACK HostDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
   { 
   switch (message) 
      { 
      case WM_INITDIALOG:
         {
         HWND hwndCtl = GetDlgItem(hwndDlg, IDC_COMBO1);
         SendMessage(hwndCtl, CB_ADDSTRING, 0, (LPARAM)"http://bot.whatismyipaddress.com");
         SendMessage(hwndCtl, CB_ADDSTRING, 0, (LPARAM)"http://api.ipify.org");
         SendMessage(hwndCtl, CB_ADDSTRING, 0, (LPARAM)"https://wtfismyip.com/text");

         char sz[1024];
         GetPrivateProfileString("MoonbaseConsole", "IPService", "http://api.ipify.org", sz, sizeof(sz), szMoonbaseIniFileG);
         SendMessage(hwndCtl, WM_SETTEXT, NULL, (LPARAM)sz);

         int nValue;

         nValue = GetPrivateProfileInt("MoonbaseConsole", "IPAddress", 0, szMoonbaseIniFileG);
         if (nValue)
            SendMessage(GetDlgItem(hwndDlg, IDC_IPADDRESS1), IPM_SETADDRESS, 0, (LPARAM)nValue);

         EnableDialog(hwndDlg, true);

         return TRUE; 
         }
  
      case WM_NOTIFY:
         switch (wParam)
            {
            case IDC_IPADDRESS1:
               LPNMIPADDRESS lpnmipa;
               lpnmipa = (LPNMIPADDRESS) lParam;

               // TODO:This has to be changed to be the right button
               EnableDisableHostButton(hwndDlg);

               return TRUE;

            default:
               return FALSE;
            }

      case WM_COMMAND: 
         switch (LOWORD(wParam)) 
            {
            case IDC_BUTTON1:
               {
               char sz[1024];

               GetDlgItemText(hwndDlg, IDC_COMBO1, sz, 1024);

               // TODO: Some kind of timeout here?
               EnableDialog(hwndDlg, false);
               SendMessage(GetDlgItem(hwndDlg, IDC_IPADDRESS1), IPM_SETADDRESS, 0, (LPARAM)DetectIPAddress(sz));
               EnableDialog(hwndDlg, true);

               return TRUE;
               }

            case IDC_BUTTON3:
               {
               // TODO: Some kind of timeout here?
               EnableDialog(hwndDlg, false);
               SendMessage(GetDlgItem(hwndDlg, IDC_IPADDRESS1), IPM_SETADDRESS, 0, (LPARAM)DetectLANIPAddress());
               EnableDialog(hwndDlg, true);

               return TRUE;
               }

            case IDOK:
               {
               char sz[80];
               SendMessage(GetDlgItem(hwndDlg, IDC_IPADDRESS1), IPM_GETADDRESS, 0, (LPARAM)&dwIPAddressG);
               wsprintf(sz, "%d", dwIPAddressG);
               WritePrivateProfileString("MoonbaseConsole", "IPAddress", sz, szMoonbaseIniFileG);

               GetDlgItemText(hwndDlg, IDC_COMBO1, sz, 80);
               WritePrivateProfileString("MoonbaseConsole", "IPService", sz, szMoonbaseIniFileG);

               EndDialog(hwndDlg, TRUE);

               return TRUE;
               }

            case IDCANCEL:
               {
               dwIPAddressG = NULL;

               EndDialog(hwndDlg, FALSE);

               return TRUE;
               }
               
            default:
               return FALSE;
            }
      } 
   return FALSE; 
   } 



BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
   { 
   switch (message) 
      { 
      case WM_INITDIALOG:
         {
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(4, 10));
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER2), TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(1, 2));
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER3), TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 6));
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER4), TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 6));
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER5), TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 6));
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER6), TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(1, 6));

         int nValue;

         nValue = GetPrivateProfileInt("MoonbaseConsole", "SizeValue", 4, szMoonbaseIniFileG);
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_SETPOS, TRUE, nValue);  
         nValue = GetPrivateProfileInt("MoonbaseConsole", "SizeRandom", FALSE, szMoonbaseIniFileG);
         CheckDlgButton(hwndDlg, IDC_CHECK1, nValue);
         
         nValue = GetPrivateProfileInt("MoonbaseConsole", "GeneratorValue", 1, szMoonbaseIniFileG);
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER2), TBM_SETPOS, TRUE, nValue);  
         nValue = GetPrivateProfileInt("MoonbaseConsole", "GeneratorRandom", TRUE, szMoonbaseIniFileG);
         CheckDlgButton(hwndDlg, IDC_CHECK2, nValue);

         nValue = GetPrivateProfileInt("MoonbaseConsole", "EnergyValue", 3, szMoonbaseIniFileG);
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER3), TBM_SETPOS, TRUE, nValue);  
         nValue = GetPrivateProfileInt("MoonbaseConsole", "EnergyRandom", TRUE, szMoonbaseIniFileG);
         CheckDlgButton(hwndDlg, IDC_CHECK3, nValue);

         nValue = GetPrivateProfileInt("MoonbaseConsole", "TerrainValue", 3, szMoonbaseIniFileG);
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER4), TBM_SETPOS, TRUE, nValue);  
         nValue = GetPrivateProfileInt("MoonbaseConsole", "TerrainRandom", TRUE, szMoonbaseIniFileG);
         CheckDlgButton(hwndDlg, IDC_CHECK4, nValue);

         nValue = GetPrivateProfileInt("MoonbaseConsole", "WaterValue", 3, szMoonbaseIniFileG);
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER5), TBM_SETPOS, TRUE, nValue);  
         nValue = GetPrivateProfileInt("MoonbaseConsole", "WaterRandom", TRUE, szMoonbaseIniFileG);
         CheckDlgButton(hwndDlg, IDC_CHECK5, nValue);

         nValue = GetPrivateProfileInt("MoonbaseConsole", "TilesetValue", 5, szMoonbaseIniFileG);
         SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER6), TBM_SETPOS, TRUE, nValue);  
         nValue = GetPrivateProfileInt("MoonbaseConsole", "TilesetRandom", TRUE, szMoonbaseIniFileG);
         CheckDlgButton(hwndDlg, IDC_CHECK6, nValue);

         SendMessage(hwndDlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIconG);

         EnableDialog(hwndDlg, true);

         return TRUE; 
         }

      case WM_VSCROLL:
         {
         if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SLIDER1))
            CheckDlgButton(hwndDlg, IDC_CHECK1, BST_UNCHECKED);
         else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SLIDER2))
            CheckDlgButton(hwndDlg, IDC_CHECK2, BST_UNCHECKED);
         else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SLIDER3))
            CheckDlgButton(hwndDlg, IDC_CHECK3, BST_UNCHECKED);
         else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SLIDER4))
            CheckDlgButton(hwndDlg, IDC_CHECK4, BST_UNCHECKED);
         else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SLIDER5))
            CheckDlgButton(hwndDlg, IDC_CHECK5, BST_UNCHECKED);
         else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SLIDER6))
            CheckDlgButton(hwndDlg, IDC_CHECK6, BST_UNCHECKED);
         return TRUE;
         }

      case WM_COMMAND: 
         switch (LOWORD(wParam)) 
            {
            case IDC_BUTTON1: // Help
               {
               HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_TXT1), "TXT");
               HGLOBAL hgbl = LoadResource(NULL, hrsrc);
               BYTE *pReadme = (BYTE *)LockResource(hgbl);

               char sz[MAX_PATH];
               strcpy(sz, szMoonbasePathG);
               strcat(sz, "\\MC_README.TXT");
               FILE *pf;
               pf = fopen(sz, "wb");
               fwrite(pReadme, 1, SizeofResource(NULL, hrsrc), pf);
               fclose(pf);

               ShellExecute(hwndDlg, "open", "MC_README.TXT", NULL, szMoonbasePathG, SW_SHOWNORMAL);

               return TRUE;
               }

            case IDC_BUTTON3: // Credits
               MessageBox(hwndDlg, "A lot of people helped in one way or the other. Here are the main contributors (in alphabetical order).\n"
                                   "\n"
                                   "Katton designed and coded the Katton map generation algorithm. He decoded a lot of the tiles and published what he learned. He also coded the algorithm that puts the \"craters\" on the map and wrote the code that generates the map preview showing the options.\n"
                                   "\n"
                                   "MYCROFT wrote the tools to rip apart the MAP files so they could be deciphered, decoded the rest of the tiles, defined the format that allows the generators to describe the map without caring about tiles, wrote the code to translate the generator output to a useable map, and did the Windows coding.\n"
                                   "\n"
                                   "SpacemanSpiff designed and coded the Spiff map generation algorithm. He also contributed to deciphering the map file format.\n"
                                   "\n"
                                   "Special thanks to Bwappo for his contest-winning thumbnail image.\n"
                                   "\n"
                                   "A number of other people on the Moonbase Commander forums also contributed to map decoding or beta testing including Bwappo, Covak, florent28, Kamolas, llangford, and YorkdinK.\n"
                                   "\n"
				   "Michael Barlow added the Game Options section and updated the functionality to attempt to get it working on machines in 2018.\n",
                                   "Moonbase Console Credits",
                                   MB_OK);
               return TRUE;

            case IDC_BUTTON4: //Settings
               {
		       OutputDebugString("Game Options clicked");
                   if (!DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG4), hwndDlg, GameOptionsDlgProc, 0))
                       return TRUE; // bail if they cancel
               }

            
            case IDC_BTN_HOST_GAME:
               if (!DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG3), hwndDlg, HostDlgProc, 0))
                  return TRUE; // bail if they cancel
               // fall through

            case IDC_SINGLE_GENERATE:
               {
               CGameInfo gi;

               gi.nVersion = GAMENUM_VERSION;

               if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHECK1))
                  gi.nMapSize = ((rand() % 5) + 4) * 8; // Don't randomly pick nonstandard map sizes.
               else
                  gi.nMapSize = SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_GETPOS, 0, 0) * 8;

               if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHECK2))
                  gi.nGenerator = (rand() % 2) + 1;
               else
                  gi.nGenerator = SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER2), TBM_GETPOS, 0, 0);

               if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHECK6))
                  gi.nTileset = (rand() % 6) + 1;
               else
                  gi.nTileset = 7 - SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER6), TBM_GETPOS, 0, 0);;

               gi.nIP1 = (unsigned char) FIRST_IPADDRESS(dwIPAddressG);
               gi.nIP2 = (unsigned char) SECOND_IPADDRESS(dwIPAddressG);
               gi.nIP3 = (unsigned char) THIRD_IPADDRESS(dwIPAddressG);
               gi.nIP4 = (unsigned char) FOURTH_IPADDRESS(dwIPAddressG);

               MBCMIF mif;

               if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHECK3))
                  gi.nEnergy = (rand() % 3) + 2;   // Only use [2, 3, 4] of the legal [0, 1, 2, 3, 4, 5, 6]
               else
                  gi.nEnergy = 6 - SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER3), TBM_GETPOS, 0, 0);

               if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHECK4))
                  gi.nTerrain = (rand() % 3) + 2;  // Only use [2, 3, 4] of the legal [0, 1, 2, 3, 4, 5, 6]
               else
                  gi.nTerrain = 6 - SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER4), TBM_GETPOS, 0, 0);

               if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHECK5))
                  gi.nWater = (rand() % 3) + 2;    // Only use [2, 3, 4] of the legal [0, 1, 2, 3, 4, 5, 6]
               else
                  gi.nWater = 6 - SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER5), TBM_GETPOS, 0, 0);

               gi.nSeed = rand(); // It is critical that the seed get set right before the generate.
               srand(gi.nSeed);

               switch (gi.nGenerator)
                  {
                  case SPIFF_GEN:
                     SpiffGenerate(gi, mif);
                     break;

                  case KATTON_GEN:
                     KattonGenerate(gi, mif);
                     break;
                  }

               try
                  {
                  BackupSystemMapFile();// TODO: Do single player differently.
                  makewiz(gi.nEnergy, gi.nTerrain, gi.nWater);
                  MakeMap(mif, gi);

                  if (LOWORD(wParam) == IDC_SINGLE_GENERATE)
                     {
                     gi.nIP1 = 192;
                     gi.nIP2 = 168;
                     gi.nIP3 = 1;
                     gi.nIP4 = 1;
                     }
                  else
                     {
                     PublishGameNumber(gi, hwndDlg);

                     // Prompt the user, tell them about the game number, and give them a chance to bail.
                     if (IDCANCEL == MessageBox(hwndDlg, "Your random map has been generated.\n\nThe game number for this map needs to be given to the people that you will be playing with.\nThis number is already on the clipboard, paste it into a message to them.\n\nOnce you have done this, press Ok to start Moonbase Commander or Cancel to abort.", "Moonbase Console Message", MB_OKCANCEL))
                        {
                        RestoreSystemMapFile();

                        return TRUE;
                        }
                     }

                  HostGame(hwndDlg, gi);
                  }
               catch (CMapError e)
                  {
                  RestoreSystemMapFile();

                  char szErrorHeader[80];
                  char szError[512];
                  sprintf(szErrorHeader, "Map Generation Error: Please Report This Information");
                  sprintf(szError, "%s\n\n"
                              "Generator:\t%s\n"
                              "Seed:\t\t%04X\n"
                              "Size:\t\t%d\n"
                              "nEnergy:\t\t%d\n"
                              "nTerrain:\t\t%d\n"
                              "nWater:\t\t%d\n"
                              "\n(%d, %d)\n"
                              "%d %d %d %d\n"
                              " %c %c %c\n"
                              "%d %d %d %d\n"
                              " %c %c %c\n"
                              "%d %d %d %d\n"
                              " %c %c %c\n"
                              "%d %d %d %d\n",
                              e.m_szErrorDescription,
                              (gi.nGenerator == SPIFF_GEN) ? "SpacemanSpiff" : "Katton",
                              (int)gi.nSeed,
                              (int)gi.nMapSize,
                              gi.nEnergy,
                              gi.nTerrain,
                              gi.nWater,
                              e.m_nX, e.m_nY,
                              mif.TTLLCorner(e.m_nX,e.m_nY),  mif.TTLCorner(e.m_nX,e.m_nY),  mif.TTRCorner(e.m_nX,e.m_nY), mif.TTRRCorner(e.m_nX,e.m_nY),
                                        mif.TLCenter(e.m_nX,e.m_nY),   mif.TCenter(e.m_nX,e.m_nY),    mif.TRCenter(e.m_nX,e.m_nY), 
                               mif.TLLCorner(e.m_nX,e.m_nY),   mif.TLCorner(e.m_nX,e.m_nY),   mif.TRCorner(e.m_nX,e.m_nY),  mif.TRRCorner(e.m_nX,e.m_nY),
                                         mif.LCenter(e.m_nX,e.m_nY),  mif.aaCenterMap[e.m_nX][e.m_nY], mif.RCenter(e.m_nX,e.m_nY),
                               mif.BLLCorner(e.m_nX,e.m_nY),   mif.BLCorner(e.m_nX,e.m_nY),   mif.BRCorner(e.m_nX,e.m_nY),  mif.BRRCorner(e.m_nX,e.m_nY),
                                        mif.BLCenter(e.m_nX,e.m_nY),   mif.BCenter(e.m_nX,e.m_nY),    mif.BRCenter(e.m_nX,e.m_nY),
                              mif.BBLLCorner(e.m_nX,e.m_nY),  mif.BBLCorner(e.m_nX,e.m_nY),  mif.BBRCorner(e.m_nX,e.m_nY), mif.BBRRCorner(e.m_nX,e.m_nY));

                  MessageBox(NULL, szError, szErrorHeader, MB_OK);
                  }
               catch (CGenericError e)
                  {
                  MessageBox(NULL, e.m_szErrorDescription, "Moonbase Console Error", MB_OK);
                  }

               return TRUE;
               }

            case IDC_BUTTON2:
               {
               if (!DialogBoxParam(hInstanceG, MAKEINTRESOURCE(IDD_DIALOG2), hwndDlg, JoinDlgProc, 0))
                  return TRUE; // bail if they cancel

               MBCMIF mif;

               // seed the generator
               srand(giG.nSeed);

               switch(giG.nGenerator)
                  {
                  case SPIFF_GEN:
                     SpiffGenerate(giG, mif);
                     break;

                  case KATTON_GEN:
                     KattonGenerate(giG, mif);
                     break;
                  }

               try
                  {
                  BackupSystemMapFile();
                  makewiz(giG.nEnergy, giG.nTerrain, giG.nWater);
                  MakeMap(mif, giG);
                  }
               catch (CGenericError e)
                  {
                  RestoreSystemMapFile();
                  MessageBox(NULL, e.m_szErrorDescription, "Moonbase Console Error", MB_OK);
                  }
               catch (...)
                  {
                  // Shouldn't get map generation errors here, only valid keys should be coming in.
                  }

               JoinGame(hwndDlg, giG);

               return TRUE;
               }
               
            default:
               return FALSE;
            }

      case WM_CLOSE:
         {
         char sz[80];

         wsprintf(sz, "%d", SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_GETPOS, 0, 0));
         WritePrivateProfileString("MoonbaseConsole", "SizeValue", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK1));
         WritePrivateProfileString("MoonbaseConsole", "SizeRandom", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER2), TBM_GETPOS, 0, 0));
         WritePrivateProfileString("MoonbaseConsole", "GeneratorValue", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK2));
         WritePrivateProfileString("MoonbaseConsole", "GeneratorRandom", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER3), TBM_GETPOS, 0, 0));
         WritePrivateProfileString("MoonbaseConsole", "EnergyValue", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK3));
         WritePrivateProfileString("MoonbaseConsole", "EnergyRandom", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER4), TBM_GETPOS, 0, 0));
         WritePrivateProfileString("MoonbaseConsole", "TerrainValue", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK4));
         WritePrivateProfileString("MoonbaseConsole", "TerrainRandom", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER5), TBM_GETPOS, 0, 0));
         WritePrivateProfileString("MoonbaseConsole", "WaterValue", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK5));
         WritePrivateProfileString("MoonbaseConsole", "WaterRandom", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER6), TBM_GETPOS, 0, 0));
         WritePrivateProfileString("MoonbaseConsole", "TilesetValue", sz, szMoonbaseIniFileG);

         wsprintf(sz, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHECK6));
         WritePrivateProfileString("MoonbaseConsole", "TilesetRandom", sz, szMoonbaseIniFileG);

         DestroyWindow(hwndDlg);
         return TRUE;
         }

      case WM_DESTROY:
         CleanUpMoonbaseINI();
         PostQuitMessage(0);
         return TRUE;
      } 
   return FALSE; 
   } 



bool InitializeWindowsStuff (void)
   {
   INITCOMMONCONTROLSEX iccex;

   iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   iccex.dwICC = ICC_INTERNET_CLASSES;

   InitCommonControlsEx(&iccex); // Needed for IP address control

   ZeroMemory(&piG, sizeof(piG));

   return true;
   }

bool InitializeMBCStuff (LPSTR lpCmdLine)
   {
   if (strlen(lpCmdLine))
      {
      strcpy(szMoonbasePathG, lpCmdLine);

      strcpy(szMoonbaseIniFileG, szMoonbasePathG);
      strcat(szMoonbaseIniFileG, "moonbase.ini");
      }
   else
      {
      if (!GetMoonbaseCommanderPath())
         {
         MessageBox(NULL, "Moonbase Commander is not installed. The registry key specifying the path to the Moonbase Commander directory could not be found.", "Moonbase Console Error", MB_OK);
         return false;
         }
      }

   RestoreSystemMapFile();
   CleanUpMoonbaseINI();

   return true;
   }

HHOOK g_hLowLevelKeyHook;
/**
 * This is used to capture keystrokes for redefining keys.
 * Seems hacky but I blame Microsoft
 */
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *)lParam;
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
    {
	if (changeKeyHwnd != NULL){
		UINT result = tolower(MapVirtualKey(pkbhs->vkCode, MAPVK_VK_TO_CHAR));
		if (preferenceSet.insert(result).second == false){
			// Already added, ignore
			return CallNextHookEx(g_hLowLevelKeyHook, nCode, wParam, lParam);
		}
		changedKey = result;
		EndDialog(changeKeyHwnd, TRUE); 
		changeKeyHwnd = NULL;
	}

    }
    return CallNextHookEx(g_hLowLevelKeyHook, nCode, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
   {   
	   g_hLowLevelKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), NULL);

   hInstanceG = hInstance;

   srand((unsigned)time(NULL));

   if (!InitializeWindowsStuff())
      return -1;

   if (!InitializeMBCStuff(lpCmdLine))
      return -1;

   hIconG = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
   hwndDlgModelessG = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), GetDesktopWindow(), (DLGPROC) DlgProc); 
   ShowWindow(hwndDlgModelessG, SW_SHOW); 

   while (TRUE)
      {
      DWORD result;
      MSG msg;
      DWORD cObjects = 0;
      
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
         { 
         if (msg.message == WM_QUIT)  
            return 0; 
         
         if (!IsDialogMessage(hwndDlgModelessG, &msg)) 
            { 
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
            }
         }

      if (piG.hProcess)
         cObjects = 1;
      else
         cObjects = 0;

      result = MsgWaitForMultipleObjectsEx(cObjects, &piG.hProcess, INFINITE, QS_ALLINPUT | QS_ALLPOSTMESSAGE, 0); 
      
      if (result == (WAIT_OBJECT_0 + cObjects))
         continue; // New messages have arrived, continue to the top of the loop to dispatch them and resume waiting.
      else 
         { 
         // MBC has exited
         EnableDialog(hwndDlgModelessG, true);

         int nReplayNumberHighest = FindHighestReplayNumber();

         if (nReplayNumberHighest > nReplayNumberHighestG)
            {
            char szReplay[MAX_PATH];
            sprintf(szReplay, "%suser\\Rep%03d.rep", szMoonbasePathG, nReplayNumberHighest);
            int nReplayMap = GetPrivateProfileInt("SETUP", "5-10", 1, szReplay);
            if (nReplayMap == 1)    // Only copy random map if they used it.
               {
               char szSource[MAX_PATH];
               char szDest[MAX_PATH];

               strcpy(szSource, szMoonbasePathG);
               strcat(szSource, "map\\moon001.map");

               sprintf(szDest, "%suser\\Rep%03d.map", szMoonbasePathG, nReplayNumberHighest);

               CopyFile(szSource, szDest, TRUE);

               WritePrivateProfileString("SETUP", "5-10", "66", szReplay); // Update the replay to reflect the custom map.
               }
            }

         RestoreSystemMapFile();

         CloseHandle(piG.hProcess);
         CloseHandle(piG.hThread);
         ZeroMemory(&piG, sizeof(piG));
         } 
      }
   }
