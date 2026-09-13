cmake_minimum_required(VERSION 3.15)

project(IopenMPI LANGUAGES CXX)

# ============================================================
# 全局设置
# ============================================================
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 默认使用 x64 平台（VS2019 生成器）
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# ============================================================
# MS-MPI SDK 配置
# 通过 cmake -DMSMPI_ROOT=... 可覆盖，默认 D:/software/MSMPISDK/MPI
# ============================================================
set(MSMPI_ROOT "D:/software/MSMPISDK/MPI" CACHE PATH "MS-MPI SDK root directory")

if(NOT EXISTS "${MSMPI_ROOT}/Include/mpi.h")
    message(FATAL_ERROR "未在 ${MSMPI_ROOT} 下找到 MS-MPI SDK (Include/mpi.h)。"
                        "请通过 -DMSMPI_ROOT=<路径> 指定正确的 SDK 路径。")
endif()

# 根据目标平台选择 Lib 目录（x64 / x86）
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(MSMPI_ARCH "x64")
else()
    set(MSMPI_ARCH "x86")
endif()

set(MSMPI_INCLUDE_DIR "${MSMPI_ROOT}/Include")
set(MSMPI_LIBRARY_DIR "${MSMPI_ROOT}/Lib/${MSMPI_ARCH}")
set(MSMPI_LIBRARIES "${MSMPI_LIBRARY_DIR}/msmpi.lib")

if(NOT EXISTS "${MSMPI_LIBRARIES}")
    message(FATAL_ERROR "未找到 MS-MPI 库文件: ${MSMPI_LIBRARIES}")
endif()

message(STATUS "MS-MPI SDK root     : ${MSMPI_ROOT}")
message(STATUS "MS-MPI include dir  : ${MSMPI_INCLUDE_DIR}")
message(STATUS "MS-MPI library      : ${MSMPI_LIBRARIES}")

# ============================================================
# 导入 MS-MPI 作为外部库目标
# ============================================================
add_library(MS-MPI::msmpi UNKNOWN IMPORTED)
set_target_properties(MS-MPI::msmpi PROPERTIES
    IMPORTED_LOCATION             "${MSMPI_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${MSMPI_INCLUDE_DIR}"
)

# ============================================================
# 可执行文件
# ============================================================
set(IOPENMPI_SOURCES
    src/IopenMPI.cpp
    src/IopenMPI.h
    src/Command.cpp
    src/Command.h
    src/TaskRegistry.cpp
    src/TaskRegistry.h
    src/DistributedTask.cpp
    src/DistributedTask.h
    src/FilterTask.cpp
    src/FilterTask.h
    src/SumTask.cpp
    src/SumTask.h
    src/StructTask.cpp
    src/StructTask.h
)

add_executable(IopenMPI ${IOPENMPI_SOURCES})

# 让源码可以用 #include "Command.h" 这样的短路径，也让 IDE 能找到头文件
target_include_directories(IopenMPI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(IopenMPI PRIVATE MS-MPI::msmpi)

# 在 VS 解决方案中按目录结构组织源文件
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${IOPENMPI_SOURCES})

# MSVC 特定选项：源码含中文注释，指定 UTF-8 编码避免警告
if(MSVC)
    target_compile_options(IopenMPI PRIVATE /utf-8 /wd4828)
endif()

# ============================================================
# 运行辅助：MS-MPI 运行时（msmpi.dll / mpiexec.exe）位置
# SDK 目录通常只有头文件和库；运行时由 Microsoft MPI 安装包提供
# ============================================================
find_file(MSMPI_MPIEXEC_EXECUTABLE mpiexec.exe
    HINTS
        "$ENV{ProgramW6432}/Microsoft MPI/Bin"
        "${MSMPI_ROOT}/Bin/${MSMPI_ARCH}"
    PATHS "C:/Program Files/Microsoft MPI/Bin"
)

# 添加一个调试运行配置，让 VS 调试时能找到 msmpi.dll
set_target_properties(IopenMPI PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=C:/Program Files/Microsoft MPI/Bin;${MSMPI_ROOT}/Bin/${MSMPI_ARCH};%PATH%"
)

if(MSMPI_MPIEXEC_EXECUTABLE)
    message(STATUS "提示: 可用 ${MSMPI_MPIEXEC_EXECUTABLE} -n 4 $<TARGET_FILE:IopenMPI> 运行")
endif()
