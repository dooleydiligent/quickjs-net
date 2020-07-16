#include "qjsnet.h"

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

static JSClassID qjsnet_server_class_id;

static void qjsnet_server_finalizer(JSRuntime *rt, JSValue val)
{
	JSServerData *s = JS_GetOpaque(val, qjsnet_server_class_id);
	/* Note: 's' can be NULL in case JS_SetOpaque() was not called */
	js_free_rt(rt, s);
}
/**
 * host address can "localhost" or an IPv4 address, incl 0.0.0.0 for all interfaces
 * Here we just validate that an IP address was passed in and fail if not
 * 1 is success
 * 0 or -1 is failure
 */
static int qjsnet_get_host_or_ip(JSContext *ctx, JSServerData *s, JSValue arg)
{
	struct sockaddr_in sa;
	char str[INET_ADDRSTRLEN];
	char *address = JS_ToCString(ctx, arg);
	if (stricmp(address, "localhost") == 0)
	{
		address = "127.0.0.1";
	}
	// Try to convert it to an actual ipv4 address
	return inet_pton(AF_INET, address, &(sa.sin_addr));
}

/**
 * new Server(port?, address?)
 * 
 * All options are optional.
 * port defaults to 7981
 * We only allow IPV4 address atm
 * address defaults to "0.0.0.0" but we will allow "LoCaLhOsT"
 */
static JSValue qjsnet_server_ctor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv)
{
	JSServerData *s;
	JSValue obj = JS_UNDEFINED;
	JSValue proto;
	const char *exception = NULL;

	s = js_mallocz(ctx, sizeof(*s));
	if (!s)
		return JS_EXCEPTION;

	// Set defaults which can be overridden below
	s->port = 7981;
	s->host = "0.0.0.0";

	/* using new_target to get the prototype is necessary when the
       class is extended. */
	proto = JS_GetPropertyStr(ctx, new_target, "prototype");
	if (JS_IsException(proto))
		goto fail;
	obj = JS_NewObjectProtoClass(ctx, proto, qjsnet_server_class_id);
	JS_FreeValue(ctx, proto);
	if (JS_IsException(obj))
		goto fail;

	exception = "Expected port to be an integer [0-65535]";

	if (argc > 0)
		if (JS_ToInt32(ctx, &s->port, argv[0]))
			goto fail;

	exception = "Invalid host address";
	if (argc > 1)
		if (qjsnet_get_host_or_ip(ctx, s, argv[1]) != 1)
			goto fail;

	JS_SetOpaque(obj, s);
	return obj;
fail:
	js_free(ctx, s);
	JS_FreeValue(ctx, obj);
	JS_ThrowTypeError(ctx, "Exception instantiating Server: %s", exception);
	return JS_EXCEPTION;
}

static JSValue qjsnet_server_get_prop(JSContext *ctx, JSValueConst this_val, int magic)
{
	JSServerData *s = JS_GetOpaque2(ctx, this_val, qjsnet_server_class_id);
	if (!s)
		return JS_EXCEPTION;
	switch (magic)
	{
	case 0:
		return JS_NewInt32(ctx, s->port);
		break;
	case 1:
		return JS_NewString(ctx, s->host);
		break;
	default:
		return JS_EXCEPTION;
	}
}

static JSValue qjsnet_server_set_prop(JSContext *ctx, JSValueConst this_val, JSValue val, int magic)
{
	JSServerData *s = JS_GetOpaque2(ctx, this_val, qjsnet_server_class_id);
	int v;
	if (!s)
		return JS_EXCEPTION;
	switch (magic)
	{
	case 0:
		if (JS_ToInt32(ctx, &v, val))
			return JS_EXCEPTION;
		s->port = v;
		break;
	case 1:
		if (qjsnet_get_host_or_ip(ctx, s, val) != 1)
			return JS_EXCEPTION;
		break;
	default:
		return JS_EXCEPTION;
	}
	return JS_UNDEFINED;
}
/**
 * Given an optional port and ip address/hostname, create a socket and listen for incoming connections
 * 
 * Should raise a 'listening' event
 */
static JSValue qjsnet_server_listen(JSContext *ctx, JSValueConst this_val,
																		int argc, JSValueConst *argv)
{
	const char exception[256];
	const int max_connections = 5;
	JSServerData *s = JS_GetOpaque2(ctx, this_val, qjsnet_server_class_id);
	if (!s)
		return JS_EXCEPTION;

	if (argc > 0)
		if (JS_ToInt32(ctx, &s->port, argv[0]))
		{
			sprintf(exception, "Expected port to be an integer [0-65535] but found %s", argv[0]);
			goto fail;
		}

	if (argc > 1)
	{
		if (qjsnet_get_host_or_ip(ctx, s, argv[1]) != 1)
		{
			sprintf(exception, "Could not interpret host address %s", argv[1]);
			goto fail;
		}
	}
	int socket_desc;
	struct sockaddr_in server;

	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		sprintf(exception, "Could not create socket: %d", socket_desc);
		goto fail;
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, s->host, &(server.sin_addr)) == -1)
	{
		sprintf(exception, "Could not interpret the provided host: %s", s->host);
		goto fail;
	}
	server.sin_port = htons(s->port);
	if (bind(s->socket, (struct sockaddr *)&s, sizeof(s)) < 0)
	{
		sprintf(exception, "Could not bind to socket on %s:%d", s->host, s->port);
		goto fail;
	}
	// listen now
	listen(s->socket, max_connections);
	return JS_NewInt32(ctx, 0);
fail:
	JS_ThrowTypeError(ctx, "Exception instantiating Server: %s", exception);
	return JS_EXCEPTION;
}

static JSClassDef qjsnet_server_class = {
		"Server",
		.finalizer = qjsnet_server_finalizer,
};

static const JSCFunctionListEntry qjsnet_server_proto_funcs[] = {
		JS_CGETSET_MAGIC_DEF("port", qjsnet_server_get_prop, qjsnet_server_set_prop, 0),
		JS_CGETSET_MAGIC_DEF("type", qjsnet_server_get_prop, qjsnet_server_set_prop, 1),
		JS_CGETSET_MAGIC_DEF("address", qjsnet_server_get_prop, qjsnet_server_set_prop, 2),
		JS_CFUNC_DEF("listen", 2, qjsnet_server_listen),
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

	return JS_SetModuleExportList(ctx, m, qjsnet_funcs, countof(qjsnet_funcs));
}

JSModuleDef *js_init_module(JSContext *ctx, const char *module_name)
{
	JSModuleDef *m;
	m = JS_NewCModule(ctx, module_name, js_qjsnet_init);

	if (!m)
		return NULL;

	JS_AddModuleExportList(ctx, m, qjsnet_funcs, countof(qjsnet_funcs));

	JS_AddModuleExport(ctx, m, qjs_server);
	return m;
}