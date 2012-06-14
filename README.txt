OVERVIEW
--------

Asteria provides a simple scripting environment based on Google's
v8 JavaScript Engine.

At the moment Asteria is an experimental effort by it's author in order
to learn using v8. However it already provides the following 
features:

	* A 'require' based module system as specified by CommonJS.
		(http://www.commonjs.org/).
		
	* Very easy expandability either by writing modules in JavaScript or
		in C++. Have a look at the shipped modules for examples.
		
	* A simple concurrency model based on OS-processes and message passing.
		(see the Examples)
		
	* CoffeeScript compiler included, so the scripts can also be written
		in CoffeeScript without any drawbacks.
		(http://coffeescript.org/)

	* A focus on shellscripting (in JavaScript or CoffeeScript).


Note that:

	* Asteria's concurrency is not exactly light-weight with every 
		spawned process running it's own instance of v8 (and thus
		consuming around four Megabytes of RAM).

	* Implementation of the CommonJS Standard is far from complete.
	
	* The shipped set of libraries is small and the libraries
		are incomplete. But it is of course possible to use CommonJS
		compliant third-party libraries.

	* There are certainly a lot of bugs around ;)
	

INSTALLATION
------------

At the moment Asteria does only run on i686 and x86_64 Linux Systems.
It has been tested on Arch and Debian boxes.

Dependencies (other than the included): 
	* libboost_thread (>= 1.42.0.1)	
	* libreadline-dev (>= 6.1-3)

The current build process is rather primitive:

	* make
	* (sudo) make install
	* make test (optional, runs some simple tests)
	
The binaries of Google v8 and Google protobuf are included. Libraries
will be installed in /usr/local/lib/asteria and executables in
/usr/local/bin. Modules will be installed in /opt/asteria/modules.

UNINSTALL
---------

	* (sudo) make uninstall


IDEAS FOR FUTURE RELEASES
-------------------------

	* ARM support.
	* Foreign Function Interface.
	* Make concurrent.spawn network transparent.
	* Documentation.
