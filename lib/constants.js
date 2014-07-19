"use strict";

var omxcam = require ("../build/Release/addon");

module.exports = function (o){
  omxcam.add_ISO_constants (o);
  omxcam.add_EXPOSURE_constants (o);
  omxcam.add_MIRROR_constants (o);
  omxcam.add_ROTATION_constants (o);
  omxcam.add_METERING_constants (o);
  omxcam.add_WHITE_BALANCE_constants (o);
  omxcam.add_IMAGE_FILTER_constants (o);
}