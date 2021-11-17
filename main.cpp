
//#define _AFXDLL
#include "stdafx.h"
#include "ping.h"
#include "XSleep.h"
#include <mmsystem.h>
#include <fstream>
#include <csignal>
#include "windows.h"
#include <string.h>
#pragma comment( lib, "Winmm.lib" )

using namespace std;

int nRequestsSent = 0;
int failed = 0;
int makenoise = 0;
int thiscount = 0;
char thistimestamp[];

void term(int whatsig)
{
	cout << "\n\nControl-C" << endl;
	cout << "Pings Sent: " << nRequestsSent << endl << "Pings Failed: " << failed << endl;
	exit(1);
}

class CMyApp : public CWinApp
{
  virtual BOOL InitInstance();

};



CMyApp theApp;
int Interval;
int ColorBlocks;



class CMyCommandLineInfo : public CCommandLineInfo
{
public:
  CMyCommandLineInfo();
  virtual void ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);

//Our specific command line options
  CString m_sHostToResolve;
  CString m_sLocalBoundAddress;
  BOOL    m_bResolveAddressesToHostnames;
  BOOL    m_bPingTillStopped;
  int     m_nRequestsToSend;
  UCHAR   m_nTTL;
  UCHAR   m_nTOS;
  WORD    m_wDataRequestSize;
  DWORD   m_dwTimeout;
  BOOL    m_bUseRawSockets;
  BOOL    m_bDontFragment;
  int	  Interval;
  CString Filename;
  BOOL	  m_bWriteToTextfile;
};

CMyCommandLineInfo::CMyCommandLineInfo()
{
  m_bResolveAddressesToHostnames = FALSE;
  m_bPingTillStopped = TRUE;
  m_nRequestsToSend = 1;
  m_nTTL = 30;
  m_wDataRequestSize = 32;
  m_dwTimeout = 5000;
  m_bUseRawSockets = FALSE;
  ColorBlocks = 0;
  m_nTOS = 0;
  m_bDontFragment  = FALSE;
  Interval = 800;
  m_bWriteToTextfile = FALSE;
 
}



void CMyCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
  if (bFlag)
  {
    //Form a CString version of the parameter (just makes it easier to parse with)
    CString sParam(lpszParam);

	  if (_tcsicmp(lpszParam, _T("a")) == 0)
		  m_bResolveAddressesToHostnames = TRUE;
    else if (_tcsicmp(lpszParam, _T("t")) == 0)
		  m_bPingTillStopped = TRUE;
    else if (_tcsicmp(lpszParam, _T("f")) == 0)
		  m_bDontFragment = TRUE;
    else if (_tcsicmp(lpszParam, _T("m")) == 0)
		  m_bUseRawSockets = FALSE;
    else if (_tcsicmp(lpszParam, _T("s")) == 0)
		  m_bUseRawSockets = TRUE;
	else if (_tcsicmp(lpszParam, _T("o")) == 0)
		  m_bWriteToTextfile = TRUE;
	else if (_tcsicmp(lpszParam, _T("c")) == 0)
		  ColorBlocks = 1;
	else if (sParam.Find(_T("n:")) == 0 && sParam.GetLength() > 2)
      m_nRequestsToSend = _ttoi(sParam.Right(sParam.GetLength() - 2));
    else if (sParam.Find(_T("e:")) == 0 && sParam.GetLength() > 2)
      Interval = _ttoi(sParam.Right(sParam.GetLength() - 2));
	else if (sParam.Find(_T("i:")) == 0 && sParam.GetLength() > 2)
      m_nTTL = static_cast<UCHAR>(_ttoi(sParam.Right(sParam.GetLength() - 2)));
    else if (sParam.Find(_T("l:")) == 0 && sParam.GetLength() > 2)
      m_wDataRequestSize = static_cast<WORD>(_ttoi(sParam.Right(sParam.GetLength() - 2)));
    else if (sParam.Find(_T("v:")) == 0 && sParam.GetLength() > 2)
      m_nTOS = static_cast<UCHAR>(_ttoi(sParam.Right(sParam.GetLength() - 2)));
    else if (sParam.Find(_T("w:")) == 0 && sParam.GetLength() > 2)
      m_dwTimeout = static_cast<DWORD>(_ttoi(sParam.Right(sParam.GetLength() - 2)));

	
else if (sParam.Find(_T("b:")) == 0 && sParam.GetLength() > 2)
    {
      m_sLocalBoundAddress = sParam.Right(sParam.GetLength() - 2);
      m_bUseRawSockets = TRUE; //Can only use a bound address when using raw sockets
    }
    else
      CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
  }
  else
    m_sHostToResolve = lpszParam;;
}

BOOL CMyApp::InitInstance()
{
  //Parse the command line
  CMyCommandLineInfo rCmdInfo;
	for (int i = 1; i < __argc; i++)
	{
    #ifdef _UNICODE
		LPCTSTR pszParam = __wargv[i];
    #else
    LPCTSTR pszParam = __argv[i];
    #endif
		BOOL bFlag = FALSE;
		BOOL bLast = ((i + 1) == __argc);
		if (pszParam[0] == _T('-') || pszParam[0] == _T('/'))
		{
			// remove flag specifier
			bFlag = TRUE;
			++pszParam;
		}
		rCmdInfo.ParseParam(pszParam, bFlag, bLast);
	}


  //Check that the host to ping has been specified
  if (rCmdInfo.m_sHostToResolve.IsEmpty())
  {
	_tprintf(_T("Usage: dping [-t] [-a] [-n:count] [-l:size] [-i:TTL] [-e:interval] \n"));
    _tprintf(_T("             [-w timeout] [-m | -s] [-b:BoundAdress] destination\n\n"));
    _tprintf(_T("  Options:\n"));
    _tprintf(_T("        -t              Ping the specified host until stopped.\n"));
    _tprintf(_T("                        To stop - type Control-C.\n"));
	_tprintf(_T("        -e:time		Ping Interval (Default: 800ms)\n"));
	_tprintf(_T("                        1000ms = 1sec\n"));
	_tprintf(_T("        -a              Resolve addresses to hostnames.\n"));
    _tprintf(_T("        -n:count        Number of echo requests to send.\n"));
    _tprintf(_T("        -l:size         Send data buffer size.\n"));
    _tprintf(_T("        -f              Set Don't Fragment flag in packet.\n"));
    _tprintf(_T("        -i:TTL          Time To Live.\n"));
    _tprintf(_T("        -v:TOS          Type Of Service.\n"));
	_tprintf(_T("        -c		Colorized blocks.\n")); 
	_tprintf(_T("        -w:timeout      Timeout in milliseconds to wait for each reply.\n"));
#ifndef CPING_NO_ICMP
    _tprintf(_T("        -m              Use IP Helper (or as a fallback ICMP dll) to do ping.\n"));
#endif
#ifndef CPING_NO_WINSOCK2
    _tprintf(_T("        -s              Use raw sockets.\n"));
#endif
    _tprintf(_T("        -b:boundaddress The local address to bind to.\n"));

    return FALSE;
  }
  cout << "Interval is " << rCmdInfo.Interval <<"ms" << " (Change with -e:)" << endl;
  if (ColorBlocks) 
  {
	  // cout << "Color block mode is enabled" << endl;
	  system("cls");
	  cout << "Pinging host " << rCmdInfo.m_sHostToResolve << " at ";
	  timestamp();
	  cout << "in color mode. Control-C to abort." << endl;
	  // Open a file

		
  }

signal(SIGINT, term);

//  if (!outdata) {
//	  cerr << "Error: File cannot be opened." << endl;
//	  exit(1);
//  }



  if (rCmdInfo.m_wDataRequestSize > 65500)
  {
    _tprintf(_T("Bad value for option -l. valid range is 0 to 65500.\n"));
    return FALSE;
  }

  //Initialize sockets
  if (rCmdInfo.m_bUseRawSockets)
  {
	  WSADATA wsa;
	  if (WSAStartup(MAKEWORD(2, 0), &wsa) != 0)
    {
      _tprintf(_T("Failed to initialize Winsock 2 Stack\n"));
      return FALSE;
    }
  }
  else
  {
	  WSADATA wsa;
	  if (WSAStartup(MAKEWORD(1, 1), &wsa) != 0)
    {
      _tprintf(_T("Failed to initialize Winsock 1.1 Stack\n"));
      return FALSE;
    }
  }

  BOOL bSuccess = FALSE;
	CPing p;
	CPingReply pr;

  while (1)
  {
    if (rCmdInfo.m_bUseRawSockets)
    {
#ifndef CPING_NO_WINSOCK2
      if (rCmdInfo.m_sLocalBoundAddress.IsEmpty())
        bSuccess = p.PingUsingWinsock(rCmdInfo.m_sHostToResolve, pr, rCmdInfo.m_nTTL, rCmdInfo.m_dwTimeout, 
                                      rCmdInfo.m_wDataRequestSize, rCmdInfo.m_nTOS, rCmdInfo.m_bDontFragment);
      else
	  {
		  cout << "Interval is " << rCmdInfo.Interval <<"ms" << endl;
		  bSuccess = p.PingUsingWinsock(rCmdInfo.m_sHostToResolve, pr, rCmdInfo.m_nTTL, rCmdInfo.m_dwTimeout, 
                                      rCmdInfo.m_wDataRequestSize, rCmdInfo.m_nTOS, rCmdInfo.m_bDontFragment, rCmdInfo.m_sLocalBoundAddress);
	  }


#endif
    }
    else
    {
#ifndef CPING_NO_ICMP
  	  
		bSuccess = p.PingUsingICMP(rCmdInfo.m_sHostToResolve, pr, rCmdInfo.m_nTTL, rCmdInfo.m_dwTimeout, 
                                 rCmdInfo.m_wDataRequestSize, rCmdInfo.m_nTOS, rCmdInfo.m_bDontFragment);
#endif
    }

    ++nRequestsSent;
	if (bSuccess)
	  {
      hostent* phostent = NULL;
      if (rCmdInfo.m_bResolveAddressesToHostnames)
		    phostent = gethostbyaddr(reinterpret_cast<char *>(&pr.Address.S_un.S_addr), 4, PF_INET);
	  if (phostent)
      {
        CString sHost(phostent->h_name);
		//Google comparing strings...
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),2);		
		_tprintf(_T("%d.%d.%d.%d [%s], reply :%dms (%d/%d)\n"), 
					     pr.Address.S_un.S_un_b.s_b1, pr.Address.S_un.S_un_b.s_b2, pr.Address.S_un.S_un_b.s_b3, 
					     pr.Address.S_un.S_un_b.s_b4, sHost.operator LPCTSTR(), pr.RTT, nRequestsSent, failed);
	  }
      else
		  
		  if (ColorBlocks == 1)
		  {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),BACKGROUND_GREEN);
				cout << " " ;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),7);
			// If we've been down, then play a sound to announce we're back up.
			if (makenoise >= 1)
			{
				makenoise++;
				if (makenoise == 5)
					makenoise=0;	
				else if (makenoise > 2) 
					PlaySound("E:\\cping\\sonar.wav",NULL,SND_NODEFAULT);
				thiscount=2;
				if (thiscount != 0)
				{
					int outagetime = thiscount*Interval;
					ofstream myfile;
					myfile.open("E:\\monitor.txt");
					myfile << "Outage Begins...TIMESTAMP";
					myfile << thiscount << " pings missed at " << Interval << "ms " << outagetime;
					thiscount = 0;
					
				}

			}

		  }
		  else
		  {
		  // 4 red 2 green 12 intense red 
		  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),2);
		  cout << "o ";
		  //printf("\a");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),7);
		  timestamp();  

		  _tprintf(_T("[%d.%d.%d.%d] reply %dms \t (p:%d/f:%d)\n"), 
						pr.Address.S_un.S_un_b.s_b1, pr.Address.S_un.S_un_b.s_b2, pr.Address.S_un.S_un_b.s_b3, 
					     pr.Address.S_un.S_un_b.s_b4, pr.RTT, nRequestsSent, failed);

		  }
		  }
	  else
	  {

		 if (ColorBlocks == 1)
		 {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),BACKGROUND_RED);
				cout << " " ;
				++failed;
				if (makenoise == 0)
				{
					PlaySound("C:\\windows\\media\\Windows Ding.wav",NULL,SND_NODEFAULT);
					makenoise=1;
				}
					thiscount++;
					if (thiscount == 1)
					{

					}

				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),7);
		 }
		 else 
		 {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),12);
		cout << "x ";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),7);
		  timestamp();
	      cout << "Request Timed out \t\t(" << ++failed << " of "  << nRequestsSent << " packets have failed)" << endl;
//		  outdata << "Request Timed out \t\t(" << failed << " of "  << nRequestsSent << " packets have failed)" << endl;
		  
		 }}


    //Prepare for the next loop around?
    //Sleep added by D. Tate to delay the ping retry.
	  XSleep(rCmdInfo.Interval);
	  if (!rCmdInfo.m_bPingTillStopped)
    {
      if (nRequestsSent == rCmdInfo.m_nRequestsToSend)
        break;
	 
    }
  }


  //Cleanup usage of winsock
  WSACleanup();
  return FALSE;
  // stop (hammertime); - for J :)
}


