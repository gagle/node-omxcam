#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <stdio.h>
#include <string.h>

#include "../deps/omxcam/omxcam.h"

using namespace v8;
using namespace node;;

Local<Function> onError;
Local<Function> onData;
Local<Function> onEnd;

void callbackVideo (uint8_t* data, uint32_t length){
  Local<Object> globalObj = Context::GetCurrent ()->Global ();
  
  Local<Value> len = Uint32::New (length);
  Buffer* slowBuffer = Buffer::New (length);
  memcpy(Buffer::Data (slowBuffer), data, length);
  
  Local<Function> bufferConstructor =
      Local<Function>::Cast (globalObj->Get (String::New ("Buffer")));
  Handle<Value> constructorArgs[3] = {
    slowBuffer->handle_,
    len,
    Uint32::New (0)
  };
  Local<Object> buffer = bufferConstructor->NewInstance (3, constructorArgs);
  
  Local<Value> argv[2] = { buffer, len };
  onData->Call (globalObj, 2, argv);
}

Handle<Value> still (const Arguments& args){
  HandleScope scope;
  
  return scope.Close (Undefined ());
}

Handle<Value> cancelStill (const Arguments& args){
  HandleScope scope;
  
  return scope.Close (Undefined ());
}

Handle<Value> startVideo (const Arguments& args){
  HandleScope scope;
  
  printf ("startVideo\n");
  
  onError = Local<Function>::Cast (args[1]);
  onData = Local<Function>::Cast (args[2]);
  onEnd = Local<Function>::Cast (args[3]);
  
  OMXCAM_VIDEO_SETTINGS settings;
  OMXCAM_initVideoSettings (&settings);
  settings.bufferCallback = callbackVideo;
  OMXCAM_ERROR error;
  
  if ((error = OMXCAM_startVideo (&settings, 3000))){
    printf ("ERROR: %s (%s)\n", OMXCAM_errorName (error), OMXCAM_strError ());
    return scope.Close (Undefined ());
  }
  
  onEnd->Call (Context::GetCurrent ()->Global (), 0, 0);
  
  return scope.Close (Undefined ());
}

Handle<Value> stopVideo (const Arguments& args){
  HandleScope scope;
  
  printf ("stopVideo\n");
  
  return scope.Close (Undefined ());
}

void init (Handle<Object> exports){
  NODE_SET_METHOD (exports, "still", still);
  NODE_SET_METHOD (exports, "cancelStill", cancelStill);
  NODE_SET_METHOD (exports, "startVideo", startVideo);
  NODE_SET_METHOD (exports, "stopVideo", stopVideo);
}

NODE_MODULE (addon, init)