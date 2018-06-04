/* include/llvm/Config/llvm-config.h.  Generated from llvm-config.h.in by configure.  */
/*===-- llvm/config/llvm-config.h - llvm configure variable -------*- C -*-===*/
/*                                                                            */
/*                     The LLVM Compiler Infrastructure                       */
/*                                                                            */
/* This file is distributed under the University of Illinois Open Source      */
/* License. See LICENSE.TXT for details.                                      */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

/* This file enumerates all of the llvm variables from configure so that
   they can be in exported headers and won't override package specific
   directives.  This is a C file so we can include it in the llvm-c headers.  */

/* To avoid multiple inclusions of these variables when we include the exported
   headers and config.h, conditionally include these.  */
/* TODO: This is a bit of a hack.  */
#ifndef CONFIG_H

/* Installation directory for binary executables */
#define LLVM_BINDIR "/usr/local/bin"

/* Time at which LLVM was configured */
#define LLVM_CONFIGTIME "Wed Mar  4 01:21:58 EST 2015"

/* Installation directory for data files */
#define LLVM_DATADIR "/usr/local/share/llvm"

/* Installation directory for documentation */
#define LLVM_DOCSDIR "/usr/local/share/doc/llvm"

/* Installation directory for config files */
#define LLVM_ETCDIR "/usr/local/etc/llvm"

/* Host triple we were built on */
#define LLVM_HOSTTRIPLE "x86_64-unknown-linux-gnu"

/* Installation directory for include files */
#define LLVM_INCLUDEDIR "/usr/local/include"

/* Installation directory for .info files */
#define LLVM_INFODIR "/usr/local/info"

/* Installation directory for libraries */
#define LLVM_LIBDIR "/usr/local/lib"

/* Installation directory for man pages */
#define LLVM_MANDIR "/usr/local/man"

/* Build multithreading support into LLVM */
#define LLVM_MULTITHREADED 1

/* LLVM architecture name for the native architecture, if available */
/* #undef LLVM_NATIVE_ARCH */

/* LLVM name for the native AsmParser init function, if available */
/* #undef LLVM_NATIVE_ASMPARSER */

/* LLVM name for the native AsmPrinter init function, if available */
/* #undef LLVM_NATIVE_ASMPRINTER */

/* LLVM name for the native Target init function, if available */
/* #undef LLVM_NATIVE_TARGET */

/* LLVM name for the native TargetInfo init function, if available */
/* #undef LLVM_NATIVE_TARGETINFO */

/* LLVM name for the native target MC init function, if available */
/* #undef LLVM_NATIVE_TARGETMC */

/* Define if this is Unixish platform */
#define LLVM_ON_UNIX 1

/* Define if this is Win32ish platform */
/* #undef LLVM_ON_WIN32 */

/* Define to path to circo program if found or 'echo circo' otherwise */
#define LLVM_PATH_CIRCO "/usr/bin/circo"

/* Define to path to dot program if found or 'echo dot' otherwise */
#define LLVM_PATH_DOT "/usr/bin/dot"

/* Define to path to dotty program if found or 'echo dotty' otherwise */
#define LLVM_PATH_DOTTY "/usr/bin/dotty"

/* Define to path to fdp program if found or 'echo fdp' otherwise */
#define LLVM_PATH_FDP "/usr/bin/fdp"

/* Define to path to Graphviz program if found or 'echo Graphviz' otherwise */
/* #undef LLVM_PATH_GRAPHVIZ */

/* Define to path to gv program if found or 'echo gv' otherwise */
/* #undef LLVM_PATH_GV */

/* Define to path to neato program if found or 'echo neato' otherwise */
#define LLVM_PATH_NEATO "/usr/bin/neato"

/* Define to path to twopi program if found or 'echo twopi' otherwise */
#define LLVM_PATH_TWOPI "/usr/bin/twopi"

/* Installation prefix directory */
#define LLVM_PREFIX "/usr/local"

#endif
