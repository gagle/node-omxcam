"use strict";

var omxcam = require ("../build/Release/addon");

module.exports.yuvPlanes = function (width, height){
  //Wrap the native function in a javascript function, so it will always receive
  //2 parameters
  return omxcam.yuv_planes (width, height);
};

module.exports.yuvPlanesSlice = function (width){
  //Wrap the native function in a javascript function, so it will always receive
  //1 parameter
  return omxcam.yuv_planes_slice (width);
};;