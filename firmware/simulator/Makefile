#######
#
#  To add new executables for building look lower in this file for "EXE_TARGETS"
#
#######

#######
#
#  If the location of this directory is changed relative to the src/
#  directory OR if the src/shared/ or src/Motherboard/ directories
#  are moved, then update the SRCDIR, SHAREDDIR, and MOTHERDIR variables
#

# Relative path to G3Firmware/firmware/src

SRCDIR = ../src

#  Relative paths to the shared sources and Motherboard sources

SHAREDDIR = $(SRCDIR)/MightyBoard/shared
MOTHERDIR = $(SRCDIR)/MightyBoard/Motherboard
BOARDDIR  = $(MOTHERDIR)/boards/mighty_one
AVRFIXDIR = $(MOTHERDIR)/avrfix

#
#######

#######
#
#  Since we need to compile sources from other directories,
#  use make's VPATH functionality

VPATH=./ $(SHAREDDIR) $(MOTHERDIR) $(AVRFIXDIR)

#
#######

AVRFIXFLAGS = -DTEST_ON_PC

#######
#
#  OS/Platform dependencies -- deal with them here

# Which platform?
BUILD_OS:=$(subst /,_,$(shell uname -s))

# Begin OS/Platform dependencies

ifeq ($(BUILD_OS), Darwin)

CXX = g++
CXXFLAGS = -Wall -g -DSIMULATOR -I./ -I$(SHAREDDIR) -I$(MOTHERDIR) -I$(BOARDDIR) -I$(AVRFIXDIR) $(AVRFIXFLAGS) -DMODEL_REPLICATOR2
CC = cc
CCFLAGS = -Wall -g -DSIMULATOR -I./ -I$(SHAREDDIR) -I$(MOTHERDIR) -I$(BOARDDIR) -I$(AVRFIXDIR) $(AVRFIXFLAGS) -DMODEL_REPLICATOR2
LDFLAGS = -g -lm
OBJ = .o
DEP = .d
OBJDIR = DarwinObj
EXEDIR = $(OBJDIR)

MAKE  = make
MKDIR = mkdir -p
RMDIR = rm -rf

endif

ifeq ($(BUILD_OS), Linux)

CXX = gcc
CXXFLAGS = -Wall -g -DSIMULATOR -I./ -I$(SHAREDDIR) -I$(MOTHERDIR) -I$(BOARDDIR) -I$(AVRFIXDIR) $(AVRFIXFLAGS)
CC = gcc
CCFLAGS = -Wall -g -DSIMULATOR -I./ -I$(SHAREDDIR) -I$(MOTHERDIR) -I$(BOARDDIR) -I$(AVRFIXDIR) $(AVRFIXFLAGS)
LDFLAGS = -g -lstdc++
OBJ = .o
DEP = .d
OBJDIR = LinuxObj
EXEDIR = $(OBJDIR)

MAKE  = make
MKDIR = mkdir -p
RMDIR = rm -rf

endif

#  End OS/Platform dependencies (hopefully)
#
##########

##########
#
#  Add executables to build to the EXE_TARGETS variable
#
##########

EXE_TARGETS = planner sailtime s3gdump

##########
#
#  For each executable to build, list its sources using the convention
#
#    <executable-name>_SRCS = source-file-1 [source-file-2 ...]
#    <executable-name>_OBJS = \
#             $(addprefix $(OBJDIR)/, $(notdir $(planner_SRCS:.cc=$(OBJ))))
#
##########

planner_DEFS = $(AVRFIXFLAGS)
planner_SRCS = planner.cc \
	  StepperAccelPlannerExtras.cc \
	  s3g.c \
	  s3g_stdio.c \
	  $(AVRFIXDIR)/avrfix.c \
	  $(SHAREDDIR)/StepperAccelPlanner.cc \
	  $(MOTHERDIR)/Point.cc \
	  $(MOTHERDIR)/Steppers.cc \
	  $(MOTHERDIR)/StepperAxis.cc
planner_LIBS = m

planner_OBJS = $(notdir $(patsubst %.c,%$(OBJ),$(planner_SRCS:.cc=$(OBJ))))

sailtime_DEFS = $(AVRFIXFLAGS) -DSAILTIME
sailtime_SRCS = sailtime.cc \
	  StepperAccelPlannerExtras.cc \
	  s3g.c \
	  s3g_stdio.c \
	  $(AVRFIXDIR)/avrfix.c \
	  $(SHAREDDIR)/StepperAccelPlanner.cc \
	  $(MOTHERDIR)/Point.cc \
	  $(MOTHERDIR)/Steppers.cc \
	  $(MOTHERDIR)/StepperAxis.cc
sailtime_LIBS = m

sailtime_OBJS = $(notdir $(patsubst %.c,%$(OBJ),$(sailtime_SRCS:.cc=$(OBJ))))

#float_planner_OBJS = $(notdir $(patsubst %.c,%$(OBJ),$(planner_SRCS:.cc=$(OBJ))))

s3gdump_SRCS = s3gdump.c \
	s3g.c \
	s3g_stdio.c
s3gdump_OBJS = $(notdir $(s3gdump_SRCS:.c=$(OBJ)))
s3gdump_LIBS = m

##########
#
#  Everything from here on down is mundane
#
##########

EXE_TARGET_OBJS = $(foreach objs, \
	$(addsuffix _OBJS, $(EXE_TARGETS)), \
	$(addprefix $(OBJDIR)/, $($(objs))))

LINK_TARGETS = $(addprefix $(OBJDIR)/, $(EXE_TARGETS))

all:: $(LINK_TARGETS)

clean:
	test -d $(OBJDIR) && $(RMDIR) $(OBJDIR)

# Pull in auto-generated dependency information
-include $(wildcard $(OBJDIR)/*.d)

$(LINK_TARGETS): $(EXE_TARGET_OBJS)
	$(CXX) -o $@ \
		$(addprefix $(OBJDIR)/, $($(notdir $(addsuffix _OBJS, ${@})))) \
		$(addprefix -l, $($(notdir $(addsuffix _LIBS, ${@})))) \
	        $(LDFLAGS)

# Really should have variables for the dependency switches, but
# there's only so many hours in the day
$(OBJDIR)/%$(OBJ): %.cc
	test -d $(OBJDIR) || $(MKDIR) $(OBJDIR)
	$(CXX) $(CXXFLAGS) $($(notdir $(addsuffix _DEFS, $(basename ${@})))) -c -o $@ $<
	$(CXX) $(CXXFLAGS) $($(notdir $(addsuffix _DEFS, $(basename ${@})))) \
		-MM -MF $(OBJDIR)/$*$(DEP) -MT $(OBJDIR)/$*$(OBJ) $(CXXFLAGS) $<

$(OBJDIR)/%$(OBJ): %.c
	test -d $(OBJDIR) || $(MKDIR) $(OBJDIR)
	$(CC) $(CCFLAGS) $($(notdir $(addsuffix _DEFS, $(basename ${@})))) -c -o $@ $<
	$(CC) $(CCFLAGS) $($(notdir $(addsuffix _DEFS, $(basename ${@})))) \
		-MM -MF $(OBJDIR)/$*$(DEP) -MT $(OBJDIR)/$*$(OBJ) $(CXXFLAGS) $<
