# Make sure Make is installed
echo "\$ sudo apt install build-essential"
sudo apt install build-essential

# Make sure Mingw is installed
echo "\$ sudo apt-get install mingw-w64"
sudo apt-get install mingw-w64

cd source/raylib/src # (Raylib's makefile is in here)

# Undo previous build in case it was done before differently
echo "
\$ make clean"
make clean

# Build raylib, causing libraylib.a to be created here in source/raylib/src
echo "
\$ make PLATFORM=PLATFORM_DESKTOP PLATFORM_OS=WINDOWS CC=CC=x86_64-w64-mingw32-gcc RAYLIB_LIBTYPE=STATIC"
make PLATFORM=PLATFORM_DESKTOP PLATFORM_OS=WINDOWS CC=x86_64-w64-mingw32-gcc RAYLIB_LIBTYPE=STATIC

echo "
cd ../.."
cd ../../.. # (I want to build the game itself out here in the parent folder)

# Build my game itself. The -I flag tells it to look for header files in that folder,
# the -L flag tells it to look for libraylib.a in that folder,
# and IDK what the rest does but its all necessary, I got it from raylib documentation
echo "
\$ x86_64-w64-mingw32-gcc source/hellotest.c -o hellotest.exe \
    -I./source/raylib/src -L./source/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32"
x86_64-w64-mingw32-gcc source/main.c -o winmain.exe \
    -I./source/raylib/src -L./source/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32
