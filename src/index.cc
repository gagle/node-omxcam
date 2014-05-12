#include <v8.h>
#include <node.h>
#include <stdio.h>

#include "../deps/omxcam/omxcam.h"

using namespace v8;

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
  
  //Local<Function> onError = Local<Function>::Cast (args[1]);
  //Local<Function> onData = Local<Function>::Cast (args[2]);
  Local<Function> onEnd = Local<Function>::Cast (args[3]);
  
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