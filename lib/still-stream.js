"use strict";

var util = require ("util");
var stream = require ("stream");
var camera = require ("./camera");

var StillStream = module.exports = function (options){
  stream.Readable.call (this);
  
  var me = this;
  
  process.nextTick (function (){
    try{
      var cam = camera.instance ();
    }catch (error){
      //Camera already capturing
      return me.emit ("error", error);
    }
    
    cam.onError = function (error){
      me.emit ("error", error);
    };
    cam.onData = function (buffer){
      me.push (buffer);
    };
    cam.onEnd = function (){
      me.push (null);
    };
    
    //Start video capture
    me._still = cam.still (options);
  });
};

util.inherits (StillStream, stream.Readable);

StillStream.prototype._read = function (){
  //No-op
};

StillStream.prototype.stop = function (){
  this._still.cancel ();
};