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
COMPONENT_NAME = Rpi_Test_cases
CPPUTEST_HOME = ../cpputest

CPPUTEST_USE_EXTENSIONS = Y
CPP_PLATFORM = Gcc


SRC_DIRS = ./src

TEST_SRC_DIRS = ./

INCLUDE_DIRS =\
  .\
  $(CPPUTEST_HOME)/include\

include $(CPPUTEST_HOME)/build/MakefileWorker.mk


