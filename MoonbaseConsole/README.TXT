Moonbase Console v2.1 Documentation
===================================

What is This?
-------------

Moonbase Console is a utility that generates random maps for use with the game Moonbase Commander.



How do I use it?
----------------

Single Player:

If you would like to play a game against bots, just press the "Single Player" button. Moonbase Commander will start automatically. The random map will be the first one in the list and will be selected for you. Just set the other game options however you like and play as you would normally.


Multiplayer:

If you would like to play a game against other people, you need to decide who will host the game.

If you're the host, press the "Host" button. You need to enter your IP address in the "IP Address" field and then press the "Host" button. (If you don't know your IP address, there are some buttons to help you.) A prompt will appear explaining that a game number has been placed on the clipboard. Paste the game number into a message to your opponents using a chat program or an email and send it to them. Once you have done this, press OK on the prompt and Moonbase Commander will start automatically. The random map will be the first one in the list and will be selected for you. Wait for your opponents to join, set all your options the way you like and start the game.

If someone else is the host, get a game number from them and press the "Join" button. The game number must be exactly right, so you should probably copy and paste it instead of trying to type it. If you get any part of the game number wrong, you won't be able to proceed. Paste the game number into the "Game Number" field and press the "Join" button. Moonbase Commander will start automatically.


Random Map Options:

If you are hosting a game or playing a single player game you can control the parameters of the random map. Change the generation algorithm to choose the personality of the map. Set the size of the map to control the length of the game as well as how soon you'll see action. The tileset doesn't affect gameplay, just the graphics used to draw the map. Move the sliders to change the roughness of the terrain, amount of water, and amount of energy on the map. If you want, you can have the parameters selected randomly by clicking the "Random" checkbox under each parameter. If you do this, the most extreme values of the energy, water, and terrain parameters won't be chosen. 



Troubleshooting
---------------

"I tried to play multiplayer, but I can't connect"

First make sure that you can play a normal multiplayer game without Moonbase Console. Then make sure that you're using the right IP address. Moonbase Console doesn't actually do any networking, so if you can play a normal networked game of Moonbase Commander you should be able to use Moonbase Console.

If you can't get a normal game of Moonbase Commander to work, make sure you have the latest version (1.1 was the newest when this document was written). You may also have trouble with a piece of networking technology called a firewall. Check the forums at http://www.strategyplanet.com/mbc for help.


"I want to have two people on my internet connection play at the same time against other people on the internet"

You can't. This has nothing to do with Moonbase Console, Moonbase Commander was not written to allow this.



Questions and Answers
---------------------

* I know what small, medium, large, and huge maps are but what is a SAI, Ridiculous, or MAX map?

Map sizes are measured in multiples of 8. A small map is 32x32, medium is 40x40, large is 48x48, and huge is 56x56. SAI is one of the built-in maps (one of the better multiplayer ones). Interestingly enough it is 64x64, which is a size that can't be made in the map editor.

A Ridiculous map is 72x72 and a MAX map is 80x80. These maps are non-standard. They will crash Moonbase Commander if you load them in the map editor, and the minimap doesn't work perfectly with them (the rectangle doesn't show up). Because of this, if you pick a "Random" size map these two will never be picked. 

* What is a Katton map or a SpacemanSpiff map?

These are the GameSpy handles of the people who developed these different map generation algorithms (see the credits at the end). Each has its own "feel", you should play both to see which one you prefer. Katton maps have no intentional symmetry and never create large energy pools. SpacemanSpiff maps usually have one of several types of symmetry and create all types of energy.

* When I start a single player game, Moonbase Console starts up talking about an internet connection!

Moonbase Console actually handles single player games as multiplayer games with no opponents. Don't worry about it, it was easier to implement that way.

* Do replays work with random maps?

Yes. Make sure that if you give a replay to someone else you also give them the .map file that goes with it.

* Where can I find a copy of Moonbase Commander?

The game can generally only be found in the bargain bin, or on EBay at this point. 

* I got the random maps working fine, but when I play online I lost so badly it wasn't even fun!

Like lots of online games, the level of play has refined quite a bit over time. You should practice againts bots first, if you can't beat them you don't stand a chance online. You should also check the forums at http://www.strategyplanet.com/mbc for tips. Finally there is a collection of replays at http://www.mbc-rankings.com that you can watch. This is a great way to learn the tricks of the masters!
