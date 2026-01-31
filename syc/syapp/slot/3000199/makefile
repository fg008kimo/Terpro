BIN=mammon_3000199_server

mammon_3000199_server : algMammon.o Alg2.o net.o server.o utils.o data.o tools/cJSON.o tools/random/rng.o logic/board.o logic/freegame.o Test/TestMain.o Test/TestRandom.o
	gcc $^ -o $@ -L$(HOME)/zlog/src -lzlog -lm -lpthread
	
.c.o:
	gcc -D${PLATFORM} -O2 -c $< -o $@ -L$(HOME)/zlog/src -lzlog -lm -lpthread -I$(HOME)/zlog/src	

#关掉宏__LINUX__：可跨平台编译、可跑主逻辑（备注：linux平台要加-lm -lpthread）
# mammon_3000199_server : algMammon.o Alg2.o net.o server.o utils.o data.o tools/cJSON.o tools/random/rng.o logic/board.o logic/freegame.o Test/TestMain.o Test/TestRandom.o
# 	gcc $^ -o $@
	
# .c.o:
# 	gcc -c $< -o $@

clean	:
	rm -f *.o $(BIN) tools/*.o tools/random/*.o logic/*.o Test/*.o *.conf

install:
	mkdir -p $(HOME)/bin/${SUBPATH}/
	rm -f $(HOME)/bin/${SUBPATH}/$(BIN)
	cp $(BIN) $(HOME)/bin/${SUBPATH}/

.PHONY : clean all

