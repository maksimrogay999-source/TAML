NDK_PATH      := $(HOME)/android-sdk/ndk/android-ndk-r29
TOOLCHAIN     := $(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/bin
CXX           := $(TOOLCHAIN)/aarch64-linux-android24-clang++

ANDROID_HOME  := $(HOME)/android-sdk
PLATFORM_JAR  := $(ANDROID_HOME)/platforms/android-34/android.jar
D8            := $(ANDROID_HOME)/build-tools/34.0.0/d8

CXXFLAGS      := -shared -fPIC -O2 -Wall \
                 -I/data/data/com.termux/files/usr/include/luajit-2.1 \
                 -I$(HOME)/TAML
LDFLAGS       := -llog -ldl -static-libstdc++ -L$(HOME)/TAML -lluajit

LIBS          := classes.jar core.jar

SRC_CPP       := main.cpp
JAVA_SRC      := MOD.java

BUILD_DIR     := build
OUT_DIR       := out
TARGET_SO     := $(BUILD_DIR)/libmodloader.so
TARGET_DEX    := $(BUILD_DIR)/classesX.dex

.PHONY: all build clean native java help

all: build

build: native java
	@echo "✅ Build complete. Files are in the '$(BUILD_DIR)/' directory."

native: $(TARGET_SO)

$(TARGET_SO): $(SRC_CPP)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC_CPP) $(LDFLAGS)

java: $(TARGET_DEX)

$(OUT_DIR)/.javac_done: $(JAVA_SRC)
	@rm -rf $(OUT_DIR)
	@mkdir -p $(OUT_DIR)
	javac -source 8 -target 8 -Xlint:-options \
	  -bootclasspath $(PLATFORM_JAR) \
	  -classpath $(LIBS) \
	  -d $(OUT_DIR)/ \
	  $(JAVA_SRC)
	@touch $(OUT_DIR)/.javac_done

$(TARGET_DEX): $(OUT_DIR)/.javac_done
	@mkdir -p $(BUILD_DIR)
	cd $(OUT_DIR) && jar cf ../$(BUILD_DIR)/classes.jar .
	$(D8) --output $(BUILD_DIR)/ \
	  --lib $(PLATFORM_JAR) \
	  $(BUILD_DIR)/classes.jar \
	  $(LIBS)
	@if [ -f "$(BUILD_DIR)/classes.dex" ]; then mv $(BUILD_DIR)/classes.dex $(TARGET_DEX); fi
	@rm -f $(BUILD_DIR)/classes.jar

clean:
	rm -rf $(BUILD_DIR) $(OUT_DIR)
	@echo "🧹 Cleaned."

help:
	@echo "TAML Mod Loader - Makefile targets:"
	@echo "  make all     - Build everything (native + java)"
	@echo "  make native  - Build only libmodloader.so"
	@echo "  make java    - Build only classesX.dex"
	@echo "  make clean   - Remove build artifacts"