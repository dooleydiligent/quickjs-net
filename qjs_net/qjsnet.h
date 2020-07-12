#ifndef QJSNET_H
#define QJSNET_H

#include <quickjs/quickjs.h>

#define countof(x) (sizeof(x) / sizeof((x)[0]))

static JSValue qjsnet_bind(JSContext *ctx, JSValueConst this_val, int argc,
								JSValueConst *argv);
static JSValue qjsnet_socket(JSContext *ctx, JSValueConst this_val, int argc,
								JSValueConst *argv);
static JSValue plusNumbers(JSContext *ctx, JSValueConst this_val, int argc,
								JSValueConst *argv);


static const JSCFunctionListEntry js_qjsnet_funcs[] = {
    JS_CFUNC_DEF("plus", 2, plusNumbers),
		JS_CFUNC_DEF("bind", 1, qjsnet_bind), 
		JS_CFUNC_DEF("socket", 0, qjsnet_socket),
};

#endif /* QJSNET_H */