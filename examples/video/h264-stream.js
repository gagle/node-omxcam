"use strict";

var fs = require ("fs");
var omxcam = require ("../../lib");

var record = function (filename, settings, updateSettings, cb){
  var err = null;
  
  var ws = fs.createWriteStream (filename)
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
      })
      .on ("ready", function (){
        setTimeout (function (){
          vs.update (updateSettings);
        }, 1000);
      });

  vs.pipe (ws);
};

record ("video.h264", { width: 640, height: 480, timeout: 2000 },
    { saturation: 100 }, function (error){
      if (error) return console.error (error);
      console.log ("ok");
    });