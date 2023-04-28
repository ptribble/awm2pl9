This file last changed February 8th, 1989.

In the grand tradition of adding to the README file, rather than
just starting it from scratch...

This represents the 2nd netwide release of awm. A number of bugs in
the menu package have been fixed (though by no means all) and awm
should work much better on color systems. The code has been more
extensively linted and generally cleaned up. Several new bits of
knowledge have made it possible to speed up things quite a bit in
certain situations. Subjective analysis is encouraged.
Many many niggling little problems have been fixed, though nothing
substantial has been changed or added. It was felt that increasing
the reliability of awm was far more important than adding new features,
though many are planned. Awm should now be a good deal more portable,
if you get any warnings (under BSD or SYSV) during compilation, let us
know.

As usual, people are encouraged to send mail to: awm-bugs@ardent.ardent.com
or to the author:

				Jordan Hubbard
                                PCS Computer Systeme GmbH
                                Pfaelzer-Wald-Str. 36
                                D-8000 Muenchen 90.
                                West Germany
				uunet!pyramid!pcsbst!jkh

-------
Old README follows:

This represents the first real release of the Ardent Window Manager (awm).
It's being released on the same terms as its predecessor (uwm) with
one additional request: Since this window manager is the "official"
window manager of Ardent Computer, a lot more effort and time will
be expended to ensure that it works reliably. Please try and coordinate
fixes and enhancements through Ardent so that all may benefit. There
are sure to be some bugs, we'll be here trying to fix them. Send
bug reports to the author, or to:

	{decwrl, uunet, hplabs, ucbcad, dlb}!ardent!awm-bugs


INSTALLATION:

Installation should be fairly straightforward. If you're using imake,
first modify $(TOPDIR)/util/imake.includes/Imake.tmpl to define a macro
called AWMDIR (look at UWMDIR for an example) which should point to
someplace where you want to stash the system.awmrc file. If you like,
you can just make UWMDIR and AWMDIR point to the same place since uwm
and awm's file names are different and won't conflict with eachother. If
you're using make (and don't want to use imake), modify the definition
of AWMDIR in the Makefile instead (sort of the wrong way to do it though).

Certain compilers don't like the #ident lines we use for sccs. If yours
doesn't, do a

	make noident

In the awm directory to remove all #ident lines from the source code.

The usual differences between system V and BSD include file structures
may also cause you trouble. In particular, <sys/time.h> may be different.
If FocusChng.c doesn't compile correctly on your system, see if the
correct include file is in /usr/include or /usr/include/sys (the correct
one should define a timeval struct). I've also heard that SunOS 4.0
fails to compile code that compiles fine on SunOS 3.x. I don't know
what the symptoms are (since I don't have a 4.0 system around), but
I've heard that the fix is to include <sys/file.h> in exp_path.c and
awm.c.

If you want awm's output to go to the console device (assumed to be
/dev/console), define CONSOLE in the I/Makefile (there are appropriate
comments that will show you what to do).

If all this seems confusing, send me mail and I'll try to explain
it differently.

Support for the RTL Neaten package has been added. If you'd like to compile
it in, you need to do two things:

  1. Obtain the RTL neaten package somehow. It's too big to bundle with awm,
     so it's expected that you'll have obtained it by some other means.
     If you are on a system V system with 14 character file names,
     you're in for a bit for work. Many of the files in the neaten
     package have very long names. After you've renamed all of the > 14 char
     files, you'll want to modify Makefile.rtl (in the awm directory)
     identically so that the file names match the new ones you've chosen.

  2. The makefile that comes with the neaten package assumes that you want to
     compile their neatening window manager (nuwm), so you don't want to use
     that. Awm will automatically use the "Makefile.rtl" makefile to compile
     the neaten package (see below), as long as it knows where you've put it.
     Modify awm's Imakefile (or Makefile) to point to the directory
     where neaten resides (the macro NEATEN_LIB) and uncomment/comment the
     appropriate macro definition lines.

  3. Do an imake/make. The make will compile all of awm's files and then
     proceed to make a neaten.a (in the neaten directory) to link against.


   If you don't compile awm with Neaten, the function f.neaten can
   still be bound but it will just print a warning message if invoked.

   Please note that the Neaten package has not been extensively tested with
   awm and should be considered an experimental "frill" more than anything else.
   It seems to shuffle icons around ok, though I don't understand some of its
   window placement logic. If it's useful, use it, if not, don't compile it in.


The rtl menu package in menus (non-optional) has its own Makefile which
you may wish to customize (compiler flags, compiler type, etc), though
the default configuration should produce a working awm binary on your
system. This whole setup needs to be gone through and redone, but that will
have to wait for another day (or a kind volunteer).


First time users of awm will probably want to read the manual page carefully
and then set about tailoring their .Xdefaults file accordingly. The actual
format of the .awmrc file does not differ substantially from uwm's .uwmrc file,
but since much of the variable declaration stuff has been moved into .Xdefaults,
a .uwmrc file will fail miserably if blithly copied into a .awmrc file. It's
probably easier to go from scratch, starting with .Xdefaults.

After defaults have been entered (by far the largest task), a careful
examination of your current uwm interface should be done to see what possible
benefits might be derived from title bar, gadget and border contexts.
You will most likely be able to eliminate almost all "chorded" buttons and
go to naked buttons on title bars/gadgets/borders. You can now also bind
naked buttons to icons without having the button stolen from applications,
so it's usually a win to bind an f.iconify to the icon context if you like the
way X10's xterm used to work.

Highlighting is a new feature which does quite a bit more than just tweak
border colors. It will change the title bar text font (and redisplay the text)
as well as alternating between two different title bar background pixmaps and
or border context pixmaps.

I use a blank pixmap for the regular background (which is the default, I.E.
I don't declare one) and a pixmap containing 7 horizontal lines for the
BoldPixmap.  The effect is not unlike a macintosh window. With some careful
artistry (and placement) of gadget boxes, one could probably emulate this
even more closely, though I'm not sure why one would want to.

When creating gadget box pixmaps, it's suggested that you look at the cursor
font first as there are a lot of suitable glyphs already there.


Any and all suggestions are, of course, appreciated. In addition to the
awm-bugs address given previously, you may communicate with the author
at any of the following addresses:


			Author:		Jordan Hubbard
			Internet:	jkh@violet.berkeley.edu
			UUCP:		{decwrl, hplabs, uunet}!ardent!jkh
			U.S. Mail:	Ardent Computer
					880 Maude
					Sunnyvale, Ca. 94086
