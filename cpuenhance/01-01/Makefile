# This makefile is for building on Windows with nmake. It is setup to use the
# MSVC compiler. All build artifacts are placed into the build directory to keep
# the root directory clean.

BUILDDIR = build
TESTDIR = test_output
LISTING_0037 = ..\materials\perfaware\part1\listing_0037_single_register_mov
LISTING_0038 = ..\materials\perfaware\part1\listing_0038_many_register_mov
CFLAGS = /Fo$(BUILDDIR)\ /Fd$(BUILDDIR)\ /Fe$(BUILDDIR)\ /Zi /WX /Wall

decode: $(BUILDDIR)\decode.exe
$(BUILDDIR)\decode.exe: $(BUILDDIR) decode.c
	$(CC) $(CFLAGS) decode.c

$(BUILDDIR):
	mkdir $(BUILDDIR)

test: test_listing_0037 test_listing_0038
	echo all tests passed
test_listing_0037: $(TESTDIR)\listing_0037.bin
	diff $(TESTDIR)\listing_0037.bin $(LISTING_0037)
$(TESTDIR)\listing_0037.bin: $(TESTDIR)\listing_0037.asm
	nasm $(TESTDIR)\listing_0037.asm -o $(TESTDIR)\listing_0037.bin
$(TESTDIR)\listing_0037.asm: $(TESTDIR) $(BUILDDIR)\decode.exe
	$(BUILDDIR)\decode.exe $(LISTING_0037) > $(TESTDIR)\listing_0037.asm

test_listing_0038: $(TESTDIR)\listing_0038.bin
	diff $(TESTDIR)\listing_0038.bin $(LISTING_0038)
$(TESTDIR)\listing_0038.bin: $(TESTDIR)\listing_0038.asm
	nasm $(TESTDIR)\listing_0038.asm -o $(TESTDIR)\listing_0038.bin
$(TESTDIR)\listing_0038.asm: $(TESTDIR) $(BUILDDIR)\decode.exe
	$(BUILDDIR)\decode.exe $(LISTING_0038) > $(TESTDIR)\listing_0038.asm

$(TESTDIR):
	mkdir $(TESTDIR)

clean:
	rmdir /S /Q $(BUILDDIR)
	rmdir /S /Q $(TESTDIR)
