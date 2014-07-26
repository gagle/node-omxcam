"use strict";

var omxcam = require ("../build/Release/addon");

module.exports.video = function (options){
  return new Video (options);
};

var Video = function (options){
  this._options = options;
};


Video.prototype.start = function (cb){
  omxcam.video_start (this._options, cb);
};

Video.prototype.stop = function (cb){
  //Wrap the native function in a javascript function, so it will always receive
  //1 parameter
  omxcam.video_stop (cb);
};

Video.prototype.read = function (cb){
  //Wrap the native function in a javascript function, so it will always receive
  //1 parameter
  return omxcam.video_read (cb);
};

Video.prototype.update = function (options, cb){
  //Wrap the native function in a javascript function, so it will always receive
  //2 parameters
  omxcam.video_update (options, cb);
};

Video.prototype.settings = function (){
  //Wrap the native function in a javascript function, so it will always receive
  //2 parameters
  return omxcam.video_settings (this._options);
};