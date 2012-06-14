exports.checkBaseModules = function () {
	var value = true;
	value &= checkModule('fs');
	value &= checkModule('console');
	value &= checkModule('binary');
	value &= checkModule('shell');
	value &= checkModule('system');
	return value;
}

exports.checkRequireWorks = function() {
	var module = require('fs');
	if(typeof module.file === 'undefined') {
		return false;
	}

	return true;
}

exports.checkBaseModules.description = "checking wether base modules are present";
exports.checkRequireWorks.description = "checking wether require works";
