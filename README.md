# BSOUND

This is BSOUND, a little live audio processing tool I have written in C using the portaudio and ncurses libraries. This is my first medium sized project in C, so a lot of the code is horrendous, but I'm slowly improving functionality and making BSOUND slightly better, as I learn more and more about C programming (among other things).


# SYSTEM REQUIREMENTS

This only works on UNIX based systems, because the "GUI" currently uses ncurses. The audio part theoretically works on Windows, as well, but I have only included the portaudio code for mac in this repository.

# OPERATING PRINCIPLE

The idea is very simple: BSOUND takes in audio from your default audio input and sends it through a customizable FX chain and out to your default audio output.

# INSTALLING

Navigate to the downloaded directory, type 'make && make clean' and if no errors come up, start up BSOUND. This expects some frameworks to be in their system default locations (which maybe isn't a good idea) and so on and so forth, but it's worth a shot (and the makefile is so short, I'm sure you can edit it to your heart's content).

The loading and saving expect 4 files for the save slots: saving will just create files if none are in the expected place, but loading will complain, so keep that in mind if you relocate the executable. Likewise, BSOUND expects the "manual" and "welcome text" files included in this repository to be in the same file as the executable.
