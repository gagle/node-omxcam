"use strict";

var WebSocketServer = require ("ws").Server;
var omxcam = require ("../../../lib");

var startServer = function (cameraSettings, serverSettings, cb){
  var video = omxcam.video (cameraSettings);
  var stop = false;
  
  process.on ("SIGINT", function (){
    stop = true;
    wss.close ();
  });
  
  var wss = new WebSocketServer (serverSettings);
  wss.broadcast = function (data){
    for (var client in this.clients){
      this.clients[client].send (data, function (error){
        if (error){
          wss.close ();
          cb (error);
        }
      });
    }
  };
  wss.on ("listening", function (){
    video.start (function (error){
      if (error){
        wss.close ();
        return cb (error);
      }
      
      var read = function (error, buffer){
        if (error){
          wss.close ();
          return cb (error);
        }
        
        wss.broadcast (buffer);
        
        if (!stop){
          video.read (read);
        }else{
          video.stop (cb);
        }
      };
      
      video.read (read);
    });
  });
};

startServer ({ width: 640, height: 480 }, { port: 1337 }, function (error){
  if (error) return console.error (error);
  console.log ("ok");
});