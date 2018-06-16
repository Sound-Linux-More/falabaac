#  falab - free algorithm lab 
#  Copyright (C) 2012 luolongzhi  罗龙智 (Chengdu, China)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
#  filename: Makefile
#  version : v1.0.0
#  time    : 2012/07/07 18:42 
#  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
#  code URL: http://code.google.com/p/falab/
#
#  comment : this file is the simple template which will be used in falab,
#            it will be changed according to the project
#

include Makefile.include

INCLUDEDIR = ./include

ifeq ($(ARCH),ARCH_ARM)
LDFLAGS += -L./libfalabaac -static -lfalabaac -lm -lpthread -lrt 
else
ifeq ($(DEBUG), Y)
LDFLAGS += -lm -lpthread -lrt 
else 
LDFLAGS += -L./libfalabaac -static -lfalabaac -lm -lpthread -lrt 
endif
endif

ifeq ($(DEBUG), Y)
TARGET      =  falabaac_g 
CSRCFILES =  $(shell ls ./frontend/*.c)
CSRCFILES += $(shell ls ./libfalabaac/*.c)
COBJFILES =  $(patsubst %.c,%.o,$(CSRCFILES))
else 
TARGET      =  falabaac 
CSRCFILES   =  $(shell ls ./frontend/*.c)
COBJFILES   =  $(patsubst %.c,%.o,$(CSRCFILES))
endif


SRCFILES    =  $(CSRCFILES) 
OBJFILES    =  $(COBJFILES) 

CFLAGS      += -I.  -I$(INCLUDEDIR)

all: clean preaction $(OBJFILES) 
	@rm $(TARGET) -f 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)	 

preaction:
ifeq ($(DEBUG), Y)
	cd ./libfalabaac
	@rm *.o -f
else 
	cd ./libfalabaac && $(MAKE) 
endif
	cd ..

clean : 
	@rm ./libfalabaac/*.o -f
	@rm ./libfalabaac/libfalabaac.a -f
	@rm ./libfalabaac/libfalabaac.so -f
	@rm ./frontend/*.o -f
	@rm *.out -f	
	@rm falabaac -f	
	@rm falabaac_g -f	

install :
	sudo cp ./libfalabaac/libfalabaac.a /usr/local/lib/
	sudo cp ./libfalabaac/libfalabaac.so /usr/local/lib/
	sudo cp ./include/fa_aacapi.h /usr/local/include/
	sudo cp ./include/fa_inttypes.h /usr/local/include/
	sudo cp ./falabaac /usr/local/bin/

