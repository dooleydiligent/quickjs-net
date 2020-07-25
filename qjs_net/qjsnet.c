#include "qjsnet.h"

int isdebug = -1;

int qjs_debug_log(JSContext *ctx, const char *format, ...)
{
	va_list argp;
	char buf[1024];
	char numbuf[33];
	int len = 0;
	const char *s;
	if (isdebug == -1)
	{
		const char *debug = getenv("QJSNET_DEBUG");
		if (debug)
		{
			if (strcasecmp(debug, "true") == 0)
			{
				isdebug = 1;
			}
		}
	}
	if (isdebug == 1)
	{
		memset(buf, '\0', sizeof(buf));
		va_start(argp, format);
		while (*format != '\0' && len < 1024)
		{
			if (*format == '%')
			{
				format++;
				switch (*format)
				{
				case '%':
					buf[len++] = '%';
					break;
				case 'c':
					buf[len++] = va_arg(argp, int);
					break;
				case 'd':
					sprintf(numbuf, "%d", va_arg(argp, int));
					char *ptr = numbuf;
					while (*ptr != '\0' && len < 1024)
					{
						buf[len++] = *ptr++;
					}
					break;
				case 's':
					s = va_arg(argp, const char *);
					while (*s != '\0' && len < 1024)
					{
						buf[len++] = *s++;
					}
					break;
				default:
					buf[len++] = *format;
				}
			}
			else
			{
				buf[len++] = *format;
			}
			format++;
		}
		va_end(argp);
		fprintf(stderr, "QJS_NET - [DEBUG] - %s\n", buf);
	}
	return len;
}

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
	const char *address;
	JSValue options = argv[0];
	JSValue opt_socket = JS_GetPropertyStr(ctx, options, "socket");
	JSValue opt_port = JS_GetPropertyStr(ctx, options, "port");
	JSValue opt_address = JS_GetPropertyStr(ctx, options, "address");

	if (JS_IsNumber(opt_port))
		JS_ToInt32(ctx, &port, opt_port);

	if (JS_IsString(opt_address))
		address = JS_ToCString(ctx, opt_address);
	else
		address = "127.0.0.1";

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
		fprintf(stderr, "Could not bind to socket on %s:%d", address, port);
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

/* The Server class */

static JSClassID qjsnet_server_class_id;

/**
 * address address can "localhost" or an IPv4 address, incl 0.0.0.0 for all interfaces
 * Here we just validate that an IP address was passed in and fail if not
 * 1 is success
 * 0 or -1 is failure 
 */
static int qjsnet_get_address_or_ip(JSContext *ctx, JSServerData *s, JSValue arg)
{
	struct sockaddr_in sa;
	char *address = JS_ToCString(ctx, arg);
	qjs_debug_log(ctx, "qjsnet_get_address(%s)", address);
	// fprintf(stderr,"Input BEFORE: %s\n", s->address);
	if (strcasecmp(address, "localhost") == 0)
	{
		strcpy(s->address, "127.0.0.1");
	} else {
		strcpy(s->address, address);
	}
	JS_FreeCString(ctx, address);
	// Try to convert it to an actual ipv4 address
	const int result = inet_pton(AF_INET, s->address, &(sa.sin_addr));
	// fprintf(stderr,"Input AFTER %s\n", s->address);

	return result;
}

/**
 * new Server(port?, address?)
 * 
 * All options are optional.
 * port defaults to 7981
 * We only allow IPV4 address atm
 * address defaults to "127.0.0.1" but we will allow "LoCaLhOsT"
 */
static JSValue qjsnet_server_ctor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv)
{
	JSServerData *s;
	JSValue proto, obj;
	const char *exception = NULL;

	s = js_mallocz(ctx, sizeof(JSServerData));
	if (!s)
		return JS_EXCEPTION;
	obj = JS_UNDEFINED;

	// Set defaults which can be overridden below
	s->port = 7981;
	strcpy(s->address, "127.0.0.1");
	s->socket = 0;
	init_list_head(&s->event_list);
	/* using new_target to get the prototype is necessary when the class is extended. */
	proto = JS_GetPropertyStr(ctx, new_target, "prototype");
	if (JS_IsException(proto))
		goto fail;
	obj = JS_NewObjectProtoClass(ctx, proto, qjsnet_server_class_id);
	JS_FreeValue(ctx, proto);
	if (JS_IsException(obj))
		goto fail;

	exception = "Expected port to be an integer [0-65535]";

	if (argc > 0)
	{
		if (JS_ToInt32(ctx, &s->port, argv[0]))
			goto fail;
		qjs_debug_log(ctx, "Port set to %d", s->port);
		exception = "Invalid host address";
		if (argc > 1)
		{
			JS_FreeCString(ctx, s->address);
			if (qjsnet_get_address_or_ip(ctx, s, argv[1]) != 1)
				goto fail;
			qjs_debug_log(ctx, "qjsnet_server_ctor(%d, %s)", s->port, s->address);
		}
		else
		{
			qjs_debug_log(ctx, "qjsnet_server_ctor(%d)", s->port);
		}
	}
	else
	{
		qjs_debug_log(ctx, "qjsnet_server_ctor()");
	}
	JS_SetOpaque(obj, s);
	s->ctx = JS_DupContext(ctx);
	return obj;
fail:
	js_free(ctx, s);
	JS_FreeValue(ctx, obj);
	JS_ThrowTypeError(ctx, "Exception instantiating Server: %s", exception);
	return JS_EXCEPTION;
}

static void qjsnet_server_finalizer(JSRuntime *rt, JSValue val)
{
	JSServerData *s = JS_GetOpaque(val, qjsnet_server_class_id);

	/* Note: 's' can be NULL in case JS_SetOpaque() was not called */
	// Release the event list now
	struct list_head *el;
	qjsnet_event_callback *event;

	while (!list_empty(&s->event_list))
	{
		el = s->event_list.next;
		event = list_entry(el, qjsnet_event_callback, link);
		qjs_debug_log(s->ctx, "Deleting the event from the list");
		/* remove the callbac from the event list */
		list_del(&event->link);

		qjs_debug_log(s->ctx, "Releasing callback code for event %s", event->event_name);
		JS_FreeValue(s->ctx, event->func_obj);

		qjs_debug_log(s->ctx, "Releasing callback %s", event->event_name);
		JS_FreeCString(s->ctx, event->event_name);
		qjs_debug_log(s->ctx, "Releasing class reference");
		JS_FreeValue(s->ctx, event->this_obj);
	}

	qjs_debug_log(s->ctx, "All memory released");
	JS_FreeContext(s->ctx);

	js_free_rt(rt, s);
}

static JSValue qjsnet_server_get_prop(JSContext *ctx, JSValueConst this_val, int magic)
{
	JSServerData *s = JS_GetOpaque2(ctx, this_val, qjsnet_server_class_id);

	if (!s)
		return JS_EXCEPTION;

	switch (magic)
	{
	case 0:
		qjs_debug_log(ctx, "qjsnet_server_get_prop(PORT=>%d)", s->port);
		return JS_NewInt32(ctx, s->port);
		break;
	case 1:
		qjs_debug_log(ctx, "qjsnet_server_get_prop(HOST=%s)", s->address);
		return JS_NewString(ctx, s->address);
		break;
	case 2:
		qjs_debug_log(ctx, "qjsnet_server_get_prop(SOCKET=%d)", s->socket);
		return JS_NewInt32(ctx, s->socket);
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
		if (s->address) {
			JS_FreeCString(ctx, s->address);
		}
		if (qjsnet_get_address_or_ip(ctx, s, val) != 1)
			return JS_EXCEPTION;
		break;
	case 2:
	  fprintf(stderr, "Socket is a read-only property");
	  return JS_EXCEPTION;
		break;
	default:
		return JS_EXCEPTION;
	}
	return JS_UNDEFINED;
}
/**
 * Stop listening, close the socket and unref the count
 */
static JSValue qjsnet_server_close(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
	qjs_debug_log(ctx, "qjsnet_server_close()");
	JSServerData *s = JS_GetOpaque2(ctx, this_val, qjsnet_server_class_id);
	if (!s)
		return JS_EXCEPTION;
	if (s->socket)
	{
		qjs_debug_log(ctx, "closing socket %d", s->socket);
		close(s->socket);
	}
	s->socket = 0;
	return JS_UNDEFINED;
}
static JSValue qjsnet_raise_event(JSContext *ctx,
																	int argc, JSValueConst *argv)
{
	// Find and invoke all events with the provided name
	// The first argv is the event name
	// The last argv is the "this_val" passed in qjsnet_server_on_event
	// TODO: process argv between first and last as arguments to the callback

	qjs_debug_log(ctx, "qjsnet_raise_event");

	struct list_head *el;
	qjsnet_event_callback *event;
	const char *event_name;
	JSServerData *s = JS_GetOpaque2(ctx, argv[argc - 1], qjsnet_server_class_id);
	if (!s)
	{
		fprintf(stderr, "Exception: Cannot find server data in qjsnet_raise_event");
		return JS_EXCEPTION;
	}
	event_name = JS_ToCString(ctx, argv[0]);
	list_for_each(el, &s->event_list)
	{
		event = list_entry(el, qjsnet_event_callback, link);
		if (strcasecmp(event->event_name, event_name) == 0)
		{
			// Invoke the callback
			qjs_debug_log(ctx, "Invoking the callback: %s", event->event_name);
			JS_Call(ctx, event->func_obj, argv[argc - 1], argc, argv);
		}
	}
	qjs_debug_log(ctx, "freeing event_name: %s", event_name);
	JS_FreeCString(ctx, event_name);
	return JS_UNDEFINED;
}
static JSValue qjsnet_server_on_event(JSContext *ctx, JSValueConst this_val,
																			int argc, JSValueConst *argv)
{
	qjsnet_event_callback *e;
	JSServerData *s = JS_GetOpaque2(ctx, this_val, qjsnet_server_class_id);
	if (!s)
	{
		fprintf(stderr, "qjsnet_server_on_event: Could not find server data");
		goto fail;
	}
	if (argc > 0)
	{
		if (argc > 1 && JS_IsFunction(ctx, argv[1]))
		{
			e = js_malloc(ctx, sizeof(*e) + argc * sizeof(JSValue));
			if (!e)
			{
				fprintf(stderr, "Could not allocate memory for new callback");
				goto fail;
			}
			// TODO : JS_FreeCString(ctx, event_name) in finalizer?
			e->event_name = JS_ToCString(ctx, argv[0]);
			qjs_debug_log(ctx, "setting callback %s", e->event_name);
			e->ctx = ctx;
			e->func_obj = JS_DupValue(ctx, argv[1]);
			e->this_obj = JS_DupValue(ctx, this_val);
			// Add this listener to the list of listeners
			list_add_tail(&e->link, &s->event_list);
			qjs_debug_log(ctx, "callback %s is ready", e->event_name);
		}
		else
		{
			fprintf(stderr, "event callback is required");
		}
	}
	else
	{
		fprintf(stderr, "event name is required");
		goto fail;
	}
	return JS_DupValue(ctx, this_val); // Return the server
fail:
	return JS_EXCEPTION;
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
	JSValueConst args[2];
	JSServerData *s = JS_GetOpaque2(ctx, this_val, qjsnet_server_class_id);
	if (!s)
		return JS_EXCEPTION;

	if (argc > 0)
	{
		if (JS_ToInt32(ctx, &s->port, *argv))
		{
			sprintf((char *)exception, "Expected port to be an integer [0-65535] but found %s", JS_ToCString(ctx, argv[0]));
			goto fail;
		}

		if (argc > 1)
		{
			if (qjsnet_get_address_or_ip(ctx, s, argv[1]) != 1)
			{
				sprintf((char *)exception, "Could not interpret host address %s", JS_ToCString(ctx, argv[1]));
				goto fail;
			}
			qjs_debug_log(ctx, "qjsnet_server_listene(%d, %s)", s->port, s->address);
		}
		else
		{
			qjs_debug_log(ctx, "qjsnet_server_listen(%d)", s->port);
		}
	}
	else
	{
		qjs_debug_log(ctx, "qjsnet_server_listen()");
	}
	struct sockaddr_in server;

	//Create socket
	s->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (s->socket == -1)
	{
		sprintf((char *)exception, "Could not create socket: %d", s->socket);
		goto fail;
	}
	qjs_debug_log(ctx, "qjsnet_server_listen(%d, %s) - created socket %d", s->port, s->address, s->socket);
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_port = htons(s->port);
	// INADDR_LOOPBACK
	int result = inet_pton(server.sin_family, s->address, &(server.sin_addr));
	if (result == 0)
	{
		sprintf((char *)exception, "Not an ipv4 address: %s", s->address);
		goto fail;
	}
	if (result == -1)
	{
		sprintf((char *)exception, "Not a valid address: %s\n", s->address);
	}
	if (result != 1)
	{
		goto fail;
	}

	if (bind(s->socket, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		sprintf((char *)exception, "Could not bind to socket on %s:%d", s->address, s->port);
		goto fail;
	}
	// listen now
	listen(s->socket, max_connections);
	qjs_debug_log(ctx, "qjsnet_server_listen(%d,%s) - listening", s->port, s->address);
	// Raise the listening event
	args[0] = JS_NewString(ctx, "listening");
	args[1] = JS_DupValue(ctx, this_val);
	JS_EnqueueJob(ctx, qjsnet_raise_event, 2, args);
	JS_FreeValue(ctx, args[0]);
	JS_FreeValue(ctx, args[1]);
	js_std_loop(ctx);

	return JS_DupValue(ctx, this_val); // Return the server
fail:
	s->socket = 0;
	JS_ThrowTypeError(ctx, "Exception instantiating Server: %s", exception);
	return JS_EXCEPTION;
}

static JSClassDef qjsnet_server_class = {
		"Server",
		.finalizer = qjsnet_server_finalizer,
};

static const JSCFunctionListEntry qjsnet_server_proto_funcs[] = {
		JS_CFUNC_DEF("close", 0, qjsnet_server_close),
		JS_CGETSET_MAGIC_DEF("port", qjsnet_server_get_prop, qjsnet_server_set_prop, 0),
		JS_CGETSET_MAGIC_DEF("address", qjsnet_server_get_prop, qjsnet_server_set_prop, 1),
		JS_CGETSET_MAGIC_DEF("socket", qjsnet_server_get_prop, qjsnet_server_set_prop, 2),
		JS_CFUNC_DEF("listen", 2, qjsnet_server_listen),
		JS_CFUNC_DEF("on", 2, qjsnet_server_on_event),
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
