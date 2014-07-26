"use strict";

var StillStream = require ("./still-stream");
var VideoStream = require ("./video-stream");
var camera = require ("./camera");
var constants = require ("./constants");
var utils = require ("./utils");

constants (module.exports);

module.exports.video = camera.video;
module.exports.yuvPlanes = utils.yuvPlanes;
module.exports.yuvPlanesSlice = utils.yuvPlanesSlice;

module.exports.createStillStream = function (options){
  return new StillStream (options);
};

module.exports.createVideoStream = function (options){
  return new VideoStream (options);
};