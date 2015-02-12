#include "mmsc/mms_detokenize.h"

#ifdef __cplusplus
extern "C" {
#endif

static int mms_detokenizer_init (char *settings);

static int mms_detokenizer_fini (void);

static Octstr *mms_detokenize (Octstr *token, Octstr *request_ip);

static Octstr *mms_gettoken (Octstr *msisdn);

MmsDetokenizerFuncStruct mms_detokenizefuncs = {
	mms_detokenizer_init,
	mms_detokenize,
	mms_gettoken,
	mms_detokenizer_fini
};

#ifdef __cplusplus
}
#endif
