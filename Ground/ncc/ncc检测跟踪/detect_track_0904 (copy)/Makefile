GPU=1
CUDNN=1
DEBUG=0
OPENCV=1

ARCH= -gencode arch=compute_30,code=sm_30 \
	  -gencode arch=compute_35,code=sm_35 \
	  -gencode arch=compute_50,code=[sm_50,compute_50] \
	  -gencode arch=compute_52,code=[sm_52,compute_52]
# -gencode arch=compute_20,code=[sm_20,sm_21] \

# This is what I use, uncomment if you know your arch and want to specify
# ARCH= -gencode arch=compute_52,code=compute_52

VPATH=./darknet/include:./darknet/src:./detection:./track
TARGET=detect
OBJDIR=./obj/
CXXOBJDIR=./objcpp/

CC=gcc
CXX=g++
AR=ar
NVCC=nvcc 
OPTS=-Ofast
LDFLAGS= -lm -pthread
COMMON= 
CFLAGS=-Wall -Wfatal-errors -Wno-unused-result -fPIC
CXXFLAGS=-pipe -std=c++0x -Wall -W -fPIC
INCPATH=-I. -I./darknet/include -I./darknet/src -I./detection -I./track

ifeq ($(DEBUG), 1) 
OPTS=-O0 -g
endif

CFLAGS+=$(OPTS)
CXXFLAGS+=$(OPTS)

ifeq ($(OPENCV), 1) 
COMMON+= -DOPENCV
CFLAGS+= -DOPENCV
LDFLAGS+= `pkg-config --libs opencv` 
COMMON+= `pkg-config --cflags opencv` 
endif

ifeq ($(GPU), 1) 
COMMON+= -DGPU -I/usr/local/cuda/include/
CFLAGS+= -DGPU
LDFLAGS+= -L/usr/local/cuda/lib64 -lcuda -lcudart -lcublas -lcurand
endif

ifeq ($(CUDNN), 1) 
COMMON+= -DCUDNN 
CFLAGS+= -DCUDNN
LDFLAGS+= -lcudnn
endif

OBJ=$(patsubst %.c,%.o,$(notdir $(wildcard ./darknet/src/*.c ./detection/*.c)))
CXXOBJ=$(patsubst %.cpp,%.o,$(notdir $(wildcard ./track/*.cpp *.cpp)))

ifeq ($(GPU), 1)
OBJ+=$(patsubst %.cu,%.o,$(notdir $(wildcard ./darknet/src/*.cu)))
endif

OBJS = $(addprefix $(OBJDIR), $(OBJ))
CXXOBJS = $(addprefix $(CXXOBJDIR), $(CXXOBJ))

all: obj cxxobj $(TARGET)

$(TARGET): $(CXXOBJS) $(OBJS)
	$(CXX) $(LIBS) $^ -o $@ $(LDFLAGS)

$(CXXOBJDIR)%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) -c $< -o $@

$(OBJDIR)%.o: %.c
	$(CC) $(COMMON) $(CFLAGS) $(INCPATH) -c $< -o $@

$(OBJDIR)%.o: %.cu
	$(NVCC) $(ARCH) $(COMMON) $(INCPATH) --compiler-options "$(CFLAGS)" -c $< -o $@

obj:
	mkdir -p obj

cxxobj:
	mkdir -p objcpp

.PHONY: clean

clean:
	rm -rf $(OBJS) $(CXXOBJS) $(TARGET)

