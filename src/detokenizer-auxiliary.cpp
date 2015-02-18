#include <string.h>
#include "detokenizer-auxiliary.h"
#include "utils/dbpool/dbpool.h"

#include "utils/config/config.h"
#include "utils/log/log.h"
#include <string>

CConfig g_coConf;
CLog g_coLog;

int detokenizer_apply_settings (
	const char *p_pszSettings,
	CConfig &p_coConfig);

int aux_init (const char *p_pszSettings)
{
	int iRetVal = 0;
	int iFnRes;

	/* ������ ������ �������� */
	iFnRes = detokenizer_apply_settings (p_pszSettings, g_coConf);
	if (iFnRes) {
		return -1;
	}

	std::string strConfParam;

	/* ����������� ����� ��� ����� */
	iFnRes = g_coConf.GetParamValue ("log_file_mask", strConfParam);
	if (iFnRes) {
		return -2;
	}

	/* ������������� ������� */
	iFnRes = g_coLog.Init (strConfParam.c_str ());
	if (iFnRes) {
		return -3;
	}

	/* ������������� ���� ����������� � �� */
	iFnRes = db_pool_init (&g_coLog, &g_coConf);
	if (iFnRes) {
		return -4;
	}

	return iRetVal;
}

int aux_fini ()
{
	/* ����������� �������, ������� ����� ����������� � �� */
	db_pool_deinit ();

	/* ���������� �� ���� ���������������� ���������� ��� ����� */
	g_coLog.Flush ();

	return 0;
}

/* Looks up the token and returns the msisdn as a new Octstr.
 * Return NULL on error, otherwise an Octstr
 */
char * aux_detokenize (const char *p_pszRequestIP)
{
	int iFnRes;
	int iAttemptLeft = 2;
	otl_connect *pcoDBConn = NULL;
	char mcMSISDN[256];
	char *pszRetVal = NULL;
	std::string strMSISDN;

	do {
		/* ����������� ��������� ������ ������ ����������� � �� */
		pcoDBConn = db_pool_get ();
		if (NULL == pcoDBConn) {
			break;
		}
	try_again:
		/* ��������� ������ � �� */
		try {
			otl_stream coStream;
			coStream.open (
				1,
				"select "
					"msisdn "
				"from "
					"ps.gi_radacct_simple "
				"where "
					"ip_address = :ip_address /* char[32] */ "
					"and finish_time is null "
				"order by "
					"start_time desc",
				*pcoDBConn);
			coStream
				<< p_pszRequestIP;
			coStream
				>> mcMSISDN;
			switch (strlen (mcMSISDN)) {
			case 10:
				strMSISDN = "+7";
				break;
			case 11:
				strMSISDN += '+';
				break;
			}
			strMSISDN += mcMSISDN;
			pszRetVal = strdup (strMSISDN.c_str ());
		} catch (otl_exception &coExept) {
			g_coLog.WriteLog ("%s[%d]: %s: error: code: '%d'; description: '%s'", __FILE__, __LINE__, __FUNCTION__, coExept.code, coExept.msg);
			/* �������� ����������������� ����������� ���� ���������� ������� ��� �� ������������� */
			if (-- iAttemptLeft) {
				iFnRes = db_pool_check (*pcoDBConn);
				/* ���� ����������� ���������� ������������ */
				if (iFnRes) {
					iFnRes = db_pool_reconnect (*pcoDBConn);
					/* ���� ����������� ������������� */
					if (0 == iFnRes) {
						/* ��������� ��� ��� */
						goto try_again;
					}
				}
			}
		}
	} while (0);

	if (pcoDBConn) {
		db_pool_release (pcoDBConn);
	}

	if (pszRetVal) {
		g_coLog.WriteLog ("%s: ip-address: '%s'; msisdn: '%s'", __FUNCTION__, p_pszRequestIP, strMSISDN.c_str ());
	} else {
		g_coLog.WriteLog ("%s: ip-address: '%s'; msisdn: '%s'", __FUNCTION__, p_pszRequestIP, "not found");
	}

	return pszRetVal;
}

/* Given an msisdn, returns the token associated
 * Return NULL on error, otherwise an Octstr
 */
char * aux_gettoken (const char *p_pszMSISDN)
{
	int iFnRes;
	int iAttemptLeft = 2;
	otl_connect *pcoDBConn;
	char mcIPAddress[256];
	char *pszRetVal = NULL;

	/* ����������� ��������� ������ ������ ����������� � �� */
	pcoDBConn = db_pool_get ();
	if (NULL == pcoDBConn) {
		return NULL;
	}
try_again:
	/* ��������� ������ � �� */
	try {
		otl_stream coStream;
		coStream.open (
			1,
			"select "
				"ip_address "
			"from "
				"ps.gi_radacct_simple "
			"where "
				"msisdn = :msisdn /* char[32] */ "
				"and finish_time is null "
			"order by "
				"start_time desc",
			*pcoDBConn);
		coStream
			<< p_pszMSISDN;
		coStream
			>> mcIPAddress;
		pszRetVal = strdup (mcIPAddress);
	} catch (otl_exception &coExept) {
		g_coLog.WriteLog ("%s[%d]: %s: error: code: '%d'; description: '%s'", __FILE__, __LINE__, __FUNCTION__, coExept.code, coExept.msg);
		/* �������� ����������������� ����������� ���� ���������� ������� ��� �� ������������� */
		if (-- iAttemptLeft) {
			iFnRes = db_pool_check (*pcoDBConn);
			/* ���� ����������� ���������� ������������ */
			if (iFnRes) {
				iFnRes = db_pool_reconnect (*pcoDBConn);
				/* ���� ����������� ������������� */
				if (0 == iFnRes) {
					/* ��������� ��� ��� */
					goto try_again;
				}
			}
		}
		db_pool_release (pcoDBConn);
		return NULL;
	}

	db_pool_release (pcoDBConn);

	return pszRetVal;
}

int detokenizer_apply_settings (
	const char *p_pszSettings,
	CConfig &p_coConfig)
{
	int iRetVal = 0;
	int iFnRes;
	const char *pszParamName;
	char *pszNextParam;
	char *pszParamVal;
	unsigned int uiParamMask = 0;

	pszParamName = p_pszSettings;
	std::string strParamVal;

	while (pszParamName) {
		/* ���������� ��������� �������� */
		pszNextParam = (char *) strstr (pszParamName, ";");
		if (pszNextParam) {
			*pszNextParam = '\0';
			++pszNextParam;
		}
		/* �������� ��������� �� �������� ��������� */
		pszParamVal = (char *) strstr (pszParamName, "=");
		if (! pszParamVal) {
			goto mk_continue;
		}
		*pszParamVal = '\0';
		++pszParamVal;

		/* �������� ������� ���� ����������� ���������� */
		if (0 == strcmp ("db_user", pszParamName)) {
			uiParamMask |= 1;
		} else if (0 == strcmp ("db_pswd", pszParamName)) {
			uiParamMask |= 2;
		} else  if (0 == strcmp ("db_descr", pszParamName)) {
			uiParamMask |= 4;
		} else  if (0 == strcmp ("log_file_mask", pszParamName)) {
			uiParamMask |= 8;
		}
		strParamVal = pszParamVal;
		p_coConfig.SetParamValue (pszParamName, strParamVal);

mk_continue:
		/* ��������� � ���������� ��������� */
		pszParamName = pszNextParam;
	}

	/* ���������, ��� �� ������ ��������� �� �������� */
	if (! (uiParamMask & (unsigned int) (16 - 1))) {
		iRetVal = uiParamMask;
	}

	return iRetVal;
}
