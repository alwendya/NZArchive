## **NZArchive**
### GUI for NZArchive archive format

NZArchive is an archive format I made for myself but that can be used by anyone.  
    
My first need was fast compression because for myself speed is more important then size.  
I then oriented my choice to the Zstandard compression format.  
Zstandard have really fast compression speed within low and intermediate level (Almost compressing to disk speed), high level is more close to lzma compression (7zip) speed.  
No matters what the compression level is, it decompress at blazing speed.  
My second need was security.  
I wanna be sure to encrypt data so there will be a little privacy within my personnal archive format.  
I oriented my choice to Chacha20, which is quit fast and not easily breakable.  
Laters, I added AES256 support with TinyAES (Slower than ChaCha20) for those who want it.  
Third, I wanna be sure of data integrity so I add hashing using XXHash (from Zstandard author).  
The main advantage against some other compression format is that if a file inside the archive is broken,  
it will continue to extract other file for sure.  
I then created my own structured format (Headers containing file framing, file hashing, attributes...) : It's called NZArchive (New Zstandard Archive aka NZA).  
UPDATE:  
  * For speed issues, I added Windows BCrypt AES Engine (available since windows Vista) with hardware level encrypting/decrypting  
    If needed, TinyAES can still be used even if it is slower because of software encrypting/decrypting.  
    BCrypt and TinyAES encrypted block are the same inside, NZArchive will always read the file from both builds.    
    ( You just have to comment/uncomment '#define USEWINDOWS_BCRYPT' line in include\NZArchive\NZArchive.hpp )  
  * For integrity matters, I created my own parity system.  
    For each 32MiB block of the archive, a certain percentage can be recoverd if data get corrupted.  
    (I'm using Wirehair library to create recovery block)  
  
Feel free to use it, I'll be happy.  
I placed it in MIT licence, keeping trace of it in your program would be nice...  
  
Library used :
  
  * [Zstandard](https://github.com/facebook/zstd)  
    by Yann Collet. BSD Licence  
    
  * [ChaCha20](https://github.com/cooky451/chacha)  
    by Cooky451. MIT Licence  
    
  * [XXHash](https://github.com/Cyan4973/xxHash)  
    by Yann Collet. BSD2 Clause Licence  
    
  * [Tiny-AES](https://github.com/kokke/tiny-AES-c)  
    by Kokke. Unlicence licence  

  * [WireHair](https://github.com/catid/wirehair)  
    by CatID. BSD 3-Clause License 
    
Fonts used :
  
  * Roboto Condensed - [Google fonts](https://fonts.google.com/specimen/Roboto)  
    by Christian Robertson. Apache License, Version 2.0.
  
I provide with the source code my own compiled libraries of Zstandard in x64 / MT / VS2019 (v142).  
You can build your own with the excellent [Packet Manager VCPKG tool from Microsoft](https://vcpkg.io/en/index.html)  