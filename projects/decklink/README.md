
# DeckLink SDK wrapper

Download the BlackMagic DeckLink SDK and install it. The SDK only
ships with header files and on windows you get a set of .idl files
which are used to generate a .h and .c file. 

## Install on windows

 - Extract the SDK
 - Copy the *.idl files from _BlackMagic DeckLink SDK #.#.#\Win\include\_ 
   to _projects\decklink\sdk\idl_
 - I also copied the _DeckLinkAPI.dll_ and _DeckLinkAPI64.dll_ from the 
   _BlackMagic Media Express_ application which is found in one of the 
   other downloads from the blackmagic site. Put these dlls in the 
   _projects\decklink\sdk\bin_