"use strict";

var fs = require ("fs");
var omxcam = require ("../../lib");

var record = function (filename, settings, timeout){
  var stop = false;
  var fd = fs.openSync (filename, "w");
  
  var video = omxcam.video (settings);
  video.start ();
  
  var now = Date.now ();
  var end = now + timeout;
  var update = now + ~~(timeout/2);
  var updated = false;
  var buffer;
  
  while (!stop){
    buffer = video.read ();
    fs.writeSync (fd, buffer, 0, buffer.length, null);
    now = Date.now ();
    
    if (!updated && now >= update){
      updated = true;
      video.update ({ saturation: 100 })
    }
    if (now >= end) break;
  }
  
  video.stop ();
  fs.closeSync (fd);
};

try{
  record ("video.h264", { width: 640, height: 480,
      roi: { top: 0.5, left: 0.5, width: 0.5, height: 0.5 } }, 2000);
  console.log ("ok");
}catch (error){
  console.error (error);
}

/*var video = omxcam.video ({ width: 640, height: 480 });
video.start ();*/