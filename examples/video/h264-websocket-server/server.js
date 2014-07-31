"use strict";

var WebSocketServer = require ("ws").Server;
var omxcam = require ("../../../lib");

var startServer = function (cameraSettings, serverSettings, cb){
  var video = omxcam.video (cameraSettings);
  var stop = false;
  var started = false;
  
  process.on ("SIGINT", function (){
    stop = true;
  });
  
  var close = function (){
    if (started){
      video.stop (function (){
        wss.close ();
        cb ();
      });
    }else{
      wss.close ();
      cb ();
    }
  };
  
  var wss = new WebSocketServer (serverSettings);
  wss.broadcast = function (data){
    for (var client in this.clients){
      if (stop) return;
      this.clients[client].send (data, function (error){
        if (error){
          wss.close ();
          cb (error);
        }
      });
    }
  };
  wss.on ("listening", function (){
    if (stop) return close ();
    
    video.start (function (error){
      if (error){
        wss.close ();
        return cb (error);
      }
      
      started = true;
      if (stop) return close ();
      
      var read = function (error, buffer){
        if (error){
          wss.close ();
          return cb (error);
        }
        
        if (stop) return close ();
        
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

//The h264 streaming must include I-frames with SPS/PPS headers, otherwise,
//the clients cannot capture random sequences of frames from an h264 stream
//An IDR period of 10 means that after 9 frames, 1 is an IDR frame.
startServer ({ width: 640, height: 480,
    h264: { idrPeriod: 10, inlineHeaders: true } }, { port: 1337 },
    function (error){
      if (error) return console.error (error);
      console.log ("ok");
    });