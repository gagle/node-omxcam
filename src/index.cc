#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "omxcam.h"

using namespace v8;
using namespace node;

typedef struct {
	uv_work_t req;
	omxcam_buffer_t buffer;
	omxcam_video_settings_t settings;
	omxcam_errno error;
	Persistent<Function> cb;
} video_buffer_baton_t;

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

int video_update_task (){
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
	printf ("video_start\n");
	
	HandleScope scope;
	int args_length = args.Length ();
	
	if (args_length && args[args_length - 1]->IsFunction ()){
		video_buffer_baton_t* baton =
				(video_buffer_baton_t*)malloc (sizeof (video_buffer_baton_t));
		
		baton->req.data = (void*)baton;
		baton->cb = Persistent<Function>::New (
				Local<Function>::Cast(args[args_length - 1]));
		
		omxcam_video_init (&baton->settings);
		baton->settings.camera.width = 640;
		baton->settings.camera.height = 480;
		
		uv_queue_work (uv_default_loop (), &baton->req, video_start_async,
				video_async_cb);
	}else{
		omxcam_video_settings_t settings;
		omxcam_video_init (&settings);
		
		settings.camera.width = 640;
		settings.camera.height = 480;
		
		if (omxcam_video_start_npt (&settings)){
			return ThrowException (Exception::Error (String::New (
					omxcam_strerror (omxcam_last_error ()))));
		}
	}
	
	return scope.Close (Undefined ());
}

Handle<Value> video_stop (const Arguments& args){
	printf ("video_stop\n");
	
	HandleScope scope;
	
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
	
	return scope.Close (Undefined ());
}

Handle<Value> video_read (const Arguments& args){
	printf ("video_read\n");
	
	HandleScope scope;
	
	if (args.Length () && args[0]->IsFunction ()){
		video_buffer_baton_t* baton =
				(video_buffer_baton_t*)malloc (sizeof (video_buffer_baton_t));
		
		baton->req.data = (void*)baton;
		baton->cb = Persistent<Function>::New (Local<Function>::Cast(args[0]));
		
		uv_queue_work (uv_default_loop (), &baton->req, video_read_async,
				video_read_async_cb);
		
		return scope.Close (Undefined ());
	}else{
		return video_read_task ();
	}
}

Handle<Value> video_update (const Arguments& args){
	printf ("video_update\n");
	
	HandleScope scope;
	
	if (args.Length () == 2){
		video_buffer_baton_t* baton =
				(video_buffer_baton_t*)malloc (sizeof (video_buffer_baton_t));
		
		baton->req.data = (void*)baton;
		baton->cb = Persistent<Function>::New (Local<Function>::Cast(args[1]));
		
		uv_queue_work (uv_default_loop (), &baton->req, video_update_async,
				video_async_cb);
	}else{
		/*if (video_update_task ()){
			return ThrowException (Exception::Error (String::New (
					omxcam_strerror (omxcam_last_error ()))));
		}*/
	}
	
	return scope.Close (Undefined ());
}

void init (Handle<Object> exports){
	NODE_SET_METHOD (exports, "video_start", video_start);
	NODE_SET_METHOD (exports, "video_stop", video_stop);
	NODE_SET_METHOD (exports, "video_read", video_read);
	NODE_SET_METHOD (exports, "video_update", video_update);
}

NODE_MODULE (addon, init)