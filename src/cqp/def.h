//
// Created by richard on 2017/8/5.
//

#pragma once

#define CQAPIVER 9
#define CQAPIVERTEXT "9"

#define CQEVENT(ReturnType, Name, Size) __pragma(comment(linker, "/EXPORT:" #Name "=_" #Name "@" #Size)) extern "C" __declspec(dllexport) ReturnType __stdcall Name
