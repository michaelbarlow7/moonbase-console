# README

This is the source code to Moonbase Console, a helper program for [Moonbase Commander](http://en.wikipedia.org/wiki/Moonbase_Commander "Moonbase Commander") that generates random maps with particular parameters, and allows easy set-up of multiplayer matches via TCP/IP.

I (Michael Barlow) didn't write this code (though I may add to it in the future). The code was given to me by Ernie Anderson (MYCROFT), and I simply added this file and the MIT license, and uploaded it to github.

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
