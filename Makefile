

CJSON=1.7.6

CJSON_LIB=third-party/cJSON-${CJSON}/libcjson.so.${CJSON}




SRC_FILES=ical_schoolcal.cpp calendar_entry.cpp

CFLAGS+=-Wall -g -Ithird-party/cJSON-${CJSON}
CPPFLAGS+= ${CFLAGS} -std=c++11
LDFLAGS+=-lical -Lthird-party/cJSON-${CJSON}/ -lcjson -lcurl


OBJ_FILES=$(patsubst %.cpp,obj/%.o,$(SRC_FILES))

ical_schoolcal: ${CJSON_LIB} ${OBJ_FILES}
	$(CXX) $(LDFLAGS) -o $@ $^


obj/%.o : %.cpp
	@mkdir -p obj
	$(CXX) $(CPPFLAGS) -c $< -o $@

obj/%.o : %.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@


${CJSON_LIB}:
	cd third-party ; wget https://github.com/DaveGamble/cJSON/archive/v${CJSON}.tar.gz
	cd third-party ; gzip -cd v${CJSON}.tar.gz | tar xfv -
	cd third-party/cJSON-${CJSON} ; make
 
 
clean:
	rm -fr obj
	rm -f ical_schoolcal
	
	
.PHONY: clean

