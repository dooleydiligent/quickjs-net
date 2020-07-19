CC=clang
FLAGS=-Wall -fPIC -shared -std=gnu17
LIB=-L/usr/lib/quickjs/ -lquickjs -lpthread
DEFINE=-DJS_SHARED_LIBRARY
all:
	${CC} ${FLAGS} ${DEFINE} ${LIB} qjsnet.c -o libqjsnet.so -Wl,-rpath,"/usr/local/lib"  