"use strict";

var fs = require ("fs");
var omxcam = require ("../../lib");

var record = function (filename, motion, settings, timeout, cb){
  var now;
  var end;
  var stop = false;
  var err;
  
  var video = omxcam.video (settings);

  var ws = fs.createWriteStream (filename)
      .on ("error", function (error){
        stop = true;
        err = error;
      })
      .on ("finish", function (){
        wsMotion.end ();
      });
  
  var wsMotion = fs.createWriteStream (motion)
      .on ("error", function (error){
        stop = true;
        err = error;
      })
      .on ("finish", function (){
        cb (err);
      });
  
  var read = function (error, buffer){
    if (error) return cb (error);
    
    (buffer.isMotionVector ? wsMotion : ws).write (buffer);
    
    now = Date.now ();
    
    if (!stop && now < end){
      video.read (read);
    }else{
      video.stop (function (error){
        if (error) return cb (error);
        ws.end ();
      });
    }
  };

  video.start (function (error){
    if (error) return cb (error);
    
    now = Date.now ();
    end = now + timeout;
    
    video.read (read);
  });
};

record ("video.h264", "motion", { width: 640, height: 480,
    h264: { inlineMotionVectors: true } }, 2000, function (error){
      if (error) return console.error (error);
      console.log ("ok");
    });