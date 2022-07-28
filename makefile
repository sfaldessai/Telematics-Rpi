#---------
#
# CppUTest Examples Makefile
#
#----------

#Set this to @ to keep the makefile quiet
ifndef SILENCE
	SILENCE = @
endif

#--- Inputs ----#
COMPONENT_NAME = CustomTests
CPPUTEST_HOME = cpputest

CPPUTEST_USE_EXTENSIONS = Y
CPP_PLATFORM = Gcc


SRC_DIRS = 

TEST_SRC_DIRS = \
	AllTests

INCLUDE_DIRS =\
  .\
  $(CPPUTEST_HOME)/include\

include $(CPPUTEST_HOME)/build/MakefileWorker.mk


