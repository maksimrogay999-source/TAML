# Пути к NDK и компилятору
NDK_PATH := $(HOME)/android-sdk/ndk/android-ndk-r29
TOOLCHAIN := $(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/bin
CXX := $(TOOLCHAIN)/aarch64-linux-android24-clang++

# Пути к Android SDK (для javac и d8)
ANDROID_HOME := $(HOME)/android-sdk
PLATFORM_JAR := $(ANDROID_HOME)/platforms/android-34/android.jar
D8 := $(ANDROID_HOME)/build-tools/34.0.0/d8

# Флаги компиляции и линковки для C++
CXXFLAGS := -shared -fPIC -O2 -Wall -I/data/data/com.termux/files/usr/include/luajit-2.1
LDFLAGS := -llog -ldl -static-libstdc++
LUAJIT_LIB := libluajit.a

# Исходники и цели
SRC_CPP := main.cpp
TARGET_SO := build/libmodloader.so

JAVA_SRC := MOD.java
JAVA_CLASS := out/com/modloader/loader/MOD.class
TARGET_DEX := build/classesX.dex

# Цель по умолчанию
all: build

# Сборка всего
build: $(TARGET_SO) $(TARGET_DEX)
	@echo "Build complete. Files are in the 'build/' directory."

# Сборка нативной библиотеки
$(TARGET_SO): $(SRC_CPP) $(LUAJIT_LIB)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $(SRC_CPP) $(LUAJIT_LIB) $(LDFLAGS)

# Сборка Java-класса
$(JAVA_CLASS): $(JAVA_SRC)
	@mkdir -p out
	javac -source 8 -target 8 \
	  -bootclasspath $(PLATFORM_JAR) \
	  -d out/ \
	  $(JAVA_SRC)

# Конвертация класса в dex
$(TARGET_DEX): $(JAVA_CLASS)
	@mkdir -p build
	$(D8) --output build/ \
	  --lib $(PLATFORM_JAR) \
	  $(JAVA_CLASS)
	mv build/classes.dex $(TARGET_DEX)

# Очистка
clean:
	rm -rf build out

.PHONY: all build clean