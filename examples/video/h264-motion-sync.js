"use strict";

var fs = require ("fs");
var omxcam = require ("../../lib");

var record = function (filename, motion, settings, timeout){
  var stop = false;
  var fd = fs.openSync (filename, "w");
  var fdMotion = fs.openSync (motion, "w");
  
  var video = omxcam.video (settings);
  video.start ();
  
  var now = Date.now ();
  var end = now + timeout;
  var buffer;
  
  while (!stop){
    buffer = video.read ();
    fs.writeSync (buffer.isMotionVector ? fdMotion : fd, buffer, 0,
        buffer.length, null);
    if (Date.now () >= end) break;
  }
  
  video.stop ();
  fs.closeSync (fd);
  fs.closeSync (fdMotion);
};

try{
  record ("video.h264", "motion", { width: 640, height: 480,
      h264: { inlineMotionVectors: true } }, 2000);
  console.log ("ok");
}catch (error){
  console.error (error);
}