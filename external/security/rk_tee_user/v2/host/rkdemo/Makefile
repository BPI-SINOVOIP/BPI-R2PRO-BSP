# Normally this makefile shouldn't be called directly and we expect the output
# path to be on a certain location to fit together with the other OP-TEE
# gits and helper scripts.
ifeq ($O,)
$(error output path should be specified when calling this makefile)
endif

include $(TA_DEV_KIT_DIR)/host_include/conf.mk

# By default we expect optee_client exported folder to be on a certain relative
# path, but if the client specifies the OPTEE_CLIENT_EXPORT then that path will
# be used instead.
OPTEE_CLIENT_EXPORT ?= ../../client_export/
ifeq "$(COMPILE_NS_USER)" "64"
OPTEE_CLIENT_LIB ?= ../../../../bin/optee_v2/lib/arm64/
else
OPTEE_CLIENT_LIB ?= ../../../../bin/optee_v2/lib/arm/
endif


CC		?= $(CROSS_COMPILE)gcc
CPP		?= $(CROSS_COMPILE)cpp
LD		?= $(CROSS_COMPILE)ld
AR		?= $(CROSS_COMPILE)ar
NM		?= $(CROSS_COMPILE)nm
OBJCOPY		?= $(CROSS_COMPILE)objcopy
OBJDUMP		?= $(CROSS_COMPILE)objdump
READELF		?= $(CROSS_COMPILE)readelf


srcs := testapp_ca.c
objs 	:= $(patsubst %.c,$(O)/%.o, $(srcs))

srcs1 := testapp_storage_ca.c
objs1 	:= $(patsubst %.c,$(O)/%.o, $(srcs1))


CFLAGS += -I$(OPTEE_CLIENT_EXPORT)/public
CFLAGS += -I$(TA_DEV_KIT_DIR)/host_include
CFLAGS += -I../../ta/testapp/include
CFLAGS += -I../../ta/testapp_storage/include
CFLAGS += -Wall -Wcast-align  \
	  -Werror-implicit-function-declaration -Wextra -Wfloat-equal \
	  -Wformat-nonliteral -Wformat-security -Wformat=2 -Winit-self \
	  -Wmissing-declarations -Wmissing-format-attribute \
	  -Wmissing-include-dirs -Wmissing-noreturn \
	  -Wmissing-prototypes -Wnested-externs -Wpointer-arith \
	  -Wshadow -Wstrict-prototypes -Wswitch-default \
	  -Wwrite-strings \
	  -Wno-missing-field-initializers -Wno-format-zero-length

LDFLAGS += -L$(OPTEE_CLIENT_LIB)
LDFLAGS += -L$(OPTEE_CLIENT_EXPORT)/lib -lteec


.PHONY: all
all: rkdemo
rkdemo: testapp testapp_storage

testapp: $(objs)
	@echo "  LD      $(O)/$@"
	$(q)@$(CC) -o $(O)/$@ $+ $(LDFLAGS)

testapp_storage: $(objs1)
	@echo "  LD      $(O)/$@"
	$(q)@$(CC) -o $(O)/$@ $+ $(LDFLAGS)
	@echo "  OPTEE_CLIENT_LIB=$(OPTEE_CLIENT_LIB)"

libRkWvClient.so: $(objs2)
	@echo "  LD      $(O)/$@"
	$(q)@$(CC) -shared -Wl,-soname -o $(O)/$@ $+ $(LDFLAGS)
	@echo "  OPTEE_CLIENT_LIB=$(OPTEE_CLIENT_LIB)"

$(O)/%.o: $(CURDIR)/%.c
	$(q)mkdir -p $(O)/
	@echo '  CC      $<'
	$(q)$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@echo '  CLEAN $(O)'
	$(q)rm -f $(O)/testapp
	$(q)$(foreach obj,$(objs), rm -f $(obj))
	$(q)rm -f $(O)/testapp_storage
	$(q)$(foreach obj,$(objs1), rm -f $(obj))
