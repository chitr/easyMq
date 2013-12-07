#CFLAGS := -fPIC -O3 -g -Wall -Werror
CFLAGS := -I../include -fPIC -O0 -g -ggdb -rdynamic -Wall -Wstrict-prototypes -DMONGO_HAVE_STDINT 
#--std=c99 
LDFLAGS:=-ldl
CC := gcc
MAJOR := 0
MINOR := 0
NAME := easyMq
LIB  := ../lib/
INCLUDE  := ../include
OBJ := easyMq_PosixMQ.o 
VERSION := $(MAJOR)
#VERSION := $(MAJOR).$(MINOR)
 
lib: lib$(NAME).so.$(VERSION)
 
test: $(NAME)_test
	LD_LIBRARY_PATH=. ./$(NAME)_test
 
$(NAME)_test: lib$(NAME).so
	$(CC) $(NAME)_test.c -o $@ -L. -l$(NAME)
 
lib$(NAME).so: lib$(NAME).so.$(VERSION)
	ldconfig -v -n .
	ln -s lib$(NAME).so.$(MAJOR) lib$(NAME).so
 
lib$(NAME).so.$(VERSION): $(OBJ)
	$(CC) -shared -Wl,-soname,lib$(NAME).so.$(MAJOR) $^ -o $@
 
clean:
	$(RM) $(NAME)_test *.o *.so*
	$(RM) $(LIB)/$(NAME)_test 
	$(RM) $(LIB)/*.o 
	$(RM) $(LIB)/*.so*
install:
	cp lib*.so* /usr/lib64
	cp $(INCLUDE)/*.h /usr/include
