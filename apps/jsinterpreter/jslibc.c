#include "jslibc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ostask.h"
#define countof(x) (sizeof(x) / sizeof((x)[0]))
static void dumpObj(JSContext *ctx, FILE *f, JSValueConst val)
{
    const char *str;

    str = JS_ToCString(ctx, val);
    if (str)
    {
        fprintf(f, "%s\n", str);
        JS_FreeCString(ctx, str);
    }
    else
    {
        fprintf(f, "[exception]\n");
    }
}

static void dumpError(JSContext *ctx, JSValueConst exception_val)
{
    JSValue val;
    int is_error;

    is_error = JS_IsError(ctx, exception_val);
    dumpObj(ctx, stderr, exception_val);
    if (is_error)
    {
        val = JS_GetPropertyStr(ctx, exception_val, "stack");
        if (!JS_IsUndefined(val))
        {
            dumpObj(ctx, stderr, val);
        }
        JS_FreeValue(ctx, val);
    }
}

void jsDumpError(JSContext *ctx)
{
    JSValue exception_val;
    exception_val = JS_GetException(ctx);
    dumpError(ctx, exception_val);
    JS_FreeValue(ctx, exception_val);
}

static JSValue jsPrintf(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    int i;
    const char *str;
    size_t len;

    for (i = 0; i < argc; i++)
    {
        if (i != 0)
            putchar(' ');
        str = JS_ToCStringLen(ctx, &len, argv[i]);
        if (!str)
            return JS_EXCEPTION;
        fwrite(str, 1, len, stdout);
        JS_FreeCString(ctx, str);
    }
    return JS_UNDEFINED;
}

static JSValue jsLog(JSContext *ctx, JSValueConst this_val,
                     int argc, JSValueConst *argv)
{
    jsPrintf(ctx, this_val, argc, argv);
    putchar('\n');
    return JS_UNDEFINED;
}

void jsInitGlobalObject(JSContext *ctx)
{
    JSValue globalObj = JS_GetGlobalObject(ctx);
    JSValue console = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, console, "log",
                      JS_NewCFunction(ctx, jsLog, "log", 1));
    JS_SetPropertyStr(ctx, globalObj, "console", console);
    JS_FreeValue(ctx, globalObj);
}

static JSValue jsScanf(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    if (0 == argc)
    {
        char input[1024];
        for (size_t i = 0; i < 1024; i++)
        {
            for (;;)
            {
                int value = getchar();
                if (value != EOF)
                {
                    if (value != '\n')
                    {
                        input[i] = (char)value;
                        break;
                    }
                    else
                    {
                        input[i] = '\0';
                        goto finally;
                    }
                }
                else
                {
                    osTaskSleep(1);
                }
            }
        }
        finally:
        {
            JSValue jsInput = JS_NewString(ctx, input);
            return jsInput;
        }
    }
    return JS_UNDEFINED;
}

static JSClassID sFileClassId;
static void fileClassFinalizer(JSRuntime *rt, JSValue val)
{
    FILE *file = (FILE *)JS_GetOpaque(val, sFileClassId);
    if (file != NULL && file != stderr && file != stdin && file != stdout)
    {
        fclose(file);
        JS_SetOpaque(val, NULL);
    }
}

static JSClassDef sFileClassDef = 
{
    "FILE",
    fileClassFinalizer,
};

static JSValue jsFOpen(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue ret = JS_UNDEFINED;
    if (2 == argc)
    {
        const char *mode = NULL;
        const char *name = JS_ToCString(ctx, argv[0]);
        if (name != NULL)
        {
            mode = JS_ToCString(ctx, argv[1]); 
            if (mode != NULL)
            {
                FILE *file = fopen(name, mode);
                if (file != NULL)
                {
                    ret = JS_NewObjectClass(ctx, sFileClassId);
                    JS_SetOpaque(ret, file);
                }
            }
        }
        if (name != NULL)
        {
            JS_FreeCString(ctx, name);
        }
        if (mode != NULL)
        {
            JS_FreeCString(ctx, mode);
        }
    }
    return ret;
}

static JSValue jsFClose(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    if (1 == argc)
    {
        FILE *file = (FILE *)JS_GetOpaque(argv[0], sFileClassId);
        if (file != NULL)
        {
            fclose(file);
            JS_SetOpaque(argv[0], NULL);
        }
    }
    return JS_UNDEFINED;
}

static JSValue jsFRead(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue ret = JS_NewInt64(ctx, 0);
    if (4 == argc)
    {
        FILE *file = (FILE *)JS_GetOpaque(argv[3], sFileClassId);
        if (file != NULL)
        {
            size_t len;
            uint8_t *array = JS_GetArrayBuffer(ctx, &len, argv[0]);
            if (array != NULL && len > 0)
            {
                int64_t size, count;
                if (JS_ToInt64(ctx, &size, argv[1]) == 0 && JS_ToInt64(ctx, &count, argv[2]) == 0)
                {
                    len = fread(array, size, count, file);
                    ret = JS_NewInt64(ctx, len);
                }
            }
        }
    }
    return ret;
}

static JSValue jsFWrite(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue ret = JS_NewInt64(ctx, 0);
    if (4 == argc)
    {
        FILE *file = (FILE *)JS_GetOpaque(argv[3], sFileClassId);
        if (file != NULL)
        {
            size_t len;
            uint8_t *array = JS_GetArrayBuffer(ctx, &len, argv[0]);
            if (array != NULL && len > 0)
            {
                int64_t size, count;
                if (JS_ToInt64(ctx, &size, argv[1]) == 0 && JS_ToInt64(ctx, &count, argv[2]) == 0)
                {
                    len = fwrite(array, size, count, file);
                    ret = JS_NewInt64(ctx, len);
                }
            }
        }
    }
    return ret;
}

static JSValue jsFTell(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue ret = JS_NewInt64(ctx, 0);
    if (1 == argc)
    {
        FILE *file = (FILE *)JS_GetOpaque(argv[0], sFileClassId);
        if (file != NULL)
        {
            int64_t len = ftell(file);
            ret = JS_NewInt64(ctx, len);
        }
    }
    return ret;
}

static JSValue jsFSeek(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue ret = JS_NewInt32(ctx, -1);
    if (3 == argc)
    {
        FILE *file = (FILE *)JS_GetOpaque(argv[0], sFileClassId);
        if (file != NULL)
        {
            int64_t offset, whence;
            if (JS_ToInt64(ctx, &offset, argv[1]) == 0 && JS_ToInt64(ctx, &whence, argv[2]) == 0)
            {
                int result = fseek(file, offset, whence);
                ret = JS_NewInt32(ctx, result);
            }
        }
    }
    return ret;
}

static const JSCFunctionListEntry sJsStdFuncs[] = {
    JS_CFUNC_DEF("printf", 1, jsPrintf),
    JS_CFUNC_DEF("scanf", 0, jsScanf),
    JS_CFUNC_DEF("fopen", 2, jsFOpen),
    JS_CFUNC_DEF("fclose", 1, jsFClose),
    JS_CFUNC_DEF("fread", 4, jsFRead),
    JS_CFUNC_DEF("fwrite", 4, jsFWrite),
    JS_CFUNC_DEF("ftell", 4, jsFTell),
    JS_CFUNC_DEF("fseek", 4, jsFSeek),
    JS_PROP_INT32_DEF("SEEK_SET", SEEK_SET, JS_PROP_CONFIGURABLE ),
    JS_PROP_INT32_DEF("SEEK_CUR", SEEK_CUR, JS_PROP_CONFIGURABLE ),
    JS_PROP_INT32_DEF("SEEK_END", SEEK_END, JS_PROP_CONFIGURABLE ),
};

static int jsStdModuleInitFunc(JSContext *ctx, JSModuleDef *m)
{
    JS_SetModuleExportList(ctx, m, sJsStdFuncs, countof(sJsStdFuncs));

    JSValue stdErrFile = JS_NewObjectClass(ctx, sFileClassId);
    JS_SetOpaque(stdErrFile, stderr);
    JS_SetModuleExport(ctx, m, "stderr", stdErrFile);

    JSValue stdInFile = JS_NewObjectClass(ctx, sFileClassId);
    JS_SetOpaque(stdInFile, stdin);
    JS_SetModuleExport(ctx, m, "stdin", stdInFile);

    JSValue stdOutFile = JS_NewObjectClass(ctx, sFileClassId);
    JS_SetOpaque(stdOutFile, stdout);
    JS_SetModuleExport(ctx, m, "stdout", stdOutFile);
    return 0;
}

void jsInitStdModule(JSContext *ctx)
{
    JS_NewClassID(&sFileClassId);
    JS_NewClass(JS_GetRuntime(ctx), sFileClassId, &sFileClassDef);

    JSModuleDef *module = JS_NewCModule(ctx, "std", jsStdModuleInitFunc);
    JS_AddModuleExportList(ctx, module, sJsStdFuncs, countof(sJsStdFuncs));
    JS_AddModuleExport(ctx, module, "stderr");
    JS_AddModuleExport(ctx, module, "stdin");
    JS_AddModuleExport(ctx, module, "stdout");

    const char *input = "import * as std from 'std';\nglobalThis.std = std;\n";
    JSValue val = JS_Eval(ctx, input, strlen(input), "<input>", JS_EVAL_TYPE_MODULE);
    if (JS_IsException(val))
    {
        jsDumpError(ctx);
    }
    JS_FreeValue(ctx, val);
}