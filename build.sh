# Make sure GCC and Make are installed
# (the former is used to compile both my code and raylib's, the latter just for raylib's)
echo "\$ sudo apt install build-essential"
sudo apt install build-essential

# Make sure required libraries for raylib are installed
echo "
\$ sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev"
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev

# Warning: raylib docs say building for wayland requires manual generation of some specific files or some shit. You need these additional packages (uncomment this if so):
#sudo dnf install wayland-devel libxkbcommon-devel wayland-protocols-devel
# But beyond that, just check these links for whatever raysan has to say about it:
# https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
# https://github.com/raysan5/raylib/issues/2666

cd source/raylib/src # (Raylib's makefile is in here)

# Undo previous build in case it was done before differently
echo "
\$ make clean"
make clean

# Build raylib, causing libraylib.a to be created here in source/raylib/src
echo "
\$ make PLATFORM=PLATFORM_DESKTOP"
make PLATFORM=PLATFORM_DESKTOP
echo "
\$ sudo make install"
sudo make install

echo "
cd ../.."
cd ../../.. # (I want to build the game itself out here in the parent folder)

# Build my game itself. The -I flag tells it to look for header files in that folder,
# the -L flag tells it to look for libraylib.a in that folder,
# and IDK what the rest does but its all necessary, I got it from raylib documentation
echo "
\$ gcc source/main.c -o test -I./source/raylib/src -L./source/raylib/src -lraylib -lGL -lm -lpthread -ldl -lrt -lX11"
gcc source/main.c -o main -I./source/raylib/src -L./source/raylib/src -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
