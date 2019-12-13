server: server.c
ifeq ($(OS), Windows_NT)
	gcc server.c -oserver.exe -lws2_32
else
	gcc server.c -oserver -pthread
endif
.PHONY: clean
clean:
	rm -f server server.exe
