#pragma once
//Use this file to include windows with a minimal required feature set


//set unicode
#ifndef UNICODE
#define UNICODE
#endif

//target win10 or later
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

//include asio to avoid conflicts
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

//cut out all of this we do not need. This may lead to errors when trying to 
//do windows stuff, so start by commenting them all out and bringing them back 
//one by one. Keeping them out speeds up build times and decreases namespace pollution.

#define WIN32_LEAN_AND_MEAN
#define	NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
//#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

//#define STRICT

//and then include windows
#include <Windows.h>

////for text
////#ifndef HINST_THISCOMPONENT
////EXTERN_C IMAGE_DOS_HEADER __ImageBase;
////#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
////#endif
////#include <dwrite.h>
////#include <dwrite_1.h>
////#include <d2d1.h>
////#include <wincodec.h>
//////#include "SafeRelease.h"
////#pragma comment( lib, "D2d1.lib" )
////#pragma comment( lib, "Dwrite.lib")
//
//////DX toolkit Helpers
////#include <BufferHelpers.h>
////#include <CommonStates.h>
////#include <DDSTextureLoader.h>
////#include <DirectXHelpers.h>
////#include <Effects.h>
////#include <GamePad.h>
////#include <GeometricPrimitive.h>
////#include <GraphicsMemory.h>
////#include <Keyboard.h>
////#include <Model.h>
////#include <Mouse.h>
////#include <PostProcess.h>
////#include <PrimitiveBatch.h>
////#include <ScreenGrab.h>
////#include <SimpleMath.h>
////#include <SpriteBatch.h>
////#include <SpriteFont.h>
////#include <VertexTypes.h>
////#include <WICTextureLoader.h>
//
