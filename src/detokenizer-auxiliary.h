#ifndef _DETOKENIZER_AUXILIARY_H_
#define _DETOKENIZER_AUXILIARY_H_

#ifdef __cplusplus
extern "C" {
#endif

int aux_init (const char *p_pszSettings);

int aux_fini ();

char * aux_detokenize (const char *p_pszRequestIP);

char * aux_gettoken (const char *p_pszMSISDN);

#ifdef __cplusplus
}
#endif

#endif