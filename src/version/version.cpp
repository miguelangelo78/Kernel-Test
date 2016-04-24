/*
 * version.cpp
 *
 *  Created on: 04/04/2016
 *      Author: Miguel
 */

#include <version/version.h>

#define VER_UNKNOWN (char*)"UNNAMED";

char * ver_kernel_name = (char*)"Kernel Test";
char * ver_kernel_version_fmt = (char*)"%d.%d.%d";

int ver_kernel_major = 0;
int ver_kernel_minor = 0;
int ver_kernel_lower = 238;

char * ver_kernel_codename = VER_UNKNOWN;

char * ver_kernel_arch = (char*)"x86";

char * ver_kernel_build_date = (char*)__DATE__;
char * ver_kernel_build_time = (char*)__TIME__;

char * ver_kernel_builtby = (char*)"miguel@miguel-pc";

char * ver_kernel_author = (char*)"Miguel Santos";
