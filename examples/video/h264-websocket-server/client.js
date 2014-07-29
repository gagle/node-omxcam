"use strict";

var WebSocket = require ("ws");
var fs = require ("fs");

var startClient = function (filename, address, cb){
  var err;
  var sps = false;
  
  process.on ("SIGINT", function (){
    socket.close ();
  });
  
  var ws = fs.createWriteStream (filename)
      .on ("error", function (error){
        err = error;
        socket.close ();
      })
      .on ("finish", function (){
        if (err) return cb (err);
        cb ();
      });
  
  var socket = new WebSocket (address)
      .on ("error", function (error){
        err = error;
        ws.end ();
      })
      .on ("open", function (){
        console.log ("open");
      })
      .on ("close", function (){
        if (err) return cb (err);
        console.log ("close");
        ws.end ();
      })
      .on ("message", function (data){
        //Wait until an SPS message is received
        if (!sps){
          if ((data[4] & 0x07) !== 0x07) return;
          sps = true;
        }
        ws.write (data);
      });
};

startClient ("video.h264", "ws://localhost:1337", function (error){
  if (error) return console.error (error);
  console.log ("ok");
});