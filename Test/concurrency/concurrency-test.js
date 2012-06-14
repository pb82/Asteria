var s = require('system');

function echo() {
	var s = require('system');
	while(1) {
		var message = concurrent.receive();
		concurrent.send(message.sender, message.value);
	}
}

exports.testSpawn = function () {
	var pid = concurrent.spawn(echo, 'echo-service');
	return pid > 0;
}

exports.testSend = function () {
	concurrent.send('echo-service', 'abcd');
	return concurrent.receive().value === 'abcd';
}

exports.testFutures = function () {
	var f1 = concurrent.sendForProxy('echo-service', 'a');
	var f2 = concurrent.sendForProxy('echo-service', 'b');
	var f3 = concurrent.sendForProxy('echo-service', 'c');
	var f4 = concurrent.sendForProxy('echo-service', 'd');
	var f5 = concurrent.sendForProxy('echo-service', 'e');
	var f6 = concurrent.sendForProxy('echo-service', 'f');
	var f7 = concurrent.sendForProxy('echo-service', 'g');

	var r = f7.value + f6.value + f5.value + f4.value + f3.value + f2.value + f1.value;
	return (r === 'gfedcba');
}

exports.testKill = function () {
	var alive = concurrent.isAlive('echo-service');
	concurrent.kill('echo-service');	
	return alive & !concurrent.isAlive('echo-service');
}

exports.testHardwareConcurrency = function () {
	return concurrent.getHardwareConcurrency() >= 1;
}

exports.testSpawn.description = "testing wether processes can be spawned";
exports.testSend.description = "testing wether data can be sent and received to / from processes";
exports.testFutures.description = "testing wether the 'sendForProxy' function works";
exports.testKill.description = "testing wether processes can be killed";
exports.testHardwareConcurrency.description = "testing wether the getHardwareConcurrency function works";
