chip8-emu.exe: chip8.c
	gcc -g -o chip8-emu.exe chip8.c

clean:
	rm chip8-emu.exe