#include "detokenizer-auxiliary.h"

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

	while (pszParamName) {
		/* запоминаем следующий параметр */
		pszNextParam = (char *) strstr (pszParamName, ";");
		if (pszNextParam) {
			*pszNextParam = '\0';
			++pszNextParam;
		}
		/* получаем указатель на значение параметра */
		pszParamVal = (char *) strstr (pszParamName, "=");
		if (! pszParamVal) {
			goto mk_continue;
		}
		*pszParamVal = '\0';
		++pszParamVal;

		/* контроль наличия всех необходимых параметров */
		if (0 == strcmp ("db_user", pszParamName)) {
			uiParamMask |= 1;
		} else if (0 == strcmp ("db_pswd", pszParamName)) {
			uiParamMask |= 2;
		} else  if (0 == strcmp ("db_descr", pszParamName)) {
			uiParamMask |= 4;
		} else  if (0 == strcmp ("log_file_mask", pszParamName)) {
			uiParamMask |= 8;
		}
		p_coConfig.SetParamValue (pszParamName, std::string (pszParamVal));

mk_continue:
		/* переходим к следующему параметру */
		pszParamName = pszNextParam;
	}

	/* проверяем, все ли нужные параметры мы получили */
	if (! (uiParamMask & (unsigned int) (16 - 1))) {
		iRetVal = uiParamMask;
	}

	return iRetVal;
}
