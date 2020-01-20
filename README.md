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
	$ ./learnOpenGL2bgfx-release

License
-------
	Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
	Copyright (c) 2020 vasconssa

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
