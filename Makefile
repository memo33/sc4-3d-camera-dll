# This DLL can be compiled on Linux using `clang` as a cross-compiler.
# This roughly follows the approach outlined at https://jake-shadle.github.io/xwin/#how.
#
# Install `clang` and `lld` on your distro. We use the MSVC-compatible flavors `clang-cl` and `lld-link` for compilation.
#
# Download and extract the `xwin` downloader: https://github.com/Jake-Shadle/xwin/releases
#
# Use `xwin` to download MSVC header and lib files for 32-bit:
#
#   ./xwin-0.5.0-x86_64-unknown-linux-musl/xwin --accept-license --arch x86 splat --output ./vendor/xwin
#
# Download the header library WIL from https://github.com/microsoft/wil/releases
# and extract it to `./vendor/wil`.
# 7zip is able to extract .nupkg archives.
#
# Run `make` to compile the DLL using the below commands.

compile:
	cd src && \
		clang-cl -target i386-pc-windows-msvc -ferror-limit=1 -fuse-ld=lld-link \
		-Wno-inconsistent-missing-override \
		/std:c++20 /EHsc /LD /MD \
		/Gy /Gd /O2 /Oi \
		/D "_WIN32" /D "NDEBUG" /D "_UNICODE" /D "UNICODE" \
		/D "_USRDLL" /D "_WINDLL" \
		/imsvc ../vendor/xwin/crt/include /imsvc ../vendor/xwin/sdk/include/ucrt /imsvc ../vendor/xwin/sdk/include/um /imsvc ../vendor/xwin/sdk/include/shared \
		/I ../vendor/gzcom-dll/gzcom-dll/include \
		/I ../vendor/wil/include \
		/I ../vendor/sc4-resource-loading-hooks/src/public/include \
		/o memo.3dcamera.dll ./*.cpp ../vendor/gzcom-dll/gzcom-dll/src/{cRZCOMDllDirector,cRZBaseString}.cpp \
		/link /libpath:../vendor/xwin/crt/lib/x86 /libpath:../vendor/xwin/sdk/lib/um/x86 /libpath:../vendor/xwin/sdk/lib/ucrt/x86 \
		version.lib ole32.lib
#
# /GL = whole program optimization -> not supported by clang
# /W3 = Wall -> not necessary
# /Gy = put each function in own section
# /Gd = __cdecl default calling convention
# /MD = use DLL runtime (related with /LD)
# /FC = full path diagnosticts -> not supported, not needed


.PHONY: compile
