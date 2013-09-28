## Things to be awere of

_Windows .idl files_

The DeckLink SDK uses .idl file that are used to genereate code (header/c files).
When you want to use this code, make sure that you have not defined WIN32_LEAN_AND_MEAN
as this seems to give problems with the _interface_ keyword, which is a MSVC++ 
thing. Also, make sure to first include the DeckLink header (<decklink/DeckLink.h>)
and then the windows.h if you need it