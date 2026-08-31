# look for IBM Clang C compiler
find_program(
    IBM_CLANG_C ibm-clang64
    HINTS /usr/lpp/IBM/cnw/v2r1/openxl/bin/
    REQUIRED
)
# look for IBM Clang C++ compiler
find_program(
    IBM_CLANG_CXX ibm-clang++64
    HINTS /usr/lpp/IBM/cnw/v2r1/openxl/bin/
    REQUIRED
)
# find assembler
find_program(
    IBM_ASSEMBLER as
    REQUIRED
)

# use IBM Clang compiler for C and C++ files
set(CMAKE_C_COMPILER "${IBM_CLANG_C}")
set(CMAKE_CXX_COMPILER "${IBM_CLANG_CXX}")
set(CMAKE_ASM_COMPILER "${IBM_ASSEMBLER}")

set(IBM_CLANG_ARGS "-m64 -fzos-le-char-mode=ascii")
set(CMAKE_C_FLAGS_INIT "${IBM_CLANG_ARGS}")
set(CMAKE_CXX_FLAGS_INIT "${IBM_CLANG_ARGS}")
set(CMAKE_ASM_FLAGS_INIT "-mGOFF")

# try to locate zOpen and include it when searching for libraries and files
if (DEFINED ENV{ZOPEN_ROOTFS} AND NOT DEFINED ZOPEN_ROOTFS)
    set(ZOPEN_ROOTFS $ENV{ZOPEN_ROOTFS})
    message(STATUS "Found zOpen: ${ZOPEN_ROOTFS}")
    list(APPEND CMAKE_SYSTEM_PREFIX_PATH "${ZOPEN_ROOTFS}/usr/local")
endif()
