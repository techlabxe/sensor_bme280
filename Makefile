TARGET=sample

INCLUDES=\
	./	\

SRCS=\
	sensor_bme280.cpp \
	sample_bme280.cpp \

LDFLAGS=\
	-lpigpiod_if2   \

OBJDIR=./obj
OBJS=$(addprefix $(OBJDIR)/,$(notdir $(SRCS:.cpp=.o)))
CXX=g++

CFLAGS=-Wall 

ifeq ($(DEBUG), 1)
  CFLAGS += -g -O0
else
  CFLAGS += -O2
endif

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(OBJDIR)
	$(CXX) $(CFLAGS) $(addprefix  -I,$(INCLUDES)) -o $@ -c $<


all: clean $(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET)
