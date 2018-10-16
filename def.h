#pragma once

#define __CQ_EVENT(ReturnType, Name, Size)                                                            \
    __pragma(comment(linker, "/EXPORT:" #Name "=_" #Name "@" #Size)) extern "C" __declspec(dllexport) \
        ReturnType __stdcall Name
