"use strict";

var util = require ("util");
var stream = require ("stream");
var camera = require ("./camera");

var VideoStream = module.exports = function (options){
  stream.Readable.call (this);

  var me = this;
  this._stop = false;
  this._video = null;
  this._timeout = null;
  this._update = null;
  
  process.nextTick (function (){
    //The user stopped the stream before initialization
    if (me._stop) return me.emit ("end");
    
    me._video = camera.video (options);
    
    me._video.start (function (error){
      if (error) return me.emit ("error", error);
      
      me.emit ("ready");
      
      if (options.timeout){
        me._timeout = setTimeout (function (){
          me._stop = true;
        }, options.timeout);
      }
      
      me._readVideoBuffer ();
    });
  });
};

util.inherits (VideoStream, stream.Readable);

VideoStream.prototype._read = function (){
  //No-op
};

VideoStream.prototype._readVideoBuffer = function (){
  //Critical section, this function needs to be as fast as possible
  
  var me = this;
  
  if (this._stop){
    return this._video.stop (function (error){
      if (error) return me.emit ("error", error);
      me.push (null);
    });
  }
  
  if (this._update){
    this._video.update (this._update, function (error){
      if (error) return me.emit ("error", error);
      me._readVideoBuffer ();
    });
    this._update = null;
    return;
  }
  
  this._video.read (function (error, buffer){
    if (error) return me.emit ("error", error);
    me.push (buffer);
    me._readVideoBuffer ();
  });
};

VideoStream.prototype.stop = function (){
  this._stop = true;
  clearTimeout (this._timer);
};

VideoStream.prototype.update = function (settings){
  if (this._stop) return;
  this._update = settings;
};