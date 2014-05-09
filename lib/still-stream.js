"use strict";

var util = require ("util");
var stream = require ("stream");
var omxcam = require ("../build/Release/omxcam");

var StillStream = module.exports = function (options, globalState){
  stream.Readable.call (this);
  
  if (globalState.running) throw new Error ("The camera is already running");
  
  globalState.running = true;
  this._globalState = globalState;
  
  process.nextTick (function (){
    omxcam.still (options, function (error){
      me.emit ("error", error);
    });
  });
};

util.inherits (StillStream, stream.Readable);

StillStream.prototype._read = function (){
  //no-op
};

StillStream.prototype.cancel = function (){
  omxcam.cancelStill (function (error){
    me.emit ("error", error);
  });
};