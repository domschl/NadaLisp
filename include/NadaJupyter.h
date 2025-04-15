#ifndef NADA_JUPYTER_H
#define NADA_JUPYTER_H

#include "NadaValue.h"
#include "NadaEnv.h"

NadaValue *builtin_display_markdown(NadaValue *args, NadaEnv *env);
NadaValue *builtin_display_html(NadaValue *args, NadaEnv *env);

#endif  // NADA_JUPYTER_H
