#include <stdio.h>
#include <stdlib.h>
#include "mmsc/mms_detokenize.h"
#include "mmlib/mms_util.h"
#include "mmsc-detokenizer.h"
#include "detokenizer-auxiliary.h"

static int mms_detokenizer_init (char *settings)
{
	return aux_init (settings);
}

static int mms_detokenizer_fini (void)
{
	return aux_fini ();
}

static Octstr * mms_detokenize (Octstr *token, Octstr *request_ip)
{
	Octstr *psoRetVal = NULL;

	char *pszRequestIP = octstr_get_cstr (request_ip);

	if (NULL == pszRequestIP) {
		return NULL;
	}

	char *pszMSISDN = aux_detokenize (pszRequestIP);

	if (pszMSISDN) {
		psoRetVal = octstr_create (pszMSISDN);
		gw_native_free (pszMSISDN);
	}

	return psoRetVal;
}

static Octstr * mms_gettoken (Octstr *msisdn)
{
	Octstr *psoRetVal = NULL;

	char *pszMSISDN = octstr_get_cstr (msisdn);

	if (NULL == pszMSISDN) {
		return NULL;
	}

	char *pszToken = aux_gettoken (pszMSISDN);

	if (pszToken) {
		psoRetVal = octstr_create (pszToken);
		gw_native_free (pszToken);
	}

	return psoRetVal;
}

/* The function itself. */
MmsDetokenizerFuncStruct mms_detokenizefuncs = {
     mms_detokenizer_init,
     mms_detokenize,
     mms_gettoken,
     mms_detokenizer_fini
};
