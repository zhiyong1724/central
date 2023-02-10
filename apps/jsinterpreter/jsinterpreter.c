#include "jsinterpreter.h"
#include <stdio.h>
#include "quickjs.h"
#include "osmem.h"
#include <malloc.h>
#include "jslibc.h"
#include <assert.h>
#include <stdlib.h>
static size_t sMax = 0;
static void *jsMalloc(JSMallocState *s, size_t size)
{
    void *ret = NULL;
    if (s->malloc_size + size <= s->malloc_limit)
    {
        ret = osMalloc(size);
        if (ret != NULL)
        {
            s->malloc_count++;
            s->malloc_size += osMallocUsableSize(ret);
        }
    }
    sMax = sMax > s->malloc_count ? sMax : s->malloc_count;
    return ret;
}

static void jsFree(JSMallocState *s, void *ptr)
{
    if (ptr != NULL)
    {
        s->malloc_count--;
        s->malloc_size -= osMallocUsableSize(ptr);
        osFree(ptr);
    }
}

static void *jsRealloc(JSMallocState *s, void *ptr, size_t size)
{
    void *ret = NULL;
    if (s->malloc_size + size <= s->malloc_limit)
    {
        if (ptr != NULL)
        {
            s->malloc_count--;
            s->malloc_size -= osMallocUsableSize(ptr);
        }
        ret = osRealloc(ptr, size);
        if (ret != NULL)
        {
            s->malloc_count++;
            s->malloc_size += osMallocUsableSize(ret);
        }
    }
    sMax = sMax > s->malloc_count ? sMax : s->malloc_count;
    return ret;
}

static size_t jsMallocUsableSize(const void *ptr)
{
    return osMallocUsableSize((void *)ptr);
}

static JSMallocFunctions sJSMallocFunctions = 
{
    jsMalloc,
    jsFree,
    jsRealloc,
    jsMallocUsableSize,
};

static int evalBuf(JSContext *ctx, const void *buf, int bufLen, const char *fileName, int evalFlags)
{
    JSValue val;
    int ret;
    if ((evalFlags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE)
    {
        val = JS_Eval(ctx, buf, bufLen, fileName,
                      evalFlags | JS_EVAL_FLAG_COMPILE_ONLY);
        if (!JS_IsException(val))
        {
            val = JS_EvalFunction(ctx, val);
        }
    }
    else
    {
        val = JS_Eval(ctx, buf, bufLen, fileName, evalFlags);
    }
    if (JS_IsException(val))
    {
        jsDumpError(ctx);
        ret = -1;
    }
    else
    {
        ret = 0;
    }
    JS_FreeValue(ctx, val);
    return ret;
}

static uint8_t *loadFile(JSContext *context, size_t *len, const char *fileName)
{
    uint8_t *ret = NULL;
    FILE *file = fopen(fileName, "rb");
    if (file != NULL)
    {
        fseek(file, 0, SEEK_END);
        *len = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (*len > 0)
        {
            ret = (uint8_t *)js_malloc(context, *len + 1);
            if (ret != NULL)
            {
                fread(ret, 1, *len, file);
                ret[*len] = 0;
            }
        }
        fclose(file);
    }
    return ret;
}

static JSModuleDef *jsModuleLoader(JSContext *ctx, const char *moduleName, void *opaque)
{
    JSModuleDef *m = NULL;
    size_t len = 0;
    uint8_t *buff = loadFile(ctx, &len, moduleName);
    if (buff != NULL)
    {
        JSValue val = JS_Eval(ctx, (const char *)buff, len, moduleName, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
        if (!JS_IsException(val))
        {
            m = JS_VALUE_GET_PTR(val);
        }
        else
        {
            jsDumpError(ctx);
        }
        JS_FreeValue(ctx, val);
        js_free(ctx, buff);
    }
    else
    {
        perror(moduleName);
    }
    return m;
}

static int evalFile(JSContext *ctx, const char *fileName)
{
    int ret = -1;
    size_t len = 0;
    uint8_t *buff = loadFile(ctx, &len, fileName);
    if (!buff) {
        perror(fileName);
        return ret;
    }
    if (JS_DetectModule((const char *)buff, len))
    {
        ret = evalBuf(ctx, buff, len, fileName, JS_EVAL_TYPE_MODULE);
    }
    else
    {
        ret = evalBuf(ctx, buff, len, fileName, JS_EVAL_TYPE_GLOBAL);
    }
    js_free(ctx, buff);
    return ret;
}

int jsInterpreterStart(long argc, char *argv[])
{
    if (argc >= 2)
    {
        JSRuntime *rt = JS_NewRuntime2(&sJSMallocFunctions, NULL);
        if (rt != NULL)
        {
            JS_SetModuleLoaderFunc(rt, NULL, jsModuleLoader, NULL);
            JSContext *context = JS_NewContext(rt);
            if (context != NULL)
            {
                jsInitGlobalObject(context);
                jsInitStdModule(context);
                evalFile(context, argv[1]);
                JS_FreeContext(context);
            }
            JS_FreeRuntime(rt);
        }
    }
    else
    {
        printf("没有输入文件。\n");
    }
    return 0;
}

void shellJS(long argc, char *argv[])
{
    system("stty echo");
    jsInterpreterStart(argc, argv);
    system("stty -echo");
}