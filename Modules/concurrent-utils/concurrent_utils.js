function reduction_applicator() {
  function isArray(o) {
    return Object.prototype.toString.call(o) === '[object Array]';
  }

  var message = concurrent.receive();
  var fun = message.value[0];
  var arg = message.value[1];

  try {
    var result = isArray(arg) ? fun.apply(this, arg) : fun(arg);
    concurrent.send(message.sender, result);
  } catch (ex) {
    concurrent.throw(message.sender, ex);
  }
}

function map_applicator() {
  function isArray(o) {
    return Object.prototype.toString.call(o) === '[object Array]';
  }
  var simple_map = function (list,lambda) {
    var result = [];
    for(index in list) {
      result.push(lambda(list[index]));
    }
    return result;
  }

  var message = concurrent.receive();
  var fun = message.value[0];
  var arg = message.value[1];

  try {
    if(!isArray(arg)) throw "Argument must be of type array";
      
    var result = simple_map(arg,fun);
    concurrent.send(message.sender, result);
  } catch (ex) {
    concurrent.throw(message.sender, ex);
  }
}

function map (list,lambda) {	
  var result = [];
  var workers = [];
	
  // Create worker processes
  for(var index=0; index < concurrent.getHardwareConcurrency(); index++) {
    workers[index] = concurrent.spawn(map_applicator);
  }
	
  // Issue calculations
  var partitions = partition(list, concurrent.getHardwareConcurrency());
  for(var index = 0; index < partitions.length; index++) {
    var nextWorker = workers[index % concurrent.getHardwareConcurrency()];				
    result[index] = concurrent.sendForProxy(nextWorker, [lambda, partitions[index]]);
  }
	
  // Collect results
  for(var index = 0; index < result.length; index++) {
    try {
      result[index] = result[index].value;
    } catch(ex) {
      throw ex;
    }
  }

  return flatten(result);
}

function parallel_reduce(list,initial,lambda) {
  var workers = [];
  var result = [];

  var simple_reduce = function(list,initial,lambda) {
    var init = {init: initial};
    for(var index in list) {
      var item = list[index];
      init.init = lambda(init.init, item);
    }
    return init.init;
  }
  
  // Create worker processes
  for(var index=0; index < concurrent.getHardwareConcurrency(); index++) {
    workers[index] = concurrent.spawn(reduction_applicator);
  }

  // Partition the list, so that every physical core gets an equal part
  // of the list to process.
  var partitions = partition(list, concurrent.getHardwareConcurrency());

  for(var index = 0; index < partitions.length; index++) {
    var nextWorker = workers[index % concurrent.getHardwareConcurrency()];		
    var nextList = partitions[index];		
        
    result[index] = concurrent.sendForProxy(nextWorker, [simple_reduce, [nextList, initial, lambda]]);
  }

  // Collect results
  for(var index = 0; index < result.length; index++) {
    try {
      result[index] = result[index].value;
    } catch(ex) {
      throw ex;
    }
  }
  
  return simple_reduce(result, initial, lambda);
}

function partition(array, parts) {
  if(parts > array.length)
    throw "Number of partitions must be less or equal than the number of elements in the array";
    
  var current = 0;
  var result = [];
  var next = Math.floor(array.length / parts);
  
  for(var index = 0; index < parts; index++) {
    result[index] = [];
  }
  
  for(var index = 0; index < array.length; index++) {
    if (index > 0 && current < (parts-1) && index % next === 0) current ++;    
    result[current].push(array[index]);
  }
  
  return result;
}

function flatten(array) {
  var result = [];
  for(var index in array) {
    result = result.concat(array[index]);
  }
  return result;
}

exports.install = function (context) {
  if(context) {
    context.map = map;
    context.partition = partition;
    context.reduce = parallel_reduce;
  }
}
