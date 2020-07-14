#ifndef QJSNET_H
#define QJSNET_H

#include <stdlib.h>
#include <stdio.h>
#include <quickjs/quickjs.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define countof(x) (sizeof(x) / sizeof((x)[0]))

const char* qjs_server = "Server";

static JSValue qjsnet_bind(JSContext *ctx, JSValueConst this_val, int argc,
								JSValueConst *argv);
static JSValue qjsnet_socket(JSContext *ctx, JSValueConst this_val, int argc,
								JSValueConst *argv);

static const JSCFunctionListEntry qjsnet_funcs[] = {
	JS_CFUNC_DEF("bind", 1, qjsnet_bind), 
	JS_CFUNC_DEF("socket", 0, qjsnet_socket),
	JS_PROP_INT32_DEF("AF_INET", AF_INET, JS_PROP_NORMAL ),
	JS_PROP_INT32_DEF("AF_INET6", AF_INET6, JS_PROP_NORMAL ),
};

#endif /* QJSNET_H */