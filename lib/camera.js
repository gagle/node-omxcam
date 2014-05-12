"use strict";

var omxcam = require ("../build/Release/addon");

module.exports.instance = function (options){
  if (camera._capturing) throw new Error ("Already capturing");
  camera._capturing = true;
  return camera;
};

var Still = function (camera, options){
  camera._capture (this, omxcam.still);
};

Still.prototype.cancel = function (){
  if (this._stopped) return;
  this._stopped = true;
  omxcam.cancelStill ();
};

var Video = function (camera, options){
  camera._capture (this, omxcam.startVideo, options);
};

Video.prototype.stop = function (){
  if (this._stopped) return;
  this._stopped = true;
  omxcam.stopVideo ();
};

var Camera = function (){
  this._capturing = false;
};

Camera.prototype.still = function (options){
  return new Still (this, options);
};

Camera.prototype.video = function (options){
  return new Video (this, options);
};

Camera.prototype._capture = function (obj, capture, options){
  //Avoid calling stop before the capture starts
  obj._stopped = true;
  var me = this;
  
  //Start capture
  obj._stopped = false;
  
  var onError = function (){
    me._capturing = false;
    me.onError ();
  };
  var onEnd = function (){
    me._capturing = false;
    me.onEnd ();
  };
  
  capture (options, onError, me.onData, onEnd);
};

//Singleton instance
var camera = new Camera ();