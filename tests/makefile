
#Set this to @ to keep the makefile quiet
ifndef SILENCE
	SILENCE = @
endif

#--- Inputs ----#
COMPONENT_NAME = Rpi_Test_cases
CPPUTEST_HOME = cpputest

CPPUTEST_USE_EXTENSIONS = Y
CPPUTEST_USE_GCOV=Y

CPP_PLATFORM = Gcc

LD_LIBRARIES = -L$(CPPUTEST_HOME)/lib -pthread -lpthread

SRC_DIRS = $(wildcard ./../src/*)

TEST_SRC_DIRS = ./ ./gps_module/ ./client_controller/ ./can_bus/ ./utils/

INCLUDE_DIRS =\
  .\
  $(CPPUTEST_HOME)/include\

include $(CPPUTEST_HOME)/build/MakefileWorker.mk