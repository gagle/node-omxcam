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

/*module.exports.video = function (options){
	return new Video (options);
};

var Video = function (options){
	this._stopped = false;
	this._options = options;
};

Video.prototype.start = function (){
	if (running) throw new Error ("Camera already running");
	running = true;
	
	var me = this;
	
	var onError = function (error){
		running = false;
		me.onError (error);
	};
	
	omxcam.video_start (this._options, onError, this._options.onReady,
			this._options.onData, this._options.onStop);
};

Video.prototype.stop = function (){
	if (this._stopped) return;
	this._stopped = true;
	var me = this;
	omxcam.video_stop (function (){
		running = false;
		me.onEnd ();
	});
};

/*
module.exports.instance = function (options){
	if (camera._running) throw new Error ("Camera already running");
	camera._running = true;
	return camera;
};

var Camera = function (){
	this._running = false;
};

Camera.prototype.still = function (options){
	return new Still (this, options);
};

Camera.prototype.video = function (options){
	return new Video (this, options);
};

Camera.prototype._capture = function (obj, capture, options){
	var me = this;

	//Start capture
	obj._stopped = false;

	var onError = function (){
		me._running = false;
		me.onError ();
	};
	var onEnd = function (){
		me._running = false;
		me.onEnd ();
	};

	capture (options, onError, me.onData, onEnd);
};

//Singleton instance
var camera = new Camera ();

var Still = function (camera, options){
	camera._capture (this, omxcam.still);
};

Still.prototype.cancel = function (){
	if (this._stopped) return;
	this._stopped = true;
	omxcam.cancelStill ();
};*/

