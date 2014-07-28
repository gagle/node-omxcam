"use strict";

var fs = require ("fs");
var omxcam = require ("../../lib");

var record = function (filename, settings, updateSettings, timeout, cb){
  var now;
  var end;
  var update;
  var updated = false;
  var stop = false;
  
  var video = omxcam.video (settings);

  var ws = fs.createWriteStream (filename)
      .on ("error", function (error){
        stop = true;
      })
      .on ("finish", function (){
        cb ();
      });
  
  var read = function (error, buffer){
    if (error) return cb (error);
    
    ws.write (buffer);
    now = Date.now ();
    
    if (!updated && now >= update){
      updated = true;
      return video.update (updateSettings, function (error){
        if (error) return cb (error);
        video.read (read);
      });
      return;
    }
    
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
    update = now + ~~(timeout/2);
    
    video.read (read);
  });
};

record ("video.h264", { width: 640, height: 480 }, { saturation: 100 }, 2000,
    function (error){
      if (error) return console.error (error);
      console.log ("ok");
    });