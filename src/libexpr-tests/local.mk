check: libexpr-tests_RUN

programs += libexpr-tests

libexpr-tests_NAME := libnixexpr-tests

libexpr-tests_ENV := _NIX_TEST_UNIT_DATA=$(d)/data GTEST_OUTPUT=xml:$$testresults/libexpr-tests.xml

libexpr-tests_DIR := $(d)

ifeq ($(INSTALL_UNIT_TESTS), yes)
  libexpr-tests_INSTALL_DIR := $(checkbindir)
else
  libexpr-tests_INSTALL_DIR :=
endif

libexpr-tests_SOURCES := \
    $(wildcard $(d)/*.cc) \
    $(wildcard $(d)/value/*.cc) \
    $(wildcard $(d)/flake/*.cc)

libexpr-tests_EXTRA_INCLUDES = \
    -I src/libexpr-test-support \
    -I src/libstore-test-support \
    -I src/libutil-test-support \
    $(INCLUDE_libexpr) \
    $(INCLUDE_libexprc) \
    $(INCLUDE_libfetchers) \
    $(INCLUDE_libstore) \
    $(INCLUDE_libstorec) \
    $(INCLUDE_libutil) \
    $(INCLUDE_libutilc)

libexpr-tests_CXXFLAGS += $(libexpr-tests_EXTRA_INCLUDES)

libexpr-tests_LIBS = \
    libexpr-test-support libstore-test-support libutil-test-support \
    libexpr libexprc libfetchers libstore libstorec libutil libutilc

libexpr-tests_LDFLAGS := -lrapidcheck $(GTEST_LIBS) -lgmock

ifdef HOST_WINDOWS
  # Increase the default reserved stack size to 65 MB so Nix doesn't run out of space
  libexpr-tests_LDFLAGS += -Wl,--stack,$(shell echo $$((65 * 1024 * 1024)))
endif
