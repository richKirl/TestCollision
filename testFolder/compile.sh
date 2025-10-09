g++-14 \
 -std=c++26 -O3 -msse4.2 -mavx \
 main.cpp Window.cpp CubeV.cpp Object3D.cpp Shader.cpp Font.cpp AABB.cpp BVH.cpp Collision.cpp Camera.cpp Terrain.cpp Route.cpp Ray.cpp Light.cpp\
 -o \
 testcolBB \
 -DSHM -I/usr/include/freetype2/ -lGL -lGLU -lGLEW -lglfw -lm -lfreetype