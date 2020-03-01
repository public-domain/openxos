          ***************************************************
          *                                                 *
          *          XOS version 3.1.69 Distribution        *
          *                                                 *
          ***************************************************

The CD contains the complete XOS version 3.1.69 source and binary
distribution. It contains a complete set of executable files which can
be used to install a functioning copy of XOS without having to assemble
or link anything.  It also contains all of the files needed to rebuild
everything from source with the exception of the Watcom C compiler.  This
disk is bootable on systems which support booting from CDs.

Inorder to install XOS, all that is needed is to copy the XOS directory
tree from this disk to the root directory of a hard disk and to reaname
one file.  The hard disk must be an IDE disk which is accessable to the
real mode disk BIOS.  On most machines, this means an IDE disk connected
to the primary IDE controller.

The XOS directory tree must be placed on a DOS file system with either
12, 16, or 32-bit FATs.  Any operating system with supports the file
system with long file names (VFAT) and which can read 9660 Joliet CDs can
be used to copy these files.

Proceed as follows:

  1. Copy the entire XOS directory and all subdirectroes from the root of
     this CD to the root of the desired hard disk.

  2. In the \XOS\CFG\ directory:
       Delete the current BOOT.CFG file.
       Rename the BOOT.IDE file to BOOT.CFG.

     The files BOOT.FLP and BOOT.CD are provided for use when creating
     a bootable floppy disk or CD, respectively.  They are not needed
     when installing on a hard disk.

The resulting XOS installation should be runnable on any system, although
you will probably want to modify STARTUP.BAT and/or USTARTUP.BAT to
customize XOS for your system.

In order to run XOS, you must have a bootable disk which contains the XOS
bootstrap.  Initially, this must be either this CD or a floppy disk.  Once
XOS is running, the XOS bootstrap can be installed to the hard disk if
desired. The XOS bootstrap is a partition boot block bootstrap which allows
booting either XOS or the previously installed operating system, if any.
This has been tested with DOS (all versions), Windows95, and Windows98. XOS
can also dual-boot with WindowsXP provided it is installed in a different
partition (which is formated as FAT-15 or FAT-32) from WindowsXP.  The XP
boot manager  must be configured for booting a foreign system. WE *DO NOT*
RECOMEND INSTALLING THE XOS BOOTSTRAP ON TOP OF THE BOOTSTRAP FOR ANY OTHER
OPERATING SYSTEM!

The XOS bootstrap can load XOS from any disk, not just the disk the
bootstrap is loaded from.  Thus, if the bootstrap is loaded from this CD
or a floppy, it can then boot XOS from any disk, including a hard disk
which does not itself contain the XOS bootstrap.

Normally the XOS bootstrap must be installed on the partition which is
active for booting. If a master boot block boot manager is used, however,
the XOS bootstrap can be installed on any partition which the boot manager
can boot. The XOS bootstrap has been tested with the Boot Magic boot
manager and the WindowsXP boot manager, but it should also work with other
boot managers. As noted above, the XOS bootstrap does not have to be on the
same parttion as the XOS directory.

The simplest way to initially run XOS is from this CD, provided your system
can boot from this CD.  Booting this CD loads the XOS bootstrap.  This
bootstrap can then be used to load XOS from this CD or from a hard disk
(assuming you have already copied the files as described above).  To load
from the CD, type <ENTER> to the bootstrap.  This will load a complete
version of XOS from the CD.  This version can then, if desired, be used
to copy the various files to your hard disk, or can simply be run to
evaluate XOS directly without installing any files to your hard disk.

To load XOS from a hard disk, simply type the drive letter followed by a
colon followed by <ENTER>.  Note: If you have several partitions and
multiple hard disks, the bootstrap may not assign drive letters in the
same order as your other operating systems.  If this happens, either
install XOS on the C partition (which should usually be named correctly)
or simply try each letter until you find the correct one. You can also use
the XOS format partition names which have the format DnPm, where n is the
drive number (starting with 0) and m is the partition number (starting
with 1). On most systems the C: partition will be D0P1:.

If your system cannot boot from a CD, you will have to make a boot floppy.
This can be done under DOS or Windows using the MKFLOPPY.EXE program which
is in the root directory of this CD.  The boot floppy must be a 3.5" 1.44MB
floppy disk.  It must be formated.  Any files on the floppy will be erased.
When run, MKFLOPPY will ask you for the floppy image file. Enter
X:BOOTIMG.FLP, where X is the drive letter for the CD on your system. It
will then ask you for the floppy letter.  Enter A or B depending on which
floppy drive contains the disk you want to write.

Once this is finished, you can boot from this floppy.  As descirbed above
for a boot from this CD, this will load the XOS bootstrap.  You can load
XOS directly from the floppy by typeing <ENTER> or from your hard disk (see
above).  It should be noted that the version of XOS booted directly from
the floppy is not complete (due to space limitations on the floppy).  If
you have to boot from a floppy, it is strongly recomended that you first
copy the XOS directory tree to your hard disk and boot that, or use XOS
loaded for the floppy to copy the XOS directry tree to your hard disk. The
XOS commands to do this are:

    MKDIR C:\XOS
    COPY \XOS\...\*.* C:\XOS\...\

assuming you are copying to your C partition.

Note that no networking support is loaded when booting from the floppy disk
or the CD.  This is because the network configuration must be specified before
network access will work.  See the comments just before the call to STARTNET
in STARTUP.BAT (which is commented out) for details.


                    ******* XOS Source Files *******

The XOS source files are in the XOSSRC directory tree.  These files can be
copied to a hard disk as desired.

The following directories are located in the XOSSRC directory:

  BOOT     - Contains sources for the bootstrap and the MKBOOT program.
  DSP      - Contains sources for the display drivers.
  FLOPPY   - Contains sources for the MKFLOPPY program.  (This is the only
             supplied source which cannot be built under XOS.)
  GECKO    - Contains sources for the XOS debugger, GECKO
  KERNEL   - Contains the kernel sources.
  LIBC     - Contains the LIBC sources.  This is mostly the ANSI C library.
  LIBX     - Contains the LIBX sources.  This is mostly XOS extensions to
             the ANSI C library.
  LIBXWC   - Contains sources for the XWC client library.  This is under
             development and is currently very minimal.
  LIBXWS   - Contains sources for the XWS server library.  This is also
             under development but is somewhat further along than the
             client library.
  LKE      - Contains sources for non-network LKEs (Loadable Kernel
             Extensions).
  NETLKE   - Contains sources for network LKEs.
  NETUTIL  - Contains sources for network utilities.
  PCCARD   - Contains sources for the PCCard symbiont.
  SHELL    - Contains sources for the command shell.
  SHOW     - Contains sources for the SHOW command.
  SYSTEM   - Contains sources for system critical user mode programs.
  UDF      - Contains sources for the UDF server.
  UTIL     - Contains sources for utility programs. (This is everything that
             didn't fit someplace else.)
  VID      - Contains sources for the VID text editor.
  XLIB     - Contains sources for the XLIB library utility.
  XLINK    - Contains sources for the XLINK linker.
  XMAC     - Contains sources for the XMAC assembler.
  XMAKE    - Contains sources for the XMAKE make utility.
  XWS      - Contains sources for the XWS server.  This is under development
             and is very incomplete.
  XZIP     - Contains sources for the XZIP and XUNZIP programs.

Additional source files (generally from other freeware distributions) which
are needed to build parts of XOS are included in the XOSAUX directory tree.

The following directories are located in the XOSAUX directory:

  ZLIB     - Contains the ZLIB sources as modified to work with XOS.

    *****************************************************************
    * To compile the XOS C sources, a copy of the Watcom C compiler *
    * version 10.0 or 10.5 is required and is NOT privided on this  *
    * disk.  Only the files WCC386.EXE and W32RUN.EXE are needed.   *
    * These two file should be in a directory which is included in  *
    * the definition of the CMD: logical name.  Later versions of   *
    * this complier have not been tried under XOS, but will likely  *
    * work.  All assembler sources are assembled with XMAC which    *
    * is provided.  All programs (except MKFLOPPY) are linked with  *
    * XLINK which is also provided.                                 *
    *                                                               *
    * Unfortunately, OpenWatcom 1.0 or 1.1 does not currently run   *
    * under XOS.  It is currently planned to port GCC (the GNU C    *
    * compiler) to XOS to replace the Watcom compiler.              *
    *****************************************************************


                      *****************************
                      *                           *
                      *     LEGAL INFORMATION     *
                      *                           *
                      *****************************


ALL XOS SOFTWARE AND DOCUMENTATION PROVIDED IN THIS DISTRIBUTION IS IN THE
PUBLIC DOMAIN.  It may be freely copied and used for whatever purpose you see
fit, including commerical uses.  Anyone modifying this software may claim
ownership of the modifications, but not the complete derived code.  It would
be appreciated if the authors were told what this software is being used for,
but this is not a requirement.

THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

There are a few programs provided on this disk that are not strictly part of
XOS but are used by XOS> These programs are distributed under the terms of
their individual license agreements, which are, in general, more restrictive
than the terms under which XOS itself is distributed. Currently, the only
associated program that is distributed is ZLIB. Please refer to the 
ocumentation for this program for the licensing terms.
