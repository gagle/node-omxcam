"use strict";

var fs = require ("fs");
var omxcam = require ("../../lib");

/*var videoStream = omxcam.createVideoStream ({ timeout: 3000 })
    .on ("error", function (error){
      console.error (error);
    });

var writeStream = fs.createWriteStream ("video.h264")
    .on ("error", function (error){
      console.error (error);
    });

videoStream.pipe (writeStream);*/

omxcam.createVideoStream ({ timeout: 3000 })
    .on ("error", function (error){
      console.error (error);
    });
omxcam.createVideoStream ({ timeout: 3000 })
    .on ("error", function (error){
      console.error (error);
    });