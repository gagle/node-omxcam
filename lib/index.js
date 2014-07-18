"use strict";

var StillStream = require ("./still-stream");
var VideoStream = require ("./video-stream");
var camera = require ("./camera");

module.exports.video = camera.video;

module.exports.createStillStream = function (options){
	return new StillStream (options);
};

module.exports.createVideoStream = function (options){
	return new VideoStream (options);
};