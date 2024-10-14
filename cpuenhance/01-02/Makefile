# This makefile is for building on Windows with nmake. It is setup to use the
# MSVC compiler. All build artifacts are placed into the build directory to keep
# the root directory clean.

BUILDDIR = build
TESTDIR = test_output
LISTING_0039 = ..\materials\perfaware\part1\listing_0039_more_movs
LISTING_0040 = ..\materials\perfaware\part1\listing_0040_challenge_movs
CFLAGS = /Fo$(BUILDDIR)\ /Fd$(BUILDDIR)\ /Fe$(BUILDDIR)\ /Zi /WX /Wall

decode: $(BUILDDIR)\decode.exe
$(BUILDDIR)\decode.exe: $(BUILDDIR) decode.c
	$(CC) $(CFLAGS) decode.c

$(BUILDDIR):
	mkdir $(BUILDDIR)

test: test_listing_0039 test_listing_0040
	echo all tests passed
test_listing_0039: $(TESTDIR)\listing_0039.bin
	diff $(TESTDIR)\listing_0039.bin $(LISTING_0039)
$(TESTDIR)\listing_0039.bin: $(TESTDIR)\listing_0039.asm
	nasm $(TESTDIR)\listing_0039.asm -o $(TESTDIR)\listing_0039.bin
$(TESTDIR)\listing_0039.asm: $(TESTDIR) $(BUILDDIR)\decode.exe
	$(BUILDDIR)\decode.exe $(LISTING_0039) > $(TESTDIR)\listing_0039.asm

test_listing_0040: $(TESTDIR)\listing_0040.bin
	diff $(TESTDIR)\listing_0040.bin $(LISTING_0040)
$(TESTDIR)\listing_0040.bin: $(TESTDIR)\listing_0040.asm
	nasm $(TESTDIR)\listing_0040.asm -o $(TESTDIR)\listing_0040.bin
$(TESTDIR)\listing_0040.asm: $(TESTDIR) $(BUILDDIR)\decode.exe
	$(BUILDDIR)\decode.exe $(LISTING_0040) > $(TESTDIR)\listing_0040.asm

$(TESTDIR):
	mkdir $(TESTDIR)

clean:
	rmdir /S /Q $(BUILDDIR)
	rmdir /S /Q $(TESTDIR)
