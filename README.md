## NOTE: This repository will soon be deprectated
The random map generation is integrated into the latest builds of ScummVM, which is be the preferred way to play Moonbase Commander online. At the time of writing (July 8th 2024) this has been implemented in the ScummVM [daily builds](https://scummvm.org/downloads/#daily).

If you would like to find opponents to play against, join the [Moonbase Commander discord server](https://discord.gg/6J8c7XW).

# INSTRUCTIONS:
- Download MoonbaseConsole.exe from the releases tab
- Place MoonbaseConsole.exe in the same directory as Moonbase.exe and run as administrator.

# README

This is the source code to Moonbase Console, a helper program for [Moonbase Commander](http://en.wikipedia.org/wiki/Moonbase_Commander "Moonbase Commander") that generates random maps with particular parameters, and allows easy set-up of multiplayer matches via TCP/IP.

I (Michael Barlow) didn't write the majority of this code. The code was given to me by Ernie Anderson (MYCROFT), and I originally added this file and the MIT license, and uploaded it to github. I've since added a few more minor changes.

Usage instructions are located in Moonbase Console\README.TXT. You will need to open the following ports to play multiplayer matches:

- 2300 (TCP)
- 2350 (UDP)
- 47624 (TCP)
 
Thanks Jason Creighton for posting this information on your [blog post](http://jcreigh.blogspot.com.au/2009/04/ports-to-forward-to-host-moonbase.html "blog post")

# CREDITS

The following is taken from the "About" dialog within the application.

> A lot of people helped in one way or the other. Here are the main contributors (in alphabetical order)

> Katton designed and coded the Katton map generation algorithm. He decoded a lot of the tiles and published what he learned. He also coded the algorithm that puts the "craters" on the map and wrote the code that generates the map preview showing the options.

> MYCROFT wrote the tools to rip apart the MAP files so they could be deciphered, decoded the rest of the tiles, defined the format that allows the generators to describe the map without caring about tiles, wrote the code to translate the generator output to a useable map, and did the Windows coding.

> SpacemanSpiff designed and coded the Spiff map generation algorithm. He also contributed to deciphering the map file format.

> Special thanks to Bwappo for his contest-winning thumbnail image.

> A number of other people on the Moonbase Commander forums also contributed to map decoding or beta testing including Bwappo, Covak, florent28, Kamolas, llangford, and YorkdinK.
