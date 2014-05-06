"use strict";

var StillStream = require ("./still-stream");
var VideoStream = require ("./video-stream");

module.exports.createStillStream = function (options){
  return new StillStream (options);
};

module.exports.createVideoStream = function (options){
  return new VideoStream (options);
};