#https://gist.github.com/xuhdev/1873316
OBJS :=  input/input.o crypto/crypto.o socket/socket.o peer/peer.o relay/relay.o dfs/dfs.o engine/engine.o id/id.o auth/auth.o cash/cash.o
#CXXFLAGS := -g -O0 -DDEBUG
CXXFLAGS = -fPIC -O0 -g -std=c++17 -DDEBUG

.PHONY: all
all: libusgov.so

signal_handler.o: signal_handler.cpp signal_handler.h
	g++ ${CXXFLAGS} -I../.. -c signal_handler.cpp

libusgov.so: deps signal_handler.o
#	g++ -shared -fPIC ${CXXFLAGS} -pthread ${OBJS} signal_handler.o cfg.o -lcrypto++ -lsecp256k1 -L/usr/lib -I../.. -o libusgov.so
	g++ -shared ${CXXFLAGS} -pthread ${OBJS} signal_handler.o -lcryptopp -lsecp256k1 -L/usr/lib -I../.. -o libusgov.so

deps:
	@for a in $$(ls); do \
	if [ -d $$a ]; then \
	echo "processing folder $$a"; \
        echo "deleting $$a/$$a.o"; \
        rm -f $$a/$$a.o; \
	$(MAKE) -C $$a; \
	fi; \
	done;

.PHONY: deps

clean:
	find . -name "*.o" -delete
	rm -f libusgov.so

