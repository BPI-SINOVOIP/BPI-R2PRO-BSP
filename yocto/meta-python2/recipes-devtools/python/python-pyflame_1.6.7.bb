SUMMARY = "Pyflame: A Ptracing Profiler For Python"
DESCRIPTION = "Pyflame is a high performance profiling tool that generates \
flame graphs for Python. Pyflame is implemented in C++, and uses the Linux \
ptrace(2) system call to collect profiling information. It can take snapshots \
of the Python call stack without explicit instrumentation, meaning you can \
profile a program without modifying its source code. Pyflame is capable of \
profiling embedded Python interpreters like uWSGI. It fully supports profiling \
multi-threaded Python programs."
HOMEPAGE = "https://github.com/uber/pyflame"
SECTION = "devel/python"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=2ee41112a44fe7014dce33e26468ba93"

SRC_URI = "git://github.com/uber/pyflame.git;protocol=https;nobranch=1"
# v1.6.7
SRCREV = "c151d2f34737f28a1f5266a003b2b0720bbd9f96"

DEPENDS = "python"

S = "${WORKDIR}/git"

inherit pkgconfig autotools

COMPATIBLE_HOST_libc-musl_class-target = "null"
COMPATIBLE_HOST_mipsarch_class-target = "null"
COMPATIBLE_HOST_aarch64_class-target = "null"
COMPATIBLE_HOST_powerpc_class-target = "null"
COMPATIBLE_HOST_riscv64_class-target = "null"
COMPATIBLE_HOST_riscv32_class-target = "null"
