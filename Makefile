module_directory="/opt/asteria/modules"
library_directory="/usr/local/lib/asteria"
arch=$(shell uname -m)

all:
	@(cd google/bin/$(arch); ln -f -s libprotobuf.so.7.0.0 libprotobuf.so)
	@(cd google/bin/$(arch); ln -f -s libprotobuf.so.7.0.0 libprotobuf.so.7)
	@(cd google/bin/$(arch); ln -f -s libprotobuf.so.7.0.0 libprotobuf.so.7.0)
	@(cd src;make debug;)
	@(cd Run;make debug;)
	@(cd Modules/binary-module;make debug;)
	@(cd Modules/fs-module;make debug;)
	@(cd Modules/readline-module;make debug;)
	@(cd Modules/system-module;make debug;)
	
debug:

install:
	@mkdir -p $(module_directory)
	@mkdir -p $(library_directory)
	@cp -d -p ./google/bin/$(arch)/* $(library_directory)	
	@(if [ ! -f /etc/ld.so.conf.d/asteria.conf ]; \
		then \
			touch /etc/ld.so.conf.d/asteria.conf; \
			echo /usr/local/lib/asteria >> /etc/ld.so.conf.d/asteria.conf; \
		fi)
	@(cd src;make install;make clean;)
	@(cd Run;make install;make clean;)
	@(cd Modules/binary-module;make install;make clean;)
	@(cd Modules/fs-module;make install;make clean;)
	@(cd Modules/readline-module;make install;make clean;)
	@(cd Modules/system-module;make install;make clean;)
	@(cd Repl;make install;)
	@(cd RunCoffee;make install;)
	@(cd Modules/big-integer;make install;)
	@(cd Modules/console-module;make install;)
	@(cd Modules/shell-module;make install;)
	@(cd Modules/sprintf-module;make install;)
	@(cd Modules/coffee-compiler;make install;)
	@(cd Modules/concurrent-utils;make install;)
	@(ldconfig)

uninstall:
	@(cd /usr/local/lib; rm -f -R asteria)
	@(cd /usr/local/bin; rm -f asteria-run; rm -f asteria-repl; rm -f asteria-run-coffee);
	@(cd /etc/ld.so.conf.d/; rm -f asteria.conf)

test:
	@(cd Test;./testrunner.js;)	

clean:
	@(cd src;make clean;)
	@(cd Run;make clean;)
	@(cd Modules/binary-module;make clean;)
	@(cd Modules/fs-module;make clean;)
	@(cd Modules/readline-module;make clean;)
	@(cd Modules/system-module;make clean;)
