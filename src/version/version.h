/*
 * version.h
 *
 *  Created on: 04/04/2016
 *      Author: Miguel
 */

#ifndef SRC_VERSION_VERSION_H_
#define SRC_VERSION_VERSION_H_

extern char * ver_kernel_name;
extern char * ver_kernel_version_fmt;

extern int ver_kernel_major;
extern int ver_kernel_minor;
extern int ver_kernel_lower;

extern char * ver_kernel_codename;

extern char * ver_kernel_arch;

extern char * ver_kernel_build_date;
extern char * ver_kernel_build_time;

extern char * ver_kernel_builtby;
extern char * ver_kernel_author;

#if (defined(__GNUC__) || defined(__GNUG__)) && !(defined(__clang__) || defined(__INTEL_COMPILER))
# define COMPILER_VERSION "gcc " __VERSION__
#elif (defined(__clang__))
# define COMPILER_VERSION "clang " __clang_version__
#else
# define COMPILER_VERSION "unknown-compiler how-did-you-do-that"
#endif

#endif /* SRC_VERSION_VERSION_H_ */
