"use strict";

var util = require ("util");
var stream = require ("stream");
var camera = require ("./camera");

var VideoStream = module.exports = function (options){
  stream.Readable.call (this);
  
  this._video = null;
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
    me._video = cam.video (options);
  });
};

util.inherits (VideoStream, stream.Readable);

VideoStream.prototype._read = function (){
  //No-op
};

VideoStream.prototype.stop = function (){
  this._video.stop ();
};