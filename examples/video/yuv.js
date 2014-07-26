"use strict";

var fs = require ("fs");
var omxcam = require ("../../lib");

var record = function (filename, settings, totalFrames){
  var fd = fs.openSync (filename, "w");
  var video = omxcam.video (settings);
  
  settings = video.settings ();
  
  var planes = omxcam.yuvPlanes (settings.width, settings.height);
  var planesSlice = omxcam.yuvPlanesSlice (settings.width);
  var frameSize = planes.offsetV + planes.lengthV;
  
  video.start ();
  
  var buffer;
  var slice;
  var yBuffers = [];
  var uBuffers = [];
  var vBuffers = [];
  var current = 0;
  var frames = 0;
  
  while (true){
    buffer = video.read ();
    current += buffer.length;
    
    slice = new Buffer (planesSlice.lengthY);
    buffer.copy (slice, 0, planesSlice.offsetY,
        planesSlice.offsetY + planesSlice.lengthY);
    yBuffers.push (slice);
    
    slice = new Buffer (planesSlice.lengthU);
    buffer.copy (slice, 0, planesSlice.offsetU,
        planesSlice.offsetU + planesSlice.lengthU);
    uBuffers.push (slice);
    
    slice = new Buffer (planesSlice.lengthV);
    buffer.copy (slice, 0, planesSlice.offsetV,
        planesSlice.offsetV + planesSlice.lengthV);
    vBuffers.push (slice);
    
    if (current === frameSize){
      //An entire YUV frame has been received
      current = 0;
      
      buffer = Buffer.concat (yBuffers, planes.lengthY);
      yBuffers.length = 0;
      fs.writeSync (fd, buffer, 0, buffer.length, null);
      
      buffer = Buffer.concat (uBuffers, planes.lengthU);
      uBuffers.length = 0;
      fs.writeSync (fd, buffer, 0, buffer.length, null);
      
      buffer = Buffer.concat (vBuffers, planes.lengthV);
      vBuffers.length = 0;
      fs.writeSync (fd, buffer, 0, buffer.length, null);
      
      if (++frames === totalFrames) break;
    }
  }
  
  video.stop ();
  fs.closeSync (fd);
};

try{
  record ("video.yuv",
      { format: omxcam.FORMAT_YUV420, width: 640, height: 480 }, 10);
  console.log ("ok");
}catch (error){
  console.error (error);
}