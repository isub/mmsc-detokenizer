#include "mmsc/mms_detokenize.h"

static int mms_detokenizer_init (char *settings);

static int mms_detokenizer_fini (void);

static Octstr *mms_detokenize (Octstr *token, Octstr *request_ip);

static Octstr *mms_gettoken (Octstr *msisdn);
