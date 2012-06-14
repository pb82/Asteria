var sys = require('system');

function execute(command, arguments) {
	return sys.execute([command].concat(Array.prototype.slice.call(arguments)));
}

// Report status
// -----------------------------------------
exports.ls = function () {
	return execute('ls', arguments);
}

exports.df = function () {
	return execute('df', arguments);
}

exports.du = function () {
	return execute('du', arguments);
}

exports.whoami = function () {
	return execute('whoami', arguments);
}

exports.who = function () {
	return execute('who', arguments);
}

exports.which = function () {
	return execute('which', arguments);
}

exports.whereis = function () {
	return execute('whereis', arguments);
}

exports.whatis = function () {
	return execute('whatis', arguments);
}

exports.uname = function () {
	return execute('uname', arguments);
}

exports.cal = function () {
	return execute('cal', arguments);
}

exports.cmp = function () {
	return execute('cmp', arguments);
}

exports.date = function () {
	return execute('date', arguments);
}

exports.tty = function () {
	return execute('tty', arguments);
}
// -----------------------------------------

// Manipulate directories and files
// -----------------------------------------
exports.cd = function (path) {
	return sys.setCwd(path);
}

exports.mkdir = function () {
	return execute('mkdir', arguments);
}

exports.rm = function () {
	return execute('rm', arguments);
}

exports.mv = function () {
	return execute('mv', arguments);
}

exports.cp = function () {
	return execute('cp', arguments);
}

exports.chown = function () {
	return execute('chown', arguments);
}

exports.chgrp = function () {
	return execute('chgrp', arguments);
}

exports.dd = function () {
	return execute('dd', arguments);
}
// -----------------------------------------

// Output & Filter
// -----------------------------------------
exports.cat = function () {
	return execute('cat', arguments);
}

exports.echo = function () {
	return execute('echo', arguments);
}

exports.grep = function (options, arguments) {
	return sys.popen2(['grep'].concat(options), arguments);
}

exports.egrep = function (options, arguments) {
	return sys.popen2(['egrep'].concat(options), arguments);
}

exports.fgrep = function (options, arguments) {
	return sys.popen2(['fgrep'].concat(options), arguments);
}
// -----------------------------------------

// Aux
// -----------------------------------------
exports.clear = function () {
	return sys.system('clear');
}

exports.sum = function () {
	if(arguments.length == 2) {
			var options = arguments[0];
			var args = arguments[1];
			return sys.popen2(['sum'].concat(options), args);
	} else {
		return sys.popen2('sum', arguments[0].toString());
	}
}
// -----------------------------------------
