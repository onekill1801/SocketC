client: client.c
ifeq ($(OS), Windows_NT)
	gcc client.c -oclient.exe -lws2_32
else
	gcc client.c -oclient -pthread
endif
.PHONY: clean
clean:
	rm -f client client.exe
