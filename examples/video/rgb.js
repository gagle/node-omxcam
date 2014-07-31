"use strict";

var fs = require ("fs");
var omxcam = require ("../../lib");

var record = function (filename, settings, totalFrames){
  var fd = fs.openSync (filename, "w");
  var video = omxcam.video (settings);
  
  settings = video.settings ();
  
  video.start ();
  
  var buffer;
  var current = 0;
  var length;
  var stop = false;
  
  var total = settings.width*settings.height*3*totalFrames;
  
  while (!stop){
    buffer = video.read ();
    current += buffer.length;
    length = buffer.length;
    
    if (current >= total){
      length -= current - total;
      stop = true;
    }
    
    fs.writeSync (fd, buffer, 0, length, null);
  }
  
  video.stop ();
  fs.closeSync (fd);
};

try{
  record ("video.rgb",
      { format: omxcam.FORMAT_RGB888, width: 640, height: 480,
      h264: { inlineMotionVectors: true } }, 10);
  console.log ("ok");
}catch (error){
  console.error (error);
}