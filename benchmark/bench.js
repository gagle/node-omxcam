"use strict";

var omxcam = require ("../lib");

var jpeg = require ("./jpeg");
var rgb = require ("./rgb");
var yuv = require ("./yuv");

/*
Default settings are used. This is not a "real" benchmark since the capture time
depends on various settings like the shutter speed and scene illuminance. This
is just a little benchmark to confirm that the library is working as expected.

Results:

In video mode, the closer to 30fps and 1000ms, the better.
In still mode, the faster, the better, being 1fps the minimum desirable.

set up h264: 326ms
tear down h264: 46ms
video rgb: 17.28fps (1736ms)
video yuv: 20.59fps (1457ms)
*/

var printTimeVideo = function (label, frames){
  console.log (label + ": " + (req.frames/(diff/1000)).toFixed (2) + "fps (" +
      diff + "ms)");
}

var req = {
  width: 640,
  height: 480,
  frames: 30,
  ms: 1000,
  start: function (){
    start = Date.now ();
  },
  stop: function (){
    diff = Date.now () - start;
  }
};

var start;
var diff;
var video;

video = omxcam.video (req);
start = Date.now ();
video.start ();
diff = Date.now () - start;
console.log ("set up h264: " + diff + "ms");

start = Date.now ();
video.stop ();
diff = Date.now () - start;
console.log ("tear down h264: " + diff + "ms");

rgb.video (req);
printTimeVideo ("video rgb", req.frames);

yuv.video (req);
printTimeVideo ("video yuv", req.frames);