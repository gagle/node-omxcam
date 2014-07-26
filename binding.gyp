{
  "targets": [
    {
      "target_name": "addon",
      "dependencies": ["omxcam"],
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
        "USE_EXTERNAL_OMX"
      ],
      "cflags": [
        "-U_FORTIFY_SOURCE",
        "-ftree-vectorize",
        "-pipe",
        "-fPIC",
        "-Werror",
        "-g",
        "-Wall",
        "-O2"
      ],
      "include_dirs": [
        "deps/omxcam/include",
        "/opt/vc/include",
        "/opt/vc/include/interface/vcos/pthreads",
        "/opt/vc/include/interface/vmcs_host/linux"
      ],
      "direct_dependent_settings": {
        "include_dirs": [
          "deps/omxcam/include",
          "/opt/vc/include",
          "/opt/vc/include/interface/vcos/pthreads",
          "/opt/vc/include/interface/vmcs_host/linux"
        ]
      },
      "link_settings": {
        "library_dirs": [
          "/opt/vc/lib"
        ],
        "libraries": [
          "-lopenmaxil",
          "-lbcm_host",
          "-lvcos",
          "-lvchiq_arm",
          "-lpthread"
        ]
      },
      "sources": [
        "deps/omxcam/src/camera.c",
        "deps/omxcam/src/core.c",
        "deps/omxcam/src/debug.c",
        "deps/omxcam/src/dump_omx.c",
        "deps/omxcam/src/error.c",
        "deps/omxcam/src/event.c",
        "deps/omxcam/src/h264.c",
        "deps/omxcam/src/jpeg.c",
        "deps/omxcam/src/still.c",
        "deps/omxcam/src/utils.c",
        "deps/omxcam/src/version.c",
        "deps/omxcam/src/video.c",
      ]
    }
  ]
}
