/*
author          Oliver Blaser
date            19.12.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#ifndef IG_PROJECT_H
#define IG_PROJECT_H

#include <omw/defs.h>
#include <omw/version.h>


namespace prj {

const char* const appName = "treesha1sum";
const char* const exeName = "treesha1sum"; // eq to the linker setting

const char* const website = "https://github.com/oblaser/treesha1sum";

const omw::Version version(0, 1, 1, "");

} // namespace prj


#ifdef OMW_DEBUG
#define PRJ_DEBUG (1)
#else
#undef PRJ_DEBUG
#endif


#endif // IG_PROJECT_H
