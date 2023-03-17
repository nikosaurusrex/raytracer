default:
	make -C core
	make -C cli

all:
	make -C core
	make -C cli
	make -C gui

clean:
	make -C core clean
	make -C cli clean
	make -C gui clean