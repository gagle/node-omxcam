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
  omxcam.video_stop (cb);
};

Video.prototype.read = function (cb){
  return omxcam.video_read (cb);
};

Video.prototype.update = function (options, cb){
  if (cb){
    omxcam.video_update (options, cb);
  }else{
    omxcam.video_update (options);
  }
};