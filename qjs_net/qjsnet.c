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
  	socketId = socket(AF_INET, SOCK_STREAM , 0);

	if (socketId < 1)
	{
		fprintf(stderr, "Could not create socket\n");
		return JS_EXCEPTION;
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );
	if( bind(socketId,(struct sockaddr *)&server , sizeof(server)) < 0)
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
	return JS_NewInt32(ctx, socket(AF_INET, SOCK_STREAM , 0));
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

static int js_qjsnet_init(JSContext *ctx, JSModuleDef *m)
{
    return JS_SetModuleExportList(ctx, m, js_qjsnet_funcs, countof(js_qjsnet_funcs));
}

JSModuleDef *js_init_module(JSContext *ctx, const char *module_name)
{
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_qjsnet_init);
    
    if (!m)
        return NULL;
    
    JS_AddModuleExportList(ctx, m, js_qjsnet_funcs, countof(js_qjsnet_funcs));
    return m;
}