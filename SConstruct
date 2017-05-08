#/usr/bin/env python

import os
import platform

CFLAGS = ["-Wall", "-pedantic", "-std=c99"]
CFLAGS_RELEASE = ["-O3"]
CFLAGS_DEBUG = ["-g", "-ggdb"]
LUA_VERSION = "5.2"

PATH = os.environ["PATH"] + os.pathsep + "/opt/local/bin"

# building environment
env = Environment(CFLAGS = CFLAGS, CDEFINES = [], ENV = { "PATH" : PATH })

# handle command line arguments
vars = Variables()
vars.AddVariables(
        BoolVariable("verbose", "Set to show compilation lines", False),
        BoolVariable("release", "Set to make a release build", False),
        EnumVariable("target", "Choose a build flavor", "debug",
            allowed_values = ("release", "debug", "static"),
            ignorecase = 2),
        PathVariable("CC", "The C compiler", "gcc", PathVariable.PathAccept),
        PathVariable("CXX", "The C++ compiler", "g++", PathVariable.PathAccept),
        )
vars.Update(env)

AddOption("--verbose", action="store_true", help="verbose output", default=False)

if not GetOption("verbose"):
    env["SHCCCOMSTR"]   = "SHC $SOURCE"
    env["SHLINKCOMSTR"] = "SHCC $TARGET"
    env["CCCOMSTR"]     = "CC   $SOURCE"
    env["CCOMSTR"]    = "C  $SOURCE"
    env["LINKCOMSTR"]   = "LINK $TARGET"
    env["ARCOMSTR"]     = "ARCH $TARGET"
    env["RANLIBCOMSTR"] = "INDEX $TARGET"

if env["target"] == "release":
    env["CFLAGS"].extend(CFLAGS_RELEASE);
    env["LINKFLAGS"].extend(["-s"])
else:  # default debug
    if env["target"] == "static":
        env["CFLAGS"].extend(["-static"])
        env["LINKFLAGS"].extend(["-static"])
    env["CFLAGS"].extend(CFLAGS_DEBUG)
    env["CDEFINES"].append("DEBUG")

# list of source files to build
src_files = Glob("*.c")

# checking generic dependencies
conf = env.Configure()
conf.CheckCC()

# update the platform related files and check platform specific deps
os = platform.system().lower();
if os == "linux":
    src_files.append("compat/compat_linux.c")
    if conf.CheckLib("liblua" + LUA_VERSION):
        env.ParseConfig('pkg-config --cflags --libs lua' + LUA_VERSION)
    conf.CheckHeader("sys/inotify.h")
elif os in ["freebsd"]:
    # OpenBSD and Darwin OS have old kqueue and do not support most of the
    # events. Therefore they are considered unsupported.
    src_files.append("compat/compat_bsd.c")
    env.ParseConfig('pkg-config --cflags --libs lua-' + LUA_VERSION)
    conf.CheckLib("liblua-" + LUA_VERSION) # XXX: handle cases when LUA is not installed
    conf.CheckHeader("sys/event.h")

# check for lua headers
# XXX: handle the cases where the headers are NOT found
conf.CheckHeader("sys/stat.h")
conf.CheckHeader("lua.h")
conf.CheckHeader("lualib.h")
conf.CheckHeader("lauxlib.h")

# main function
env.Program(target = "imk", source = src_files)
