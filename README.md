# learnOpenGL2bgfx

Convert the examples from [learnopengl.com](https://learnopengl.com) to the [bgfx framework](https://github.com/bkaradzic/bgfx) using a
reduced version of [Crown Engine](https://github.com/dbartolini).

## Building

### Prerequisites

### Android

Android NDK (https://developer.android.com/tools/sdk/ndk/index.html)

	$ export ANDROID_NDK_ROOT=<path/to/android_ndk>
	$ export ANDROID_NDK_ARM=<path/to/android_ndk_arm>

### Linux (Ubuntu >= 16.04)

    $ sudo apt-get install mesa-common-dev libgl1-mesa-dev libpulse-dev libxrandr-dev

### Windows

MSYS2 (http://www.msys2.org)

### Building and running Level Editor

	$ make linux-release64
	$ cd build/linux64/bin
	$ ./crown-release
