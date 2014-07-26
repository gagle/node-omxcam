"use strict";

var omxcam = require ("../lib");

module.exports.video = function (req){
  var video = omxcam.video ({
    width: req.width,
    height: req.height,
    format: omxcam.FORMAT_YUV420
  });
  
  var current = 0;
  var currentFrames = 0;
  var planes = omxcam.yuvPlanes (req.width, req.height);
  var frameSize = planes.offsetV + planes.lengthV;
  var totalFrames = req.frames;
  var buffer;
  
  video.start ();
  
  req.start ();
  
  while (true){
    buffer = video.read ();
    current += buffer.length;
  
    if (current === frameSize){
      current = 0;
      
      if (++currentFrames === totalFrames) break;
    }
  }
  
  req.stop ();
  
  video.stop ();
};

module.exports.still = function (req){
  
};