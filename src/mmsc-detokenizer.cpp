#include <stdio.h>
#include <stdlib.h>
#include "mmsc/mms_detokenize.h"
#include "mmlib/mms_util.h"
#include "mmsc-detokenizer.h"
#include "detokenizer-auxiliary.h"
#include "utils/dbpool/dbpool.h"

CConfig g_coConf;
CLog g_coLog;

static int mms_detokenizer_init (char *settings)
{
	int iRetVal = 0;
	int iFnRes;

	/* ������ ������ �������� */
	iFnRes = detokenizer_apply_settings (settings, g_coConf);
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

static int mms_detokenizer_fini (void)
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
static Octstr *mms_detokenize (Octstr * token, Octstr *request_ip)
{
	int iFnRes;
	int iAttemptLeft = 2;
	otl_connect *pcoDBConn;
	const char *pszIPAddress;
	char mcMSISDN[256];

	/* �������� ��������� �� ������ */
	pszIPAddress = octstr_get_cstr (request_ip);
	if (NULL == pszIPAddress) {
		return NULL;
	}

	/* ����������� ��������� ������ ������ ����������� � �� */
	pcoDBConn = db_pool_get ();
	if (NULL == pcoDBConn) {
		return NULL;
	}
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
			<< pszIPAddress;
		coStream
			>> mcMSISDN;
	} catch (otl_exception &coExept) {
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

	return octstr_create (mcMSISDN);
}

/* Given an msisdn, returns the token associated
 * Return NULL on error, otherwise an Octstr
 */
static Octstr *mms_gettoken (Octstr *msisdn)
{
	int iFnRes;
	int iAttemptLeft = 2;
	otl_connect *pcoDBConn;
	const char *pszMSISDN;
	char mcIPAddress[256];

	/* �������� ��������� �� ������ */
	pszMSISDN = octstr_get_cstr (msisdn);
	if (NULL == pszMSISDN) {
		return NULL;
	}

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
			<< pszMSISDN;
		coStream
			>> mcIPAddress;
	} catch (otl_exception &coExept) {
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

	return octstr_create (mcIPAddress);
}

/* The function itself. */
MmsDetokenizerFuncStruct mms_detokenizefuncs = {
     mms_detokenizer_init,
     mms_detokenize,
     mms_gettoken,
     mms_detokenizer_fini
};
