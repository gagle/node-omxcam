{
  "targets": [
    {
      "target_name": "addon",
      "dependencies": [
        "omxcam"
      ],
      "sources": ["src/index.cc"]
    },
    {
      "target_name": "omxcam",
      "type": "static_library",
      "defines": [
        "STANDALONE",
        "__STDC_CONSTANT_MACROS",
        "__STDC_LIMIT_MACROS",
        "TARGET_POSIX",
        "_LINUX",
        "PIC",
        "_REENTRANT",
        "_LARGEFILE64_SOURCE",
        "_FILE_OFFSET_BITS=64",
        "HAVE_LIBOPENMAX=2",
        "OMX",
        "OMX_SKIP64BIT",
        "USE_EXTERNAL_OMX",
        "HAVE_LIBBCM_HOST",
        "USE_EXTERNAL_LIBBCM_HOST",
        "USE_VCHIQ_ARM",
        "USE_EXTERNAL_OMX",
        "OMXCAM_DEBUG"
      ],
      "cflags": [
        "-U_FORTIFY_SOURCE",
        "-ftree-vectorize",
        "-pipe",
        "-fPIC",
        "-ftree-vectorize",
        "-Werror",
        "-g",
        "-Wall",
        "-O2"
      ],
      "ldflags": [
        "-L/opt/vc/lib",
        "-lopenmaxil",
        "-lbcm_host",
        "-lvcos",
        "-lvchiq_arm",
        "-lpthread",
      ],
      "include_dirs": [
        "/opt/vc/include",
        "/opt/vc/include/interface/vcos/pthreads",
        "/opt/vc/include/interface/vmcs_host/linux"
      ],
      "direct_dependent_settings": {
        "include_dirs": [
          "/opt/vc/include",
          "/opt/vc/include/interface/vcos/pthreads",
          "/opt/vc/include/interface/vmcs_host/linux"
        ]
      },
      "sources": [
        "deps/omxcam/omxcam_camera.c",
        "deps/omxcam/omxcam_core.c",
        "deps/omxcam/omxcam_debug.c",
        "deps/omxcam/omxcam_dump.c",
        "deps/omxcam/omxcam_errors.c",
        "deps/omxcam/omxcam_image_encode.c",
        "deps/omxcam/omxcam_still.c",
        "deps/omxcam/omxcam_utils.c",
        "deps/omxcam/omxcam_video.c",
        "deps/omxcam/omxcam_video_encode.c"
      ]
    }
  ]
}
