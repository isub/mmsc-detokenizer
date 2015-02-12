#ifndef _DETOKENIZER_AUXILIARY_H_
#define _DETOKENIZER_AUXILIARY_H_

#include "utils/config/config.h"
#include "utils/log/log.h"

int detokenizer_apply_settings (
	const char *p_pszSettings,
	CConfig &p_coConfig);

#endif