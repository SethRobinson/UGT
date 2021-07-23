#include "PlatformPrecomp.h"
#include "UpdateChecker.h"
#include "App.h"


UpdateChecker::UpdateChecker()
{
}

UpdateChecker::~UpdateChecker()
{
}

void UpdateChecker::CheckForUpdate()
{
	if (GetApp()->m_check_for_update_on_startup != "enabled")
	{
		LogMsg("Checking for newer version on startup has been disabled in config.txt");
		return;
	}

	LogMsg("Checking for an update newer than %s", GetApp()->GetAppVersion().c_str());
	string url = "https://www.rtsoft.com/ugt/checking_for_new_version.php";
	string urlappend = "version=" + toString(GetApp()->m_versionNum);

	m_netHTTP.Setup(url, 80, urlappend, NetHTTP::END_OF_DATA_SIGNAL_HTTP);
	m_netHTTP.Start();

}

void UpdateChecker::Update()
{
	if (m_netHTTP.GetState() == m_netHTTP.STATE_IDLE) return;
	m_netHTTP.Update();

	if (m_netHTTP.GetError() != NetHTTP::ERROR_NONE)
	{
		//Big error, show message
		LogMsg("Error checking for update. Error %d, giving up.", (int)m_netHTTP.GetError());
		m_netHTTP.Reset(true);
	}

	if (m_netHTTP.GetState() == NetHTTP::STATE_FINISHED)
	{
		//transaction is finished
		string data = string((char*)m_netHTTP.GetDownloadedData(), m_netHTTP.GetDownloadedBytes());
		
		vector<string> parms = StringTokenize(data, "|");
		if (parms.size() < 5)
		{
			LogMsg("Ignoring update data, there aren't at least six parms");
		}
		else
		{
			//do we need to update?
			int latestVersion = StringToInt(parms[1]);
			string prettyVersion = parms[2];
			string downloadLink = parms[3];
			string updateMsg = parms[4];

			if (latestVersion > GetApp()->m_versionNum)
			{
				int msgboxID = MessageBox(
					NULL,
					updateMsg.c_str(),
					(string("Version ")+prettyVersion+" is available!").c_str(),
					MB_ICONASTERISK | MB_OKCANCEL
				);

				switch (msgboxID)
				{
				case IDOK:
					// TODO: add code
					LogMsg("Let's update");
					LaunchURL(downloadLink);
					GetApp()->OnExitApp(NULL);
					break;
				}
			}
			else
			{
				LogMsg( (GetApp()->GetAppVersion()+" appears to be the latest release.").c_str());
			}

		}

		m_netHTTP.Reset(true);
	}

}

