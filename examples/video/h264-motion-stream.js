"use strict";

var fs = require ("fs");
var omxcam = require ("../../lib");

var record = function (filename, motion, settings, cb){
  var err = null;
  
  var ws = fs.createWriteStream (filename)
      .on ("error", function (error){
        err = error;
        vs.stop ();
      })
      .on ("finish", function (){
        wsMotion.end ();
      });
  
  var wsMotion = fs.createWriteStream (motion)
      .on ("error", function (error){
        err = error;
        vs.stop ();
      })
      .on ("finish", function (){
        cb (err);
      });

  var vs = omxcam.createVideoStream (settings)
      .on ("error", function (error){
        err = error;
        ws.end ();
        wsMotion.end ();
      })
      .on ("motion", function (buffer){
        wsMotion.write (buffer);
      });

  vs.pipe (ws);
};

record ("video.h264", "motion", { width: 640, height: 480, timeout: 2000,
    h264: { inlineMotionVectors: true } },
    function (error){
      if (error) return console.error (error);
      console.log ("ok");
    });