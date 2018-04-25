

SRC_FILES=ical_schoolcal.cpp calendar_entry.cpp



CFLAGS+=-Wall -g -Ithird-party/cJSON-1.7.6
CPPFLAGS+= ${CFLAGS} -std=c++11
LDFLAGS+=-lical -Lthird-party/cJSON-1.7.6/ -lcjson -lcurl


OBJ_FILES=$(patsubst %.cpp,obj/%.o,$(SRC_FILES))

ical_events: ${OBJ_FILES}
	$(CXX) $(LDFLAGS) -o $@ $^


obj/%.o : %.cpp
	@mkdir -p obj
	$(CXX) $(CPPFLAGS) -c $< -o $@

obj/%.o : %.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

