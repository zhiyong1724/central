#include "jsinterpreter.h"
#include <stdio.h>
#include "quickjs.h"
#include "osmem.h"
#include "quickjs-libc.h"
#include <malloc.h>
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

int jsInterpreterStart(long argc, char *argv[])
{
    if (argc >= 2)
    {
        JSRuntime *rt = JS_NewRuntime2(&sJSMallocFunctions, NULL);
        if (rt != NULL)
        {
            JSContext *context = JS_NewContext(rt);
            if (context != NULL)
            {
                js_std_add_helpers(context, argc, argv);
                js_init_module_std(context, "std");
                size_t len = 0;
                uint8_t *buff = js_load_file(context, &len, argv[1]);
                if (buff != NULL)
                {
                    JSValue val = JS_Eval(context, (const char *)buff, len, argv[1], JS_EVAL_TYPE_GLOBAL);
                    if (JS_IsException(val))
                    {
                        js_std_dump_error(context);
                    }
                    JS_FreeValue(context, val);
                    js_free(context, buff);
                }
                else
                {
                    perror(argv[1]);
                }
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
    jsInterpreterStart(argc, argv);
}