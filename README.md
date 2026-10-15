# Hover Hugs
Hover Hugs is a restoration effort to make the Hugs codebase more
palatable. As of now, it's a heavily stripped-down version of Hugs98.

# Changes
- Removal of all platforms except Linux
- Removal of all packages except `hugsbase`
- Complete rewrite of the build system to use standard `make`
- The Hugs path is now relative to the executable

# Internal Changes
- Unicode table is now generated at build time
- `ffihugs` is now a wrapper over `runhugs`
- Header files have been made properly independent
- Source files no longer include other source files (TODO: `parser.c`)
- Delete lots of unneeded `#if` branches (TODO: still incomplete)
- Convert `#define` into proper C constructors (BIG TODO: very incomplete)
- Various other cleanups

# Status
Hugs is still not in a good shape. There are still lots of unneeded
preprocessor conditionals, awful macro usage, and K&R definitions. A lot of
comments are outdated too, now that a lot of platform support has been removed.

Some modules in `hugsbase` don't work because they depend on `base` modules.
Eventually, those modules need to be removed or the original base definitions
need to be supplied somehow. At least the `Prelude` seems to work.

Everything under `demos/`, `tests/`, `lib/`, `docs/` and some other files
are completely untested and may be scrapped.

Long term, it would be nice if Hugs could be gradually ported to a C-compatible
language that has garbage collection. Something like D or Go.

# Building
Edit `src/config.h` manually to change the build configuration. As mentioned
before, Linux is currently the only supported platform.

Running `make` should build a `dist` folder which contains the `bin` for the
associated executables. These executables are portable so they can be moved
anywhere.

# Copyright
The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
1994-2005. All rights reserved. It is distributed as free software under
the license in the file "License", which is included in the distribution.