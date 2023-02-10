#ifndef __JSLIBC_H__
#define __JSLIBC_H__
#include "quickjs.h"
void jsDumpError(JSContext *ctx);
void jsInitGlobalObject(JSContext *ctx);
void jsInitStdModule(JSContext *ctx);
#endif