BIN=random_number_generator

CXXFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS) -std=c++11
NVCCFLAGS += -D_FORCE_INLINES -ccbin=$(CXX) -Xcompiler -fPIC $(COMMON_FLAGS) -std=c++11
LINKFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS) -std=c++11

random_number_generator : main.o utils.o data.o rng.o tools/cJSON.o logic/mt.o logic/rand.o test/testMain.o
	gcc -std=c++11 $^ -o $@ -L$(HOME)/zlog/src -lzlog -lstdc++ -lm -lpthread

.c.o:
	gcc -D${PLATFORM} -O2 -c $< -o $@ -L$(HOME)/zlog/src -lzlog -lm -lpthread -I$(HOME)/zlog/src

#关掉宏__LINUX__：可跨平台编译、可跑主逻辑（备注：linux平台要加-lm -lpthread）
# random_number_generator : main.o utils.o data.o rng.o tools/cJSON.o logic/mt.o logic/rand.o test/testMain.o
# 	gcc $^ -o $@ -lstdc++
	
# .c.o:
# 	gcc -c $< -o $@

clean	:
	rm -f $(BIN) *.o *.conf logic/*.o test/*.o tools/*.o

install:
	mkdir -p $(HOME)/bin/${SUBPATH}/
	rm -f $(HOME)/bin/${SUBPATH}/$(BIN)
	cp $(BIN) $(HOME)/bin/${SUBPATH}/