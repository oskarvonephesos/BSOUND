# BSOUND

This is BSOUND, a little live audio processing tool I have written in C using the portaudio and ncurses libraries. This is my first medium sized project in C, so a lot of the code is horrendous, but I'm slowly improving functionality and making BSOUND incrementally better, as I learn more and more about C programming (among other things).


# SYSTEM REQUIREMENTS

This only works on UNIX based systems, because the "GUI" currently uses ncurses. The audio part theoretically works on Windows, as well, but I have only included the portaudio code for mac in this repository.

# OPERATION

At it's core, BSOUND is a very simple application. It takes in audio from your default audio input and sends it through a customizable FX chain and out to your default audio output. Currently, 13 different effects are available with different levels of sophistication and customizability, a little looper and the ability to save effects chains. There is a little pdf manual that goes into more details (it was written for a non-initiated audience).

# COMPILING

on MAC OS X

If you are not a developer:

Download the code and run install. It will check for CommandLineTools and guide you through their installation if necessary. If an executable called bsound does not appear, re-run install.

Else:
In terminal, navigate to the downloaded directory, and run the usual 'make && make clean' and if no errors come up, start up BSOUND. This expects some frameworks to be in their system default locations (which maybe isn't a good idea) and so on and so forth, but the makefile is pretty short, so editing it shouldn't prove too difficult either.

If you prefer portaudio's blocking interface over the (default) callback option, comment out -DUSE_CALLBACK from the CFLAGS in the makefile.

on other UNIX systems:

Before running the makefile download the portaudio code for your operating system and place it in the portaudio folder. Remove the "PA_USE_COREAUDIO" macro from the makefile, remove the (apple) frameworks, and make sure that the listing in the makefile corresponds with the files you need. There is a tutorial for compiling from source on linux in the portaudio repository, so hacking that into the makefile should get you 90% there. Make sure you have ncurses installed and your c compiler knows where to find it.

before relocating the executable:

The loading and saving expect 4 files for the save slots; saving will just create files if none are in the expected place, but loading will complain, so keep that in mind if you relocate the executable. Likewise, BSOUND expects the "manual" and "welcome text" files included in this repository to be in the same directory as the executable (and will likely throw an error otherwise).

# TESTING

If for some strange reason you would like to perform tests on BSOUND, 'make test' will generate an executable for that purpose. Currently, I have only used this code to test  in-out-channel-configurations that I can't test in hardware. 'test' will produce a binary audio file, that the likes of audacity can decode (import raw data:32-bit-float/little-endian(?)) is your friend).
