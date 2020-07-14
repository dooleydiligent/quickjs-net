#include "qjsnet.h"
#include <stdlib.h>
#include <stdio.h>
#include <quickjs/quickjs.h>
#include <netinet/in.h>
#include <sys/socket.h>
/**
 * Given a port and maybe an ip, and maybe a socket, bind to a socket.
 * 
 * If port is not passed then default to 7981 as an homage to https://github.com/khanhas/minnet-quickjs 
 * If socket is not passed then create one
 * If ip is not passed then use "127.0.0.1"
 * On success we return the socket
 */
static JSValue qjsnet_bind(JSContext *ctx, JSValueConst this_val, int argc,
													 JSValueConst *argv)
{
	struct sockaddr_in server;
	int port = 7981;
	int socketId = 0;
	const char *host;

	JSValue options = argv[0];
	JSValue opt_socket = JS_GetPropertyStr(ctx, options, "socket");
	JSValue opt_port = JS_GetPropertyStr(ctx, options, "port");
	JSValue opt_host = JS_GetPropertyStr(ctx, options, "host");

	if (JS_IsNumber(opt_port))
		JS_ToInt32(ctx, &port, opt_port);

	if (JS_IsString(opt_host))
		host = JS_ToCString(ctx, opt_host);
	else
		host = "127.0.0.1";

	if (JS_IsNumber(opt_socket))
		JS_ToInt32(ctx, &socketId, opt_socket);

	if (socketId < 1)
		socketId = socket(AF_INET, SOCK_STREAM, 0);

	if (socketId < 1)
	{
		fprintf(stderr, "Could not create socket\n");
		return JS_EXCEPTION;
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	if (bind(socketId, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		fprintf(stderr, "Could not bind to socket on %s:%d", host, port);
		return JS_EXCEPTION;
	}
	return JS_NewInt32(ctx, socketId);
}
// https://www.binarytides.com/socket-programming-c-linux-tutorial/
/**
 * Returns the socket that was created or -1 on error.
 * 
 * // TODO: Register the socket and close it on exit?
 * 
 */
static JSValue qjsnet_socket(JSContext *ctx, JSValueConst this_val, int argc,
														 JSValueConst *argv)
{
	return JS_NewInt32(ctx, socket(AF_INET, SOCK_STREAM, 0));
}

static JSValue plusNumbers(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
	int a, b;

	if (JS_ToInt32(ctx, &a, argv[0]))
		return JS_EXCEPTION;

	if (JS_ToInt32(ctx, &b, argv[1]))
		return JS_EXCEPTION;

	return JS_NewInt32(ctx, a + b);
}
/* Server class */
typedef struct
{
	int x;
	int y;
} JSServerData;

static JSClassID qjsnet_server_class_id;

static void qjsnet_server_finalizer(JSRuntime *rt, JSValue val)
{
	JSServerData *s = JS_GetOpaque(val, qjsnet_server_class_id);
	/* Note: 's' can be NULL in case JS_SetOpaque() was not called */
	js_free_rt(rt, s);
}

static JSValue qjsnet_server_ctor(JSContext *ctx,
																	JSValueConst new_target,
																	int argc, JSValueConst *argv)
{
	JSServerData *s;
	JSValue obj = JS_UNDEFINED;
	JSValue proto;

	s = js_mallocz(ctx, sizeof(*s));
	if (!s)
		return JS_EXCEPTION;
	if (JS_ToInt32(ctx, &s->x, argv[0]))
		goto fail;
	if (JS_ToInt32(ctx, &s->y, argv[1]))
		goto fail;
	/* using new_target to get the prototype is necessary when the
       class is extended. */
	proto = JS_GetPropertyStr(ctx, new_target, "prototype");
	if (JS_IsException(proto))
		goto fail;
	obj = JS_NewObjectProtoClass(ctx, proto, qjsnet_server_class_id);
	JS_FreeValue(ctx, proto);
	if (JS_IsException(obj))
		goto fail;
	JS_SetOpaque(obj, s);
	return obj;
fail:
	js_free(ctx, s);
	JS_FreeValue(ctx, obj);
	return JS_EXCEPTION;
}

static JSValue qjsnet_server_get_xy(JSContext *ctx, JSValueConst this_val, int magic)
{
	JSServerData *s = JS_GetOpaque2(ctx, this_val, qjsnet_server_class_id);
	if (!s)
		return JS_EXCEPTION;
	if (magic == 0)
		return JS_NewInt32(ctx, s->x);
	else
		return JS_NewInt32(ctx, s->y);
}

static JSValue qjsnet_server_set_xy(JSContext *ctx, JSValueConst this_val, JSValue val, int magic)
{
	JSServerData *s = JS_GetOpaque2(ctx, this_val, qjsnet_server_class_id);
	int v;
	if (!s)
		return JS_EXCEPTION;
	if (JS_ToInt32(ctx, &v, val))
		return JS_EXCEPTION;
	if (magic == 0)
		s->x = v;
	else
		s->y = v;
	return JS_UNDEFINED;
}

static JSValue qjsnet_server_norm(JSContext *ctx, JSValueConst this_val,
																	int argc, JSValueConst *argv)
{
	JSServerData *s = JS_GetOpaque2(ctx, this_val, qjsnet_server_class_id);
	if (!s)
		return JS_EXCEPTION;
	return JS_NewFloat64(ctx, sqrt((double)s->x * s->x + (double)s->y * s->y));
}

static JSClassDef qjsnet_server_class = {
		"Server",
		.finalizer = qjsnet_server_finalizer,
};

static const JSCFunctionListEntry qjsnet_server_proto_funcs[] = {
		JS_CGETSET_MAGIC_DEF("x", qjsnet_server_get_xy, qjsnet_server_set_xy, 0),
		JS_CGETSET_MAGIC_DEF("y", qjsnet_server_get_xy, qjsnet_server_set_xy, 1),
		JS_CFUNC_DEF("norm", 0, qjsnet_server_norm),
};

static int js_qjsnet_init(JSContext *ctx, JSModuleDef *m)
{
	// Server initialization goes here
	JSValue server_proto, server_class;

	/* create the Server class */
	JS_NewClassID(&qjsnet_server_class_id);
	JS_NewClass(JS_GetRuntime(ctx), qjsnet_server_class_id, &qjsnet_server_class);

	server_proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, server_proto, qjsnet_server_proto_funcs, countof(qjsnet_server_proto_funcs));

	server_class = JS_NewCFunction2(ctx, qjsnet_server_ctor, qjs_server, 2, JS_CFUNC_constructor, 0);
	/* set proto.constructor and ctor.prototype */
	JS_SetConstructor(ctx, server_class, server_proto);
	JS_SetClassProto(ctx, qjsnet_server_class_id, server_proto);

	JS_SetModuleExport(ctx, m, qjs_server, server_class);

	return JS_SetModuleExportList(ctx, m, js_qjsnet_funcs, countof(js_qjsnet_funcs));
}

JSModuleDef *js_init_module(JSContext *ctx, const char *module_name)
{
	JSModuleDef *m;
	m = JS_NewCModule(ctx, module_name, js_qjsnet_init);

	if (!m)
		return NULL;

	JS_AddModuleExportList(ctx, m, js_qjsnet_funcs, countof(js_qjsnet_funcs));

	JS_AddModuleExport(ctx, m, qjs_server);
	return m;
}