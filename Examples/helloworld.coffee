#!/usr/bin/env asteria-run-coffee

###
This script only shows how to invoke the CoffeeScript compiler and
some very basic syntax.
###

sys = require 'system'

greet = (name) -> 
		print 'hello ' + name, '\n'

name = sys.stdin.readLine 'Please enter your name: '
greet name
