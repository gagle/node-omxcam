"use strict";

var fs = require ("fs");
var omxcam = require ("../../lib");

var video = omxcam.createVideoStream ({ timeout: 3000 })
    .on ("error", function (error){
      console.error (error);
    })
    .on ("end", function (){
      console.log ("END");
    });

var file = fs.createWriteStream ("video.h264")
    .on ("error", function (error){
      console.error (error);
    });

video.pipe (file);