#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "omxcam.h"

using namespace v8;
using namespace node;

typedef struct {
  int sharpness;
  int contrast;
  int brightness;
  int saturation;
  int shutter_speed;
  int iso;
  int exposure;
  int exposure_compensation;
  int mirror;
  int rotation;
  int color_effects;
  int color_denoise;
  int metering;
  int white_balance;
  int image_filter;
  int roi;
  int frame_stabilisation;
} video_boolean_settings_t;

typedef struct {
  uv_work_t req;
  omxcam_buffer_t buffer;
  omxcam_video_settings_t settings;
  omxcam_bool is_motion_vector;
  video_boolean_settings_t boolean_settings;
  omxcam_errno error;
  Persistent<Function> cb;
} video_buffer_baton_t;

#define THROW_BAD_ARGUMENTS                                                    \
  return ThrowException (Exception::Error (String::New ("bad arguments")));

#define CONSTANT_VALUES(name, _)                                               \
  obj->Set (String::New (#name), Int32::New (OMXCAM_ ## name));

#define DEFINE_CONSTANTS(name)                                                 \
  Handle<Value> add_ ## name ## _constants (const Arguments& args){            \
    Local<Object> obj = args[0]->ToObject ();                                  \
    OMXCAM_ ## name ## _MAP (CONSTANT_VALUES);                                 \
    HandleScope scope;                                                         \
    return scope.Close (Undefined ());                                         \
  }

DEFINE_CONSTANTS (ISO)
DEFINE_CONSTANTS (EXPOSURE)
DEFINE_CONSTANTS (MIRROR)
DEFINE_CONSTANTS (ROTATION)
DEFINE_CONSTANTS (METERING)
DEFINE_CONSTANTS (WHITE_BALANCE)
DEFINE_CONSTANTS (IMAGE_FILTER)
DEFINE_CONSTANTS (DRC)
DEFINE_CONSTANTS (H264_AVC_PROFILE)

Handle<Value> add_auto_constants (const Arguments& args){
  Local<Object> obj = args[0]->ToObject ();
  CONSTANT_VALUES (SHUTTER_SPEED_AUTO, _)
  CONSTANT_VALUES (JPEG_THUMBNAIL_WIDTH_AUTO, _)
  CONSTANT_VALUES (JPEG_THUMBNAIL_HEIGHT_AUTO, _)
  CONSTANT_VALUES (H264_IDR_PERIOD_OFF, _)
  CONSTANT_VALUES (H264_QP_OFF, _)
  HandleScope scope;
  return scope.Close (Undefined ());
}

Handle<Value> add_format_constants (const Arguments& args){
  Local<Object> obj = args[0]->ToObject ();
  CONSTANT_VALUES (FORMAT_RGB888, _)
  CONSTANT_VALUES (FORMAT_RGBA8888, _)
  CONSTANT_VALUES (FORMAT_YUV420, _)
  CONSTANT_VALUES (FORMAT_H264, _)
  CONSTANT_VALUES (FORMAT_JPEG, _)
  HandleScope scope;
  return scope.Close (Undefined ());
}

Handle<Value> yuv_planes (const Arguments& args){
  if (!args[0]->IsUint32 () || !args[1]->IsUint32 ()) THROW_BAD_ARGUMENTS
  
  omxcam_yuv_planes_t planes;
  omxcam_yuv_planes (args[0]->Uint32Value (), args[1]->Uint32Value (), &planes);
  
  Local<Object> obj = Object::New ();
  obj->Set (String::New ("offsetY"), Uint32::New (planes.offset_y));
  obj->Set (String::New ("lengthY"), Uint32::New (planes.length_y));
  obj->Set (String::New ("offsetU"), Uint32::New (planes.offset_u));
  obj->Set (String::New ("lengthU"), Uint32::New (planes.length_u));
  obj->Set (String::New ("offsetV"), Uint32::New (planes.offset_v));
  obj->Set (String::New ("lengthV"), Uint32::New (planes.length_v));
  
  HandleScope scope;
  return scope.Close (obj);
}

Handle<Value> yuv_planes_slice (const Arguments& args){
  if (!args[0]->IsUint32 ()) THROW_BAD_ARGUMENTS
  
  omxcam_yuv_planes_t planes;
  omxcam_yuv_planes_slice (args[0]->Uint32Value (), &planes);
  
  Local<Object> obj = Object::New ();
  obj->Set (String::New ("offsetY"), Uint32::New (planes.offset_y));
  obj->Set (String::New ("lengthY"), Uint32::New (planes.length_y));
  obj->Set (String::New ("offsetU"), Uint32::New (planes.offset_u));
  obj->Set (String::New ("lengthU"), Uint32::New (planes.length_u));
  obj->Set (String::New ("offsetV"), Uint32::New (planes.offset_v));
  obj->Set (String::New ("lengthV"), Uint32::New (planes.length_v));
  
  HandleScope scope;
  return scope.Close (obj);
}

omxcam_bool bool_to_omxcam (bool value){
  return value ? OMXCAM_TRUE : OMXCAM_FALSE;
}

int camera_obj_to_settings (
    Local<Object> obj,
    omxcam_camera_settings_t* settings,
    video_boolean_settings_t* boolean_settings){
  Local<Value> opt;
  Local<Object> obj_nested;
  
  if (!boolean_settings){
    opt = obj->Get (String::NewSymbol ("width"));
    if (!opt->IsUndefined ()){
      if (!opt->IsUint32 ()) return -1;
      settings->width = opt->Uint32Value ();
    }
  
    opt = obj->Get (String::NewSymbol ("height"));
    if (!opt->IsUndefined ()){
      if (!opt->IsUint32 ()) return -1;
      settings->height = opt->Uint32Value ();
    }
  }
  
  opt = obj->Get (String::NewSymbol ("sharpness"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->sharpness = opt->Int32Value ();
    if (boolean_settings) boolean_settings->sharpness = 1;
  }
  
  opt = obj->Get (String::NewSymbol ("contrast"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->contrast = opt->Int32Value ();
    if (boolean_settings) boolean_settings->contrast = 1;
  }
  
  opt = obj->Get (String::NewSymbol ("brightness"));
  if (!opt->IsUndefined ()){
    if (!opt->IsUint32 ()) return -1;
    settings->brightness = opt->Uint32Value ();
    if (boolean_settings) boolean_settings->brightness = 1;
  }
  
  opt = obj->Get (String::NewSymbol ("saturation"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->saturation = opt->Int32Value ();
    if (boolean_settings) boolean_settings->saturation = 1;
  }
  
  if (!boolean_settings){
    opt = obj->Get (String::NewSymbol ("shutterSpeed"));
    if (!opt->IsUndefined ()){
      if (!opt->IsUint32 ()) return -1;
      settings->shutter_speed = opt->Uint32Value ();
    }
  }
  
  opt = obj->Get (String::NewSymbol ("iso"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->iso = (omxcam_iso)opt->Int32Value ();
    if (boolean_settings) boolean_settings->iso = 1;
  }
  
  opt = obj->Get (String::NewSymbol ("exposure"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->exposure = (omxcam_exposure)opt->Int32Value ();
    if (boolean_settings) boolean_settings->exposure = 1;
  }
  
  opt = obj->Get (String::NewSymbol ("exposureCompensation"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->exposure_compensation = opt->Int32Value ();
    if (boolean_settings) boolean_settings->exposure_compensation = 1;
  }
  
  opt = obj->Get (String::NewSymbol ("mirror"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->mirror = (omxcam_mirror)opt->Int32Value ();
    if (boolean_settings) boolean_settings->mirror = 1;
  }
  
  opt = obj->Get (String::NewSymbol ("rotation"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->rotation = (omxcam_rotation)opt->Int32Value ();
    if (boolean_settings) boolean_settings->rotation = 1;
  }
  
  opt = obj->Get (String::NewSymbol ("colorEffects"));
  if (!opt->IsUndefined ()){
    if (!opt->IsObject ()) return -1;
    if (boolean_settings) boolean_settings->color_effects = 1;
    
    settings->color_effects.enabled = OMXCAM_TRUE;
    
    obj_nested = opt->ToObject ();
    
    opt = obj_nested->Get (String::NewSymbol ("u"));
    if (!opt->IsUndefined ()){
      if (!opt->IsUint32 ()) return -1;
      settings->color_effects.u = opt->Uint32Value ();
    }
    
    opt = obj_nested->Get (String::NewSymbol ("v"));
    if (!opt->IsUndefined ()){
      if (!opt->IsUint32 ()) return -1;
      settings->color_effects.v = opt->Uint32Value ();
    }
  }
  
  if (!boolean_settings){
    opt = obj->Get (String::NewSymbol ("colorDenoise"));
    if (!opt->IsUndefined ()){
      if (!opt->IsBoolean ()) return -1;
      settings->color_denoise = bool_to_omxcam (opt->BooleanValue ());
    }
  }
  
  opt = obj->Get (String::NewSymbol ("metering"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->metering = (omxcam_metering)opt->Int32Value ();
    if (boolean_settings) boolean_settings->metering = 1;
  }
  
  opt = obj->Get (String::NewSymbol ("whiteBalance"));
  if (!opt->IsUndefined ()){
    if (!opt->IsObject ()) return -1;
    if (boolean_settings) boolean_settings->white_balance = 1;
    
    obj_nested = opt->ToObject ();
    
    opt = obj_nested->Get (String::NewSymbol ("mode"));
    if (!opt->IsUndefined ()){
      if (!opt->IsInt32 ()) return -1;
      settings->white_balance.mode = (omxcam_white_balance)opt->Int32Value ();
    }
    
    opt = obj_nested->Get (String::NewSymbol ("redGain"));
    if (!opt->IsUndefined ()){
      if (!opt->IsUint32 ()) return -1;
      settings->white_balance.red_gain = opt->Uint32Value ();
    }
    
    opt = obj_nested->Get (String::NewSymbol ("blueGain"));
    if (!opt->IsUndefined ()){
      if (!opt->IsUint32 ()) return -1;
      settings->white_balance.blue_gain = opt->Uint32Value ();
    }
  }
  
  opt = obj->Get (String::NewSymbol ("imageFilter"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->image_filter = (omxcam_image_filter)opt->Int32Value ();
    if (boolean_settings) boolean_settings->image_filter = 1;
  }
  
  opt = obj->Get (String::NewSymbol ("drc"));
  if (!opt->IsUndefined ()){
    if (!opt->IsInt32 ()) return -1;
    settings->drc = (omxcam_drc)opt->Int32Value ();
  }
  
  opt = obj->Get (String::NewSymbol ("roi"));
  if (!opt->IsUndefined ()){
    if (!opt->IsObject ()) return -1;
    if (boolean_settings) boolean_settings->roi = 1;
    
    obj_nested = opt->ToObject ();
    
    opt = obj_nested->Get (String::NewSymbol ("top"));
    if (!opt->IsUndefined ()){
      if (!opt->IsNumber ()) return -1;
      settings->roi.top = (uint32_t)(opt->NumberValue ()*100);
    }
    
    opt = obj_nested->Get (String::NewSymbol ("left"));
    if (!opt->IsUndefined ()){
      if (!opt->IsNumber ()) return -1;
      settings->roi.left = (uint32_t)(opt->NumberValue ()*100);
    }
    
    opt = obj_nested->Get (String::NewSymbol ("width"));
    if (!opt->IsUndefined ()){
      if (!opt->IsNumber ()) return -1;
      settings->roi.width = (uint32_t)(opt->NumberValue ()*100);
    }
    
    opt = obj_nested->Get (String::NewSymbol ("height"));
    if (!opt->IsUndefined ()){
      if (!opt->IsNumber ()) return -1;
      settings->roi.height = (uint32_t)(opt->NumberValue ()*100);
    }
  }
  
  if (!boolean_settings){
    opt = obj->Get (String::NewSymbol ("framerate"));
    if (!opt->IsUndefined ()){
      if (!opt->IsUint32 ()) return -1;
      settings->framerate = opt->Uint32Value ();
    }
  }
  
  opt = obj->Get (String::NewSymbol ("frameStabilisation"));
  if (!opt->IsUndefined ()){
    if (!opt->IsBoolean ()) return -1;
    settings->frame_stabilisation = bool_to_omxcam (opt->BooleanValue ());
    if (boolean_settings) boolean_settings->frame_stabilisation = 1;
  }
  
  return 0;
}

int h264_obj_to_settings (Local<Object> obj, omxcam_h264_settings_t* settings){
  Local<Value> opt;
  Local<Object> obj_nested;
  
  opt = obj->Get (String::NewSymbol ("h264"));
  if (!opt->IsUndefined ()){
    if (!opt->IsObject ()) return -1;
    
    obj = opt->ToObject ();
    
    opt = obj->Get (String::NewSymbol ("bitrate"));
    if (!opt->IsUndefined ()){
      if (!opt->IsUint32 ()) return -1;
      settings->bitrate = opt->Uint32Value ();
    }
    
    opt = obj->Get (String::NewSymbol ("idrPeriod"));
    if (!opt->IsUndefined ()){
      if (!opt->IsUint32 ()) return -1;
      settings->idr_period = opt->Uint32Value ();
    }
    
    opt = obj->Get (String::NewSymbol ("sei"));
    if (!opt->IsUndefined ()){
      if (!opt->IsBoolean ()) return -1;
      settings->sei = bool_to_omxcam (opt->BooleanValue ());
    }
    
    opt = obj->Get (String::NewSymbol ("eede"));
    if (!opt->IsUndefined ()){
      if (!opt->IsObject ()) return -1;
      
      obj_nested = opt->ToObject ();
      
      opt = obj_nested->Get (String::NewSymbol ("enabled"));
      if (!opt->IsUndefined ()){
        if (!opt->IsBoolean ()) return -1;
        settings->eede.enabled = bool_to_omxcam (opt->BooleanValue ());
      }
      
      opt = obj_nested->Get (String::NewSymbol ("lossRate"));
      if (!opt->IsUndefined ()){
        if (!opt->IsNumber ()) return -1;
        settings->eede.loss_rate = (uint32_t)(opt->NumberValue ()*100);
      }
    }
    
    opt = obj->Get (String::NewSymbol ("profile"));
    if (!opt->IsUndefined ()){
      if (!opt->IsInt32 ()) return -1;
      settings->profile = (omxcam_avc_profile)opt->Int32Value ();
    }
    
    opt = obj->Get (String::NewSymbol ("inlineHeaders"));
    if (!opt->IsUndefined ()){
      if (!opt->IsBoolean ()) return -1;
      settings->inline_headers = bool_to_omxcam (opt->BooleanValue ());
    }
    
    opt = obj->Get (String::NewSymbol ("inlineMotionVectors"));
    if (!opt->IsUndefined ()){
      if (!opt->IsBoolean ()) return -1;
      settings->inline_motion_vectors = bool_to_omxcam (opt->BooleanValue ());
    }
  }
  
  return 0;
}

int video_obj_to_settings (
    Local<Object> obj,
    omxcam_video_settings_t* settings){
  if (camera_obj_to_settings (obj, &settings->camera, 0)) return -1;
  if (h264_obj_to_settings (obj, &settings->h264)) return -1;
  
  Local<Value> opt = obj->Get (String::NewSymbol ("format"));
  if (!opt->IsUndefined ()){
    settings->format = (omxcam_format)opt->Int32Value ();
  }
  
  return 0;
}

void camera_settings_to_obj (
    omxcam_camera_settings_t* settings,
    Local<Object> obj){
  Local<Object> obj_nested;
  
  obj->Set (String::NewSymbol ("width"), Uint32::New (settings->width));
  obj->Set (String::NewSymbol ("height"), Uint32::New (settings->height));
  obj->Set (String::NewSymbol ("sharpness"), Int32::New (settings->sharpness));
  obj->Set (String::NewSymbol ("contrast"), Int32::New (settings->contrast));
  obj->Set (String::NewSymbol ("brightness"),
      Uint32::New (settings->brightness));
  obj->Set (String::NewSymbol ("saturation"),
      Int32::New (settings->saturation));
  obj->Set (String::NewSymbol ("shutterSpeed"),
      Uint32::New (settings->shutter_speed));
  obj->Set (String::NewSymbol ("iso"), Int32::New (settings->iso));
  obj->Set (String::NewSymbol ("exposure"), Int32::New (settings->exposure));
  obj->Set (String::NewSymbol ("exposureCompensation"),
      Int32::New (settings->exposure_compensation));
  obj->Set (String::NewSymbol ("mirror"), Int32::New (settings->mirror));
  obj->Set (String::NewSymbol ("rotation"), Int32::New (settings->rotation));
  
  obj_nested = Object::New ();
  obj_nested->Set (String::NewSymbol ("enabled"),
      Boolean::New (settings->color_effects.enabled));
  obj_nested->Set (String::NewSymbol ("u"),
      Uint32::New (settings->color_effects.u));
  obj_nested->Set (String::NewSymbol ("v"),
      Uint32::New (settings->color_effects.v));
  obj->Set (String::NewSymbol ("colorEffects"), obj_nested);
  
  obj->Set (String::NewSymbol ("colorDenoise"),
      Boolean::New (settings->color_denoise));
  obj->Set (String::NewSymbol ("metering"), Int32::New (settings->metering));
  
  obj_nested = Object::New ();
  obj_nested->Set (String::NewSymbol ("mode"),
      Boolean::New (settings->white_balance.mode));
  obj_nested->Set (String::NewSymbol ("redGain"),
      Uint32::New (settings->white_balance.red_gain));
  obj_nested->Set (String::NewSymbol ("blueGain"),
      Uint32::New (settings->white_balance.blue_gain));
  obj->Set (String::NewSymbol ("whiteBalance"), obj_nested);
  
  obj->Set (String::NewSymbol ("imageFilter"),
      Int32::New (settings->image_filter));
  obj->Set (String::NewSymbol ("drc"), Int32::New (settings->drc));
  
  obj_nested = Object::New ();
  obj_nested->Set (String::NewSymbol ("top"),
      Number::New (settings->roi.top/100.0));
  obj_nested->Set (String::NewSymbol ("left"),
      Number::New (settings->roi.left/100.0));
  obj_nested->Set (String::NewSymbol ("width"),
      Number::New (settings->roi.width/100.0));
  obj_nested->Set (String::NewSymbol ("height"),
      Number::New (settings->roi.height/100.0));
  obj->Set (String::NewSymbol ("roi"), obj_nested);
  
  obj->Set (String::NewSymbol ("framerate"), Uint32::New (settings->framerate));
  obj->Set (String::NewSymbol ("frameStabilisation"),
      Boolean::New (settings->frame_stabilisation));
}

void h264_settings_to_obj (
    omxcam_h264_settings_t* settings,
    Local<Object> obj){
  Local<Object> obj_nested;
  Local<Object> obj_nested_2;
  
  obj_nested = Object::New ();
  obj_nested->Set (String::NewSymbol ("bitrate"),
      Uint32::New (settings->bitrate));
  obj_nested->Set (String::NewSymbol ("idrPeriod"),
      Uint32::New (settings->idr_period));
  obj_nested->Set (String::NewSymbol ("sei"), Boolean::New (settings->sei));
  
  obj_nested_2 = Object::New ();
  obj_nested_2->Set (String::NewSymbol ("enabled"),
      Boolean::New (settings->eede.enabled));
  obj_nested_2->Set (String::NewSymbol ("lossRate"),
      Number::New (settings->eede.loss_rate/100.0));
  obj_nested->Set (String::NewSymbol ("eede"), obj_nested_2);
  
  obj_nested->Set (String::NewSymbol ("profile"),
      Int32::New (settings->profile));
  obj_nested->Set (String::NewSymbol ("inlineHeaders"),
      Boolean::New (settings->inline_headers));
  obj_nested->Set (String::NewSymbol ("inlineMotionVectors"),
      Boolean::New (settings->inline_motion_vectors));
  
  obj->Set (String::NewSymbol ("h264"), obj_nested);
}

void video_settings_to_obj (
    omxcam_video_settings_t* settings,
    Local<Object> obj){
  camera_settings_to_obj (&settings->camera, obj);
  h264_settings_to_obj (&settings->h264, obj);
  
  obj->Set (String::NewSymbol ("format"), Int32::New (settings->format));
}

Local<Object> create_buffer (omxcam_buffer_t* buffer){
  Local<Object> global = Context::GetCurrent ()->Global ();

  Buffer* slow_buffer = Buffer::New (buffer->length);
  memcpy (Buffer::Data (slow_buffer), buffer->data, buffer->length);

  Local<Function> buffer_constructor =
      Local<Function>::Cast (global->Get (String::New ("Buffer")));
  Handle<Value> constructor_args[3] = {
    slow_buffer->handle_,
    Uint32::New (buffer->length),
    Uint32::New (0)
  };
  
  HandleScope scope;
  return scope.Close (buffer_constructor->NewInstance (3, constructor_args));
}

int video_update_task (omxcam_camera_settings_t* settings,
    video_boolean_settings_t* boolean_settings){
  if (boolean_settings->sharpness &&
      omxcam_video_update_sharpness (settings->sharpness)){
    return -1;
  }
  
  if (boolean_settings->contrast &&
      omxcam_video_update_contrast (settings->contrast)){
    return -1;
  }
  
  if (boolean_settings->brightness &&
      omxcam_video_update_brightness (settings->brightness)){
    return -1;
  }
  
  if (boolean_settings->saturation &&
      omxcam_video_update_saturation (settings->saturation)){
    return -1;
  }
  
  if (boolean_settings->iso && omxcam_video_update_iso (settings->iso)){
    return -1;
  }
  
  if (boolean_settings->exposure &&
      omxcam_video_update_exposure (settings->exposure)){
    return -1;
  }
  
  if (boolean_settings->exposure_compensation &&
      omxcam_video_update_contrast (settings->exposure_compensation)){
    return -1;
  }
  
  if (boolean_settings->mirror &&
      omxcam_video_update_mirror (settings->mirror)){
    return -1;
  }
  
  if (boolean_settings->rotation &&
      omxcam_video_update_rotation (settings->rotation)){
    return -1;
  }
  
  if (boolean_settings->color_effects &&
      omxcam_video_update_color_effects (&settings->color_effects)){
    return -1;
  }
  
  if (boolean_settings->metering &&
      omxcam_video_update_metering (settings->metering)){
    return -1;
  }
  
  if (boolean_settings->white_balance &&
      omxcam_video_update_white_balance (&settings->white_balance)){
    return -1;
  }
  
  if (boolean_settings->image_filter &&
      omxcam_video_update_image_filter (settings->image_filter)){
    return -1;
  }
  
  if (boolean_settings->roi && omxcam_video_update_roi (&settings->roi)){
    return -1;
  }
  
  if (boolean_settings->frame_stabilisation &&
      omxcam_video_update_frame_stabilisation (settings->frame_stabilisation)){
    return -1;
  }
  
  return 0;
}

void video_start_async (uv_work_t* req){
  video_buffer_baton_t* baton = (video_buffer_baton_t*)req->data;
  omxcam_video_start_npt (&baton->settings);
  baton->error = omxcam_last_error ();
}

void video_stop_async (uv_work_t* req){
  video_buffer_baton_t* baton = (video_buffer_baton_t*)req->data;
  omxcam_video_stop_npt ();
  baton->error = omxcam_last_error ();
}

void video_update_async (uv_work_t* req){
  video_buffer_baton_t* baton = (video_buffer_baton_t*)req->data;
  video_update_task (&baton->settings.camera, &baton->boolean_settings);
  baton->error = omxcam_last_error ();
}

void video_async_cb (uv_work_t* req, int status){
  video_buffer_baton_t* baton = (video_buffer_baton_t*)req->data;
  
  Handle<Value> argv[1];
  
  if (baton->error){
    argv[0] = Exception::Error (String::New (omxcam_strerror (baton->error)));
  }else{
    argv[0] = Null ();
  }
  
  TryCatch try_catch;
  baton->cb->Call (Context::GetCurrent ()->Global (), 1, argv);
  if (try_catch.HasCaught ()){
    FatalException (try_catch);
  }
  
  baton->cb.Dispose ();
  free (baton);
}

void video_read_async (uv_work_t* req){
  video_buffer_baton_t* baton = (video_buffer_baton_t*)req->data;
  omxcam_video_read_npt (&baton->buffer, &baton->is_motion_vector);
  baton->error = omxcam_last_error ();
}

void video_set_motion_vector_property (
    Local<Object> buffer,
    omxcam_bool is_motion_vector){
  buffer->Set (String::New ("isMotionVector"),
      is_motion_vector ? True () : False ());
}

void video_read_async_cb (uv_work_t* req, int status){
  video_buffer_baton_t* baton = (video_buffer_baton_t*)req->data;
  
  Handle<Value> argv[2];
  
  if (baton->error){
    argv[0] = Exception::Error (String::New (omxcam_strerror (baton->error)));
    argv[1] = Null ();
  }else{
    Local<Object> global = Context::GetCurrent ()->Global ();

    Buffer* slow_buffer = Buffer::New (baton->buffer.length);
    memcpy (Buffer::Data (slow_buffer), baton->buffer.data,
        baton->buffer.length);

    Local<Function> buffer_constructor =
        Local<Function>::Cast (global->Get (String::New ("Buffer")));
    Handle<Value> constructor_args[3] = {
      slow_buffer->handle_,
      Uint32::New (baton->buffer.length),
      Uint32::New (0)
    };
    argv[0] = Null ();
    Local<Object> buff = buffer_constructor->NewInstance (3, constructor_args);
    argv[1] = buff;
    
    video_set_motion_vector_property (buff, baton->is_motion_vector);
  }
  
  TryCatch try_catch;
  baton->cb->Call (Context::GetCurrent ()->Global (), 2, argv);
  if (try_catch.HasCaught ()){
    FatalException (try_catch);
  }
  
  baton->cb.Dispose ();
  free (baton);
}

Handle<Value> video_start (const Arguments& args){
  int args_length = args.Length ();
  
  if (args[1]->IsFunction ()){
    video_buffer_baton_t* baton =
        (video_buffer_baton_t*)malloc (sizeof (video_buffer_baton_t));
    
    baton->req.data = (void*)baton;
    baton->cb = Persistent<Function>::New (
        Local<Function>::Cast(args[args_length - 1]));
    
    omxcam_video_init (&baton->settings);
    
    if (args[0]->IsObject ()){
      video_obj_to_settings (args[0]->ToObject (), &baton->settings);
    }
    
    uv_queue_work (uv_default_loop (), &baton->req, video_start_async,
        video_async_cb);
  }else{
    omxcam_video_settings_t settings;
    omxcam_video_init (&settings);
    
    if (args[0]->IsObject ()){
      if (video_obj_to_settings (args[0]->ToObject (), &settings)){
        THROW_BAD_ARGUMENTS
      }
    }else if (!args[0]->IsUndefined ()){
      THROW_BAD_ARGUMENTS
    }
    
    if (omxcam_video_start_npt (&settings)){
      return ThrowException (Exception::Error (String::New (
          omxcam_strerror (omxcam_last_error ()))));
    }
  }
  
  HandleScope scope;
  return scope.Close (Undefined ());
}

Handle<Value> video_stop (const Arguments& args){
  if (args.Length () && args[0]->IsFunction ()){
    video_buffer_baton_t* baton =
        (video_buffer_baton_t*)malloc (sizeof (video_buffer_baton_t));
    
    baton->req.data = (void*)baton;
    baton->cb = Persistent<Function>::New (Local<Function>::Cast(args[0]));
    
    uv_queue_work (uv_default_loop (), &baton->req, video_stop_async,
        video_async_cb);
  }else{
    if (omxcam_video_stop_npt ()){
      return ThrowException (Exception::Error (String::New (
          omxcam_strerror (omxcam_last_error ()))));
    }
  }
  
  HandleScope scope;
  return scope.Close (Undefined ());
}

Handle<Value> video_read (const Arguments& args){
  if (args.Length () && args[0]->IsFunction ()){
    video_buffer_baton_t* baton =
        (video_buffer_baton_t*)malloc (sizeof (video_buffer_baton_t));
    
    baton->req.data = (void*)baton;
    baton->cb = Persistent<Function>::New (Local<Function>::Cast(args[0]));
    
    uv_queue_work (uv_default_loop (), &baton->req, video_read_async,
        video_read_async_cb);
    
    HandleScope scope;
    return scope.Close (Undefined ());
  }else{
    omxcam_buffer_t buffer;
    omxcam_bool is_motion_vector;
    if (omxcam_video_read_npt (&buffer, &is_motion_vector)){
      return ThrowException (Exception::Error (String::New (
          omxcam_strerror (omxcam_last_error ()))));
    }
    Local<Object> buff = create_buffer (&buffer);
    video_set_motion_vector_property (buff, is_motion_vector);
    return buff;
  }
}

Handle<Value> video_update (const Arguments& args){
  if (!args[0]->IsObject ()) THROW_BAD_ARGUMENTS
  
  if (args[1]->IsFunction ()){
    video_buffer_baton_t* baton =
        (video_buffer_baton_t*)malloc (sizeof (video_buffer_baton_t));
    
    baton->req.data = (void*)baton;
    baton->cb = Persistent<Function>::New (Local<Function>::Cast(args[1]));
    
    memset (&baton->boolean_settings, 0, sizeof (baton->boolean_settings));  
    
    if (camera_obj_to_settings (args[0]->ToObject (), &baton->settings.camera,
        &baton->boolean_settings)){
      THROW_BAD_ARGUMENTS
    }
    
    uv_queue_work (uv_default_loop (), &baton->req, video_update_async,
        video_async_cb);
  }else{
    omxcam_video_settings_t settings;
    video_boolean_settings_t boolean_settings;
    
    memset (&boolean_settings, 0, sizeof (boolean_settings));  
    
    if (camera_obj_to_settings (args[0]->ToObject (), &settings.camera,
        &boolean_settings)){
      THROW_BAD_ARGUMENTS
    }
    
    if (video_update_task (&settings.camera, &boolean_settings)){
      return ThrowException (Exception::Error (String::New (
          omxcam_strerror (omxcam_last_error ()))));
    }
  }
  
  HandleScope scope;
  return scope.Close (Undefined ());
}

Handle<Value> video_settings (const Arguments& args){
  omxcam_video_settings_t settings;
  omxcam_video_init (&settings);
  
  if (!args[0]->IsUndefined ()){
    video_obj_to_settings (args[0]->ToObject (), &settings);
  }
  
  Local<Object> obj = Object::New ();
  
  video_settings_to_obj (&settings, obj);
  
  HandleScope scope;
  return scope.Close (obj);
}

void init (Handle<Object> exports){
  NODE_SET_METHOD (exports, "video_start", video_start);
  NODE_SET_METHOD (exports, "video_stop", video_stop);
  NODE_SET_METHOD (exports, "video_read", video_read);
  NODE_SET_METHOD (exports, "video_update", video_update);
  NODE_SET_METHOD (exports, "video_settings", video_settings);
  
  NODE_SET_METHOD (exports, "add_ISO_constants", add_ISO_constants);
  NODE_SET_METHOD (exports, "add_EXPOSURE_constants", add_EXPOSURE_constants);
  NODE_SET_METHOD (exports, "add_MIRROR_constants", add_MIRROR_constants);
  NODE_SET_METHOD (exports, "add_ROTATION_constants", add_ROTATION_constants);
  NODE_SET_METHOD (exports, "add_METERING_constants", add_METERING_constants);
  NODE_SET_METHOD (exports, "add_WHITE_BALANCE_constants",
      add_WHITE_BALANCE_constants);
  NODE_SET_METHOD (exports, "add_IMAGE_FILTER_constants",
      add_IMAGE_FILTER_constants);
  NODE_SET_METHOD (exports, "add_DRC_constants", add_DRC_constants);
  NODE_SET_METHOD (exports, "add_H264_AVC_PROFILE_constants",
      add_H264_AVC_PROFILE_constants);
  NODE_SET_METHOD (exports, "add_auto_constants", add_auto_constants);
  NODE_SET_METHOD (exports, "add_format_constants", add_format_constants);
  
  NODE_SET_METHOD (exports, "yuv_planes", yuv_planes);
  NODE_SET_METHOD (exports, "yuv_planes_slice", yuv_planes_slice);
}

NODE_MODULE (addon, init)