"use strict";

var omxcam = require ("../lib");

module.exports.video = function (req){
  var video = omxcam.video ({
    width: req.width,
    height: req.height,
    format: omxcam.FORMAT_RGB888
  });
  
  var current = 0;
  var buffer;
  var total = req.width*req.height*3*req.frames;
  
  video.start ();
  
  req.onReady ();
  
  while (true){
    buffer = video.read ();
    current += buffer.length;
  
    if (current >= total) break;
  }
  
  req.onStop ();
  
  video.stop ();
};

module.exports.still = function (req){
  
};