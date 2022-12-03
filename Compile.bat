g++ config.cpp -o config.exe -m32 -s -Wall -std=c++1y -O2 -L lib/ -static-libgcc -lfmod -lfmodL
g++ osu!asio_sound.cpp -o osu!asio_sound.exe -m32 -s -Wall -std=c++1y -O2 -L lib/ -static-libgcc -lfmod -lfmodL
g++ "osu!asio_sound (select osu exename).cpp" -o "osu!asio_sound (select osu exename).exe" -m32 -s -Wall -std=c++1y -O2 -L lib/ -static-libgcc -lfmod -lfmodL
g++ -D__DEBUG__ -c dllmain.cpp -o dllmain.o  -m32 -g3 -mwindows -Wall -std=c++1y -O2 -DBUILDING_DLL=1
g++ -D__DEBUG__ -shared dllmain.o -o osu!asio_sound.dll -static-libgcc -L lib/ -lminhook -m32 -g3 -Wall -mwindows -s -Wl,--output-def,libosu!asio_sound.def,--out-implib,libosu!asio_sound.a,--add-stdcall-alias