cmake_minimum_required(VERSION 3.16)
project(electrical_project C CXX)

# ================== Arduino (arduino-cli) 环境 ==================
set(TOOLS_DIR ${CMAKE_SOURCE_DIR}/.tools)
set(AVR_HARDWARE ${TOOLS_DIR}/arduino-cli/data/packages/arduino/hardware/avr/1.8.8)
set(AVR_GCC ${TOOLS_DIR}/arduino-cli/data/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7)

# 代码补全 / 跳转所需的头文件路径（不用于真正构建）
include_directories(
        ${AVR_HARDWARE}/cores/arduino
        ${AVR_HARDWARE}/variants/standard
        ${AVR_GCC}/avr/include
        ${AVR_GCC}/lib/gcc/avr/7.3.0/include
)
add_definitions(
        -DARDUINO=10808
        -DARDUINO_AVR_UNO
        -DARDUINO_ARCH_AVR
        -DF_CPU=16000000L
        -D__AVR__
        -D__AVR_ATmega328P__
)

include_directories(".tools/arduino-cli/user/libraries/Adafruit_NeoPixel/")

# 上传端口（板子换端口时改这里）
set(ARDUINO_PORT /dev/ttyUSB0)

# 仅为让 CLion 索引所有草图文件（请勿构建此目标）
file(GLOB_RECURSE SKETCHES RELATIVE ${CMAKE_SOURCE_DIR} "day*/p*/*.c")
add_library(sketch_index STATIC EXCLUDE_FROM_ALL ${SKETCHES})

# 为每个 .c 草图生成 compile_xxx / upload_xxx 运行目标
foreach(SK ${SKETCHES})
    string(REPLACE "/" "_" T ${SK})
    string(REPLACE ".c" "" T ${T})
    add_custom_target(compile_${T}
            COMMAND ${TOOLS_DIR}/compile.sh ${CMAKE_SOURCE_DIR}/${SK}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "编译 ${SK}")
    add_custom_target(upload_${T}
            COMMAND ${CMAKE_COMMAND} -E env ARDUINO_PORT=${ARDUINO_PORT}
                    ${TOOLS_DIR}/compile.sh --upload ${CMAKE_SOURCE_DIR}/${SK}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "烧录 ${SK} 到 ${ARDUINO_PORT}")
endforeach()
