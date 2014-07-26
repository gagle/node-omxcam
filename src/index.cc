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

Handle<Value> add_auto_constants (const Arguments& args){
  Local<Object> obj = args[0]->ToObject ();
  CONSTANT_VALUES (SHUTTER_SPEED_AUTO, _)
  CONSTANT_VALUES (JPEG_THUMBNAIL_WIDTH_AUTO, _)
  CONSTANT_VALUES (JPEG_THUMBNAIL_HEIGHT_AUTO, _)
  CONSTANT_VALUES (H264_IDR_PERIOD_OFF, _)
  HandleScope scope;
  return scope.Close (Undefined ());
}

omxcam_bool bool_to_omxcam (bool value){
  return value ? OMXCAM_TRUE : OMXCAM_FALSE;
}

int set_camera_settings (
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

int set_h264_settings (Local<Object> obj, omxcam_h264_settings_t* settings){
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
  }
  
  return 0;
}

int set_video_settings (Local<Object> obj, omxcam_video_settings_t* settings){
  if (set_camera_settings (obj, &settings->camera, 0)) return -1;
  return set_h264_settings (obj, &settings->h264);
}

Handle<Value> video_read_task (){
  omxcam_buffer_t buffer;
  if (omxcam_video_read_npt (&buffer)){
    return ThrowException (Exception::Error (String::New (
        omxcam_strerror (omxcam_last_error ()))));
  }
  
  Local<Object> global = Context::GetCurrent ()->Global ();

  Buffer* slow_buffer = Buffer::New (buffer.length);
  memcpy (Buffer::Data (slow_buffer), buffer.data, buffer.length);

  Local<Function> buffer_constructor =
      Local<Function>::Cast (global->Get (String::New ("Buffer")));
  Handle<Value> constructor_args[3] = {
    slow_buffer->handle_,
    Uint32::New (buffer.length),
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
  omxcam_video_read_npt (&baton->buffer);
  baton->error = omxcam_last_error ();
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
    argv[1] = buffer_constructor->NewInstance (3, constructor_args);
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
      set_video_settings (args[0]->ToObject (), &baton->settings);
    }
    
    uv_queue_work (uv_default_loop (), &baton->req, video_start_async,
        video_async_cb);
  }else{
    omxcam_video_settings_t settings;
    omxcam_video_init (&settings);
    
    if (args[0]->IsObject ()){
      if (set_video_settings (args[0]->ToObject (), &settings)){
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
    return video_read_task ();
  }
}

Handle<Value> video_update (const Arguments& args){
  if (args[1]->IsFunction ()){
    video_buffer_baton_t* baton =
        (video_buffer_baton_t*)malloc (sizeof (video_buffer_baton_t));
    
    baton->req.data = (void*)baton;
    baton->cb = Persistent<Function>::New (Local<Function>::Cast(args[1]));
    
    memset (&baton->boolean_settings, 0, sizeof (baton->boolean_settings));  
    
    if (set_camera_settings (args[0]->ToObject (), &baton->settings.camera,
        &baton->boolean_settings)){
      THROW_BAD_ARGUMENTS
    }
    
    uv_queue_work (uv_default_loop (), &baton->req, video_update_async,
        video_async_cb);
  }else{
    omxcam_video_settings_t settings;
    video_boolean_settings_t boolean_settings;
    
    memset (&boolean_settings, 0, sizeof (boolean_settings));  
    
    if (set_camera_settings (args[0]->ToObject (), &settings.camera,
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

void init (Handle<Object> exports){
  NODE_SET_METHOD (exports, "video_start", video_start);
  NODE_SET_METHOD (exports, "video_stop", video_stop);
  NODE_SET_METHOD (exports, "video_read", video_read);
  NODE_SET_METHOD (exports, "video_update", video_update);
  
  NODE_SET_METHOD (exports, "add_ISO_constants", add_ISO_constants);
  NODE_SET_METHOD (exports, "add_EXPOSURE_constants", add_EXPOSURE_constants);
  NODE_SET_METHOD (exports, "add_MIRROR_constants", add_MIRROR_constants);
  NODE_SET_METHOD (exports, "add_ROTATION_constants", add_ROTATION_constants);
  NODE_SET_METHOD (exports, "add_METERING_constants", add_METERING_constants);
  NODE_SET_METHOD (exports, "add_WHITE_BALANCE_constants",
      add_WHITE_BALANCE_constants);
  NODE_SET_METHOD (exports, "add_IMAGE_FILTER_constants",
      add_IMAGE_FILTER_constants);
  NODE_SET_METHOD (exports, "add_auto_constants", add_auto_constants);
}

NODE_MODULE (addon, init)