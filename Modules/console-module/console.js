/*
 * Implementation of the CommonJS console module
 * 
 */
 
var sprintf = require("sprintf").sprintf;

exports.log = function() {
	print(sprintf.apply(this, arguments));
}

exports.debug = function() {
	print(sprintf("DEBUG: %s", sprintf.apply(this, arguments)));
}

exports.info = function() {
	print(sprintf("INFO: %s", sprintf.apply(this, arguments)));
}

exports.warn = function() {
	print(sprintf("WARN: %s", sprintf.apply(this, arguments)));
}

exports.error = function() {
	print(sprintf("ERROR: %s", sprintf.apply(this, arguments)));
}

exports.assert = function(condition, msg) {
	if(!(condition)) {
		throw new Error(typeof msg === "undefined" 
			? sprintf("Assertion failed")
			:	sprintf("Assertion failed: %s", msg)
		);
	}
}

exports.dir = function(object) {
	var output = "";
	for(property in object) {
		output += property.toString();
		output += "\r\n";
	}	
	print(output);
}
