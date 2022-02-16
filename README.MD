## **NZArchive**
### GUI for NZArchive archive format

NZArchive is an archive format I made for myself but that can be used by anyone.  
    
My first need was fast compression because for myself Speed is more important then size.  
I oriented my choice the Zstandard compression format.  
Zstandard have really fast compression speed with low to intermediate level (Almost compressing to disk speed).  
No matters what the compression level is, it decompress at blazing speed.  
My second need was security.  
I wanna be sure to encrypt data so there will be a little privacy within my desired archive format.  
I oriented my choice to Chacha20, which is quit fast and not easily breakable.  
Then I added AES256 support with TinyAES (Slower than ChaCha20) for those who want it.  
Third, I wanna be sure of data integrity so I add hashing using XXHash (from Zstandard author). The main advantage agains some compression format is that if a file is broken, it will continue extract other file for sure.  
I then created my own structured format for framing file, hashing...  
It's called NZArchive (NZA). (New Zstandard Archive).
UPDATE: 
  * For speed issues, I added windows BCrypt AES Engine (avilable since windows Vista) with machine hardware concurrency  
    If needed, TinyAES can be set at compilation time instead of BCrypt and AES256 encrypted block are interchangeable between TinyAES and BCrypt.  
    You just have to comment/uncoment '#define USEWINDOWS_BCRYPT' line in include\NZArchive\NZArchive.hpp
  * For integrity matters, I created my own parity system, for each 32MiB block of the archive, a certain percentage can be recoverd if data get corrupted (I'm using Wirehair to create recovery block)

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

I provide with the source code my own compiled libraries of Zstandard in x64 / MT / VS2019 (v142).  
You can build your own with the excellent [Packet Manager VCPKG tool from Microsoft](https://vcpkg.io/en/index.html)  