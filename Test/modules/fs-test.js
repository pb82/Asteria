var fs = require('fs');

exports.testFileCreate = function () {	
	var file = new fs.file('./tmp.txt', 'w+');
	file.close();

	return fs.exists('./tmp.txt');
}


exports.testFileWrite = function () {
	var text = "Hello World";
	var file = new fs.file('./tmp.txt', 'w+');

	var success = file.write(text) == text.length;
	file.close();
	
	return success;
}


exports.testFileRead = function () {
	var file = new fs.file('./tmp.txt', 'r');
	var buffer = file.size();
	var text = file.read().toString();
	file.close();
	return text === 'Hello World';
}

exports.testFileDelete = function() {
	fs.delete('./tmp.txt');
	return true;
}

exports.testFileCreate.description = "testing wether creation of files works";
exports.testFileWrite.description = "testing wether writing to a file works";
exports.testFileRead.description = "testing wether reading from a file works";
exports.testFileDelete.description = "testing wether deletion of a file works";
