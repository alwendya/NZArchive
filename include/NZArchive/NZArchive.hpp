#pragma once
#ifndef NZARCHIVE_HEADER
#define NZARCHIVE_HEADER
#endif // !NZARCHIVE_HEADER

/*
::::    ::: :::::::::     :::     :::::::::   ::::::::  :::    ::: ::::::::::: :::     ::: ::::::::::
:+:+:   :+:      :+:    :+: :+:   :+:    :+: :+:    :+: :+:    :+:     :+:     :+:     :+: :+:
:+:+:+  +:+     +:+    +:+   +:+  +:+    +:+ +:+        +:+    +:+     +:+     +:+     +:+ +:+
+#+ +:+ +#+    +#+    +#++:++#++: +#++:++#:  +#+        +#++:++#++     +#+     +#+     +:+ +#++:++#
+#+  +#+#+#   +#+     +#+     +#+ +#+    +#+ +#+        +#+    +#+     +#+      +#+   +#+  +#+
#+#   #+#+#  #+#      #+#     #+# #+#    #+# #+#    #+# #+#    #+#     #+#       #+#+#+#   #+#
###    #### ######### ###     ### ###    ###  ########  ###    ### ###########     ###     ##########

-----------------------------------------------------------------------------------------------------

Header only library

NZAʀᴄʜɪᴠᴇ is a fast and encrypted archive format using ZStandard for fast compression
and blazing fast decompression no matter the compression level.
Encryption is using 20 round Chacha 256 bits or AES 256 buts.
Written in C++17 and oriented to 64 bits application.


▄ External library used ▄
-------------------------
	- ZSTD		: ZStandard compression
	- Bcrypt	: Windows NextGen compression library (present starting Windows Vista, can be replaced by TinyAES if needed)

▄ Using this internal library ▄
-------------------------------
	- Wirehair	: Library for generating code recovery for incomplete file
	- XXHASH	: Hashing library

▄ Revisionning ▄
----------------
	Version 1.5.0	:	Optimization of Wirehair function inside NZArchive.
	Version 1.4.0	:	Integration of Wirehair to create receovery file.
						For each 32MiB, a percentage of extra block is created and allow
						to recover damaged part of the 32Mib block.
	Version 1.3.0	:	Modification of TinyAES256 Encryot/Decrypt function in CBC Mode
						to ensure compatibility with Archive using BCrypt (And the invert too).
						by creating a padding method to ensure block s a multiple of 16 bytes
						(Standard definition of AES256 block size)
	Version 1.2.0	:	Added AES256 support with BCrypt
						(Hardware AES256 encryption/decryption CBC mode built in Windows starting Vista edition)
	Version 1.1.0	:	Change header method, now it's a separate chunk of byte,
						compressed and crypted at the end of the NZArchive
	Version 1.0.0	:	First milestone, I think's it's stable enough for daily usage
	Version 0.9.0	:	Added AES256 support with TinyAES (Software AES256 encryption/decryption CTR mode)
	Version 0.8.0	:	Total rewriting of function name for easier comprehension and usage
	Version 0.7.0	:	Implementation of function to get file list of archive in a readable way
	Version 0.6.0	:	Implementation of Unicode file support with std::wstring to
						std::byte (unsigned char) conversion to save and read unicode encoded namefrom/to file
	Version 0.5.0	:	Usage of mutex lock for interthread call. eg. progress report from a detached thread
	Version 0.4.0	:	Implementation of ZStandard 1.5.0 for speed and reliable compression
	Version 0.3.0	:	Implementation of XX3Hash 0.8.0 to ensure data integrity
	Version 0.2.0	:	Implementation of ChaCha20 256bits 20 round for encryption
	Version 0.1.0	:	Definition of NZArchive standard, only copy fonction for the moment

▄ TODO ▄
--------
	- Optimization...
	- Error management in a log file
*/
#define NZARCHIVE_MAJOR		1
#define NZARCHIVE_MINOR		5
#define NZARCHIVE_REVISION	0

/*
If USEWINDOWS_BCRYPT is defined, NZArchive will use windows NGEN library for AES256 encryption (Hardware => Faster)
if USEWINDOWS_BCRYPT is not defined, NZArchive wil use TinyAES (No windows dependency) fro AE256 encryption (Software => Slower)
It's your choice to uncomment or not next line...
*/
#define USEWINDOWS_BCRYPT


/*	  _____            _           _
	  \_   \_ __   ___| |_   _  __| | ___
	   / /\/ '_ \ / __| | | | |/ _` |/ _ \
	/\/ /_ | | | | (__| | |_| | (_| |  __/
	\____/ |_| |_|\___|_|\__,_|\__,_|\___|	*/
#include <iostream>				// Basic IO function
#include <vector>				// Vector management
#include <string>				// Standard string
#include <fstream>				// File seeking function
#include <filesystem>			// All about file information, copying, iterating
#include <algorithm>			// Random things
#include <mutex>				// Interthread locking
#include <io.h>					// _get_osfhandle 
#include "chacha.hpp"			// https://github.com/cooky451/chacha	MIT Licence
#include "xxHash/xxh3.h"		// https://github.com/Cyan4973/xxHash	BSD2 Clause licence
#define ZSTD_STATIC_LINKING_ONLY
#include "Zstd/zstd.h"			// https://github.com/facebook/zstd	BSD Licence
#include <wirehair/wirehair.h>	// Code error correction generator
#ifdef USEWINDOWS_BCRYPT
#include <bcrypt.h>				// Windows AES library (strating Vista) [hardware encryption/decryption]
#pragma comment (lib, "bcrypt.lib")
#else
#include <tinyaes.hpp>			// https://github.com/kokke/tiny-AES-c	The Unlicense licence (Public) [software encryption/decryption]
#endif // USEWINDOWS_BCRYPT
#define MutexGET(Mutex,ReadVariable,OutVariable) Mutex.lock();OutVariable = ReadVariable ; Mutex.unlock(); 

	/*	 _____ _____ _____         _   _
		|   | |__   |  _  |___ ___| |_|_|_ _ ___    ___ ___ _____ ___ ___ ___ ___ ___ ___
		| | | |   __|     |  _|  _|   | | | | -_|  |   | .'|     | -_|_ -| . | .'|  _| -_|
		|_|___|_____|__|__|_| |___|_|_|_|\_/|___|  |_|_|__,|_|_|_|___|___|  _|__,|___|___|
																			|_|
	*/
	/// @brief	NZAʀᴄʜɪᴠᴇ namespace containing Archive class
namespace NZArchive
{
	/*	 _____             _   _                    _      _         _   _        _   _              _
		|   __|_ _ ___ ___| |_|_|___ ___    ___ ___| |_   |_|___ ___|_|_| |___   | |_| |_ ___    ___| |___ ___ ___ ___ ___
		|   __| | |   |  _|  _| | . |   |  |   | . |  _|  | |   |_ -| | . | -_|  |  _|   | -_|  |  _| | .'|_ -|_ -| -_|_ -|
		|__|  |___|_|_|___|_| |_|___|_|_|  |_|_|___|_|    |_|_|_|___|_|___|___|  |_| |_|_|___|  |___|_|__,|___|___|___|___|
	*/
#pragma region NotInsideClasses
	#pragma region MEM_Operation
		/// @brief      Read uint16_t (2 byte) data from a memory pointer with a minimum size of 2 byte.
		inline uint16_t MEM_read16(const void* memPtr)
		{
			uint16_t val; memcpy_s(&val, sizeof(val), memPtr, sizeof(val)); return val;
		}
		/// @brief      Read uint32_t (4 byte) data from a memory pointer with a minimum size of 4 byte.
		inline uint32_t MEM_read32(const void* memPtr)
		{
			uint32_t val; memcpy_s(&val, sizeof(val), memPtr, sizeof(val)); return val;
		}
		/// @brief      Read uint64_t (8 byte) data from a memory pointer with a minimum size of 8 byte.
		inline uint64_t MEM_read64(const void* memPtr)
		{
			uint64_t val; memcpy_s(&val, sizeof(val), memPtr, sizeof(val)); return val;
		}
		/// @brief      Write uint16_t (2 byte) data in a memory pointer with a minimum size of 2 byte.
		inline void MEM_write16(void* memPtr, uint16_t value)
		{
			memcpy_s(memPtr, sizeof(value), &value, sizeof(value));
		}
		/// @brief      Write uint32_t (4 byte) data in a memory pointer with a minimum size of 4 byte.
		inline void MEM_write32(void* memPtr, uint32_t value)
		{
			memcpy_s(memPtr, sizeof(value), &value, sizeof(value));
		}
		/// @brief      Write uint64_t (8 byte) data in a memory pointer with a minimum size of 8 byte.
		inline void MEM_write64(void* memPtr, uint64_t value)
		{
			memcpy_s(memPtr, sizeof(value), &value, sizeof(value));
		}
	#pragma endregion
	#pragma region FILE_Operation
		/*		ᴜɪɴᴛ_xx ғᴏʀᴍᴀᴛᴛɪɴɢ ᴀɴᴅ sɪᴢɪɴɢ ɪɴ ᴀ ғɪʟᴇ
		*		---------------------------------------
		*
		*		uint_16t	Min 𝟎 Max 𝟔𝟓𝟓𝟑𝟓						Representation in file	Min 𝟎𝟎𝟎𝟎				Max 𝐅𝐅𝐅𝐅				Size in unsigned char : 𝟐
		*		uint_32t	Min 𝟎 Max 𝟒𝟐𝟗𝟒𝟗𝟔𝟕𝟐𝟗𝟓				Representation in file	Min 𝟎𝟎𝟎𝟎𝟎𝟎𝟎𝟎			Max 𝐅𝐅𝐅𝐅𝐅𝐅𝐅𝐅            Size in unsigned char : 𝟒
		*		uint_64t	Min 𝟎 Max 𝟏𝟖𝟒𝟒𝟔𝟕𝟒𝟒𝟎𝟕𝟑𝟕𝟎𝟗𝟓𝟓𝟏𝟔𝟏𝟓	Representation in file	Min 𝟎𝟎𝟎𝟎𝟎𝟎𝟎𝟎𝟎𝟎𝟎𝟎𝟎𝟎𝟎𝟎	Max 𝐅𝐅𝐅𝐅𝐅𝐅𝐅𝐅𝐅𝐅𝐅𝐅𝐅𝐅𝐅𝐅    Size in unsigned char : 𝟖	*/

		/// @brief      Read uint16_t (2 byte) data from a memory pointer with a minimum size of 2 byte from a FILE*.
		inline uint16_t FILE_read16(FILE* _File)
		{
			std::vector<uint8_t> vec_uint16;
			vec_uint16.resize(sizeof(uint16_t));
			fread_s(&vec_uint16[0], sizeof(uint16_t), sizeof(uint8_t), sizeof(uint16_t), _File);
			uint16_t val; memcpy_s(&val, sizeof(val), &vec_uint16[0], sizeof(val));
			vec_uint16.clear();
			return val;
		}
		/// @brief      Write uint16_t (2 byte) data from a memory pointer with a minimum size of 2 byte to a FILE*.
		inline void FILE_write16(FILE* _File, uint16_t val)
		{
			std::vector<uint8_t> vec_uint16;
			vec_uint16.resize(sizeof(uint16_t));
			MEM_write16(&vec_uint16[0], val);
			fwrite(&vec_uint16[0], sizeof(uint8_t), sizeof(uint16_t), _File);
			vec_uint16.clear();
		}
		/// @brief      Read uint32_t (4 byte) data from a memory pointer with a minimum size of 4 byte from a FILE*.
		inline uint32_t FILE_read32(FILE* _File)
		{
			std::vector<uint8_t> vec_uint32;
			vec_uint32.resize(sizeof(uint32_t));
			fread_s(&vec_uint32[0], sizeof(uint32_t), sizeof(uint8_t), sizeof(uint32_t), _File);
			uint32_t val; memcpy_s(&val, sizeof(val), &vec_uint32[0], sizeof(val));
			vec_uint32.clear();
			return val;
		}
		/// @brief      Write uint32_t (4 byte) data from a memory pointer with a minimum size of 4 byte to a FILE*.
		inline 	void FILE_write32(FILE* _File, uint32_t val)
		{
			std::vector<uint8_t> vec_uint32;
			vec_uint32.resize(sizeof(uint32_t));
			MEM_write32(&vec_uint32[0], val);
			fwrite(&vec_uint32[0], sizeof(uint8_t), sizeof(uint32_t), _File);
			vec_uint32.clear();
		}
		/// @brief      Read uint64_t (8 byte) data from a memory pointer with a minimum size of 8 byte from a FILE*.
		inline 	uint64_t FILE_read64(FILE* _File)
		{
			std::vector<uint8_t> vec_uint64;
			vec_uint64.resize(sizeof(uint64_t));
			fread_s(&vec_uint64[0], sizeof(uint64_t), sizeof(uint8_t), sizeof(uint64_t), _File);
			uint64_t val; memcpy_s(&val, sizeof(val), &vec_uint64[0], sizeof(val));
			vec_uint64.clear();
			return val;
		}
		/// @brief      Write uint64_t (8 byte) data from a memory pointer with a minimum size of 8 byte to a FILE*.
		inline void FILE_write64(FILE* _File, uint64_t val)
		{
			std::vector<uint8_t> vec_uint64;
			vec_uint64.resize(sizeof(uint64_t));
			MEM_write64(&vec_uint64[0], val);
			fwrite(&vec_uint64[0], sizeof(uint8_t), sizeof(uint64_t), _File);
			vec_uint64.clear();
		}
	#pragma endregion

		/// @brief	Structure with NPAR file information
		/// @param	PercentRedundancy	Percentage of error correction block
		/// @param	kMessageBytes		Block size
		/// @param	kPacketSize		Packet size
		/// @param	FILESize			Archive size
		/// @param	TABLEFILESize		NPAR file size
		struct NPARInformation
	{
		uint64_t PercentRedundancy = 0;
		uint64_t kMessageBytes = 0;
		uint64_t kPacketSize = 0;
		uint64_t FILESize = 0;
		uint64_t TABLEFILESize = 0;
	};


	/// @brief	Retrieve the percentage of Error correction block within a NPAR file
	/// @param	_File	NPAR file to retrieve percentage of Error correction block
	/// @return	UINT64_T with the percentage (Retrieve 0 if file doesn't exist or is unreadable)
	NPARInformation WIREHAIR_RetrieveNPARInformation(std::wstring _File)
	{
		NPARInformation lNPARInformation;
		FILE* fSRC = NULL;
		_wfopen_s(&fSRC, _File.c_str(), L"rb");
		if (fSRC == NULL)
			return lNPARInformation;
		else
		{
			/*
				_fseeki64(fTable, 0, SEEK_END);
				uint64_t TableFileSize = _ftelli64(fTable);
				_fseeki64(fTable, 0, SEEK_SET);
				mTable.PercentRedundancy = FILE_read64(fTable);
				mTable.kMessageBytes = FILE_read64(fTable);
				mTable.kPacketSize = FILE_read64(fTable);
				mTable.FILESize = FILE_read64(fTable);

			*/
			_fseeki64(fSRC, 0, SEEK_END);
			if (_ftelli64(fSRC) < 4)
				return lNPARInformation;
			lNPARInformation.TABLEFILESize = _ftelli64(fSRC);
			_fseeki64(fSRC, 0, SEEK_SET);
			lNPARInformation.PercentRedundancy = FILE_read64(fSRC);
			lNPARInformation.kMessageBytes = FILE_read64(fSRC);
			lNPARInformation.kPacketSize = FILE_read64(fSRC);
			lNPARInformation.FILESize = FILE_read64(fSRC);
			fclose(fSRC);
			return lNPARInformation;
		}
		return lNPARInformation;
	}

	/// @brief      Estimate the memory usage of a ZSTD context with a given compression level and thread numbers.
	/// @param  BufferSize		Size of data to be compressed 
	/// @param  CompressionLevel	Compression level (From 1 to 22)
	/// @param  ThreadNumber		Number of thread used for compression
	/// @return     size_t, with estimated memory usage in bytes
	size_t ZSTD_EstimateMemoryUsage(size_t BufferSize, size_t CompressionLevel, size_t ThreadNumber)
	{
		return ((ThreadNumber > 1) ? ThreadNumber * ZSTD_estimateCCtxSize(CompressionLevel) : ZSTD_estimateCCtxSize(CompressionLevel)) + BufferSize + ZSTD_compressBound(BufferSize) + ZSTD_CStreamInSize() + ZSTD_CStreamOutSize();
	}

	/// @brief      Truncate archive FILE* from a number of byte. 
	/// @param	_ArchiveFILE	Pointer to FILE (Archive file)
	/// @param	_byteToRemove	Number of byte to remove from end (Truncate)
	/// @return     True if truncate success, False if failed
	bool UTIL_truncate_file_by(FILE* _ArchiveFILE, UINT64 _byteToRemove)
	{
		if (_ArchiveFILE == NULL)
			return false;
		_fseeki64(_ArchiveFILE, 0, SEEK_END);//We go to the end 
		UINT64 fSize = _ftelli64(_ArchiveFILE);
		HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(_ArchiveFILE));
		LARGE_INTEGER nFileSize;
		int64_t nEndPos = (int64_t)fSize - _byteToRemove;//Truncate at 400Mo
		nFileSize.u.LowPart = nEndPos & 0xFFFFFFFF;
		nFileSize.u.HighPart = nEndPos >> 32;
		SetFilePointerEx(hFile, nFileSize, 0, FILE_BEGIN);
		SetEndOfFile(hFile);
		return true;
	}


#pragma endregion

	/*	 _____ _____ _____         _   _            _____ __    _____ _____ _____
		|   | |__   |  _  |___ ___| |_|_|_ _ ___   |     |  |  |  _  |   __|   __|
		| | | |   __|     |  _|  _|   | | | | -_|  |   --|  |__|     |__   |__   |
		|_|___|_____|__|__|_| |___|_|_|_|\_/|___|  |_____|_____|__|__|_____|_____|
	*/

	/// @brief	NZArchive, Major version int
	constexpr int NZArchiveMajor = NZARCHIVE_MAJOR;
	/// @brief	NZArchive, Minor version int
	constexpr int NZArchiveMinus = NZARCHIVE_MINOR;
	/// @brief	NZArchive, Revision version int
	constexpr int NZArchiveRevision = NZARCHIVE_REVISION;

	/// @brief	NZArchive, Magic byte definition
	std::uint8_t magicByte[8] = { 0x4e, 0x5a, 0x41, 0x1f, 0xd5, 0x8a, 0x71, 0xbe };

	/// @brief	Error status for file adding operation
	enum class ADDFILE_ERROR : int8_t {
		ADDFILE_SUCCESS = 0, ADDFILE_FILEPOINTER_NULL = -1, ADDFILE_FILENOTEXIST = -2, ADDFILE_CANTOPENFILE = -4, ADDFILE_FATALERROR = -5
	};

	/// @brief	Error status for file extraction operation
	enum class OP_EXTRACT : uint8_t {
		OP_EXTRACT_SUCCESS = 1, OP_EXTRACT_CANTCREATEOUTFILE = 0, OP_EXTRACT_CANTOPENARCHIVE = 2, OP_EXTRACT_CANTOPENPASSWORDERROR = 3, OP_EXTRACT_HASHERRORS = 4
	};

	/// @brief      Enumeration of encryption method.
	enum class EncryptionMethod : uint8_t {
		encNone = 1, encChaCha20 = 2, encAES256 = 4
	};

	/// @brief      Error status for ZSTD_stream compression / decompression
	enum class ZSTD_stream_error : uint8_t {
		ZS_NoError, ZS_CCTXError, ZS_SourceEmpty, ZS_ErrorConsumingInput, ZS_DCTXError
	};
	/// @brief      Archive class declaration
	class Archive
	{
	public:
		/// @brief	Structure representing operation progression
		/// @param	Progress		Operation progress
		/// @param	Current		Current position
		/// @param	Total		Total position
		/// @param	CurrentFile	Current file treated		
		struct _ProgressReporting
		{
			double Progress = 0.0;
			uint64_t Current = 0;
			uint64_t Total = 0;
			std::wstring CurrentFile = L"";
		};


		/// @brief      Compress and encrypt using Chacha20 encryption a vector of std::byte into a second vector of std::byte with a given level of compression.
		///				Retrieve additional information such as hash of input and output vector of std::byte
		///				If _VectorOut is null sized after compression, it means operation failed.
		/// @param  _VectorIn		Vector of std::byte to be compressed
		/// @param  _VectorOut		Vector of std::byte, empty or not, who will be filled with compressed data
		/// @param  _HashIn			Computed XXHASH64 of uncompressed vector
		/// @param  _HashOut		Computed XXHASH64 of compressed vector
		/// @param  _LevelCompression	From 0 (FAST) to 22 (MAX)
		/// @param  mCryptInChacha	True: Encrypt with ChaCha20, False: Encrypt with AES256
		void CompAndCrypt(std::vector<std::byte>& _VectorIn, std::vector<std::byte>& _VectorOut, XXH64_hash_t& _HashIn, XXH64_hash_t& _HashOut, int _LevelCompression, EncryptionMethod mCryptMethod)
		{
			_VectorOut.clear();
			_HashIn = 0;
			_HashOut = 0;
			if (_VectorIn.size() == 0)
				return;
			_HashIn = XXH3_64bits(&_VectorIn[0], _VectorIn.size());
			auto cStatus = ZSTD_stream_compress(_VectorIn, _VectorOut, _LevelCompression, mMaxThread);
			if (cStatus != ZSTD_stream_error::ZS_NoError)
				return;
			if (mCryptMethod == EncryptionMethod::encChaCha20)
			{
				chacha::buffered_cipher cipher(chacha::key_bits_256, chacha::rounds20, &mVectorKEY[0], mNonce/*Nonce de l'archive*/);
				cipher.transform(&_VectorOut[0], &_VectorOut[0], _VectorOut.size());
			}
			else if (mCryptMethod == EncryptionMethod::encAES256)
			{
				std::vector<std::byte> vIv;
				vIv.resize(16);
				MEM_write64(&vIv[0], mNonce);//uint64_t is 8byte long
				MEM_write64(&vIv[8], mNonce);
#ifdef USEWINDOWS_BCRYPT
				//Windows NewGen BCrypt engine
				std::vector<std::byte> vCrypted;
				BCryptEncryptVector(_VectorOut, vCrypted, vIv, mVectorKEY);
				_VectorOut.clear();
				_VectorOut.resize(vCrypted.size());
				memcpy_s(&_VectorOut[0], vCrypted.size(), &vCrypted[0], vCrypted.size());
				vCrypted.clear();
#else
				//TinyAES engine
				struct AES_ctx ctx;
				AES_init_ctx_iv(&ctx, reinterpret_cast<uint8_t*>(&mVectorKEY[0]), reinterpret_cast<uint8_t*>(&vIv[0]));
				AES256AddPadding(_VectorOut);
				AES_CBC_encrypt_buffer(&ctx, reinterpret_cast<uint8_t*>(&_VectorOut[0]), _VectorOut.size());
#endif // USEWINDOWS_BCRYPT
				vIv.clear();

			}
			else if (mCryptMethod == EncryptionMethod::encNone)
			{
			}
			_HashOut = XXH3_64bits(&_VectorOut[0], _VectorOut.size());
			//vIv.clear();
		}

		/// @brief  Decrypt and Uncompress using Chacha20 encryption a vector of std::byte into a second vector of std::byte.
		///			Retrieve additional information such as hash of input and output vector of std::byte
		/// @param  _VectorIn		Vector of std::byte with compressed and encrypted data
		/// @param  _VectorOut		Vector of std::byte, MUST BE SIZED TO UNCOMPRESSED DATA LENGTH, who will be filled with uncompressed data
		/// @param  _HashIn			Computed XXHASH64 of uncompressed vector
		/// @param  _HashOut		Computed XXHASH64 of compressed vector
		/// @return     Boolean, true if decompression is successful, false if something went wrong
		bool DecompAnDecrypt(std::vector<std::byte>& _VectorIn, std::vector<std::byte>& _VectorOut, XXH64_hash_t& _HashIn, XXH64_hash_t& _HashOut, EncryptionMethod mDecryptMethod)
		{
			_HashIn = 0;
			_HashOut = 0;
			if (_VectorIn.size() == 0)
				return false;
			_HashIn = XXH3_64bits(&_VectorIn[0], _VectorIn.size());
			if (mDecryptMethod == EncryptionMethod::encChaCha20)
			{
				chacha::buffered_cipher cipher(chacha::key_bits_256, chacha::rounds20, &mVectorKEY[0], mNonce/*Nonce de l'archive*/);
				cipher.transform(&_VectorIn[0], &_VectorIn[0], _VectorIn.size());
			}
			else if (mDecryptMethod == EncryptionMethod::encAES256)
			{
				std::vector<std::byte> vIv;
				vIv.resize(16);
				MEM_write64(&vIv[0], mNonce);
				MEM_write64(&vIv[8], mNonce);
#ifdef USEWINDOWS_BCRYPT
				//Windows NewGen BCrypt engine
				std::vector<std::byte> vDecrypted;
				BCryptDecryptVector(_VectorIn, vDecrypted, vIv, mVectorKEY);
				_VectorIn.clear();
				_VectorIn.resize(vDecrypted.size());
				memcpy_s(&_VectorIn[0], vDecrypted.size(), &vDecrypted[0], vDecrypted.size());
				vDecrypted.clear();
#else
				//TinyAES Engine
				struct AES_ctx ctx;
				AES_init_ctx_iv(&ctx, reinterpret_cast<uint8_t*>(&mVectorKEY[0]), reinterpret_cast<uint8_t*>(&vIv[0]));
				AES_CBC_decrypt_buffer(&ctx, reinterpret_cast<uint8_t*>(&_VectorIn[0]), _VectorIn.size());
				AddAES256RemovePadding(_VectorIn);
#endif // USEWINDOWS_BCRYPT
				vIv.clear();
			}
			else if (mDecryptMethod == EncryptionMethod::encNone)
			{
			}
			//size_t const cSize = ZSTD_decompress(&_VectorOut[0], _VectorOut.size(), &_VectorIn[0], _VectorIn.size());
			auto cStatus = ZSTD_stream_uncompress(_VectorIn, _VectorOut);
			_HashOut = XXH3_64bits(&_VectorOut[0], _VectorOut.size());
			if (cStatus != ZSTD_stream_error::ZS_NoError)
				return false;
			return true;
		}

		/// @brief      Write archive header to FILE*. Also Set progression to 0 and restart the header recording of file
		/// @param  _ArchiveFILE		Pointer to a FILE* representing output archive
		/// @param  Major, _Minus, _Revision : Set Version with these arguments
		/// @return     True if write success, False if failed
		bool archiveNZ_write_header(FILE* _ArchiveFILE, uint16_t _Major = NZArchiveMajor, uint16_t _Minus = NZArchiveMinus, uint16_t _Revision = NZArchiveRevision)
		{
			if (_ArchiveFILE == NULL)
				return false;
			mHeader.Header.clear();
			mHeader.Position = 0;
			MutexComp.lock();
			mProgressComp.Current = 0;
			mProgressComp.Total = 0;
			MutexComp.unlock();
			fwrite(NZArchive::magicByte, sizeof(NZArchive::magicByte), 1, _ArchiveFILE);//Define this archive as NZA
			HEADER_write16(_Major);
			HEADER_write16(_Minus);
			HEADER_write16(_Revision);
			return true;
		}


		/// @brief      Write a file from wstring path to given FILE* archive at the given filepath emplacement in the archive with a given compression level in multithread mode.
		///             Buffer can be adapted.
		/// @param  _ArchiveFILE		Pointer to a FILE* representing output archive
		/// @param  _FileToAdd		Path as wstring to the source file
		/// @param  _FilePathInArchive	Path as wstring to the file emplacement in the archive
		/// @param  _CompressionLevel	From 0 (FAST) to 22 (MAX)
		/// @param  _BUFFER		Defaut 8mb, can be modified
		/// @return     Return ADDFILE_XXXX constant representing operation state:
		///
		/// 	ADDFILE_FILEPOINTER_NULL = (Invalide FILE*); 
		/// 	ADDFILE_FILENOTEXIST = (Source file don't exist);
		/// 	ADDFILE_CANTOPENFILE = (Source file can't be read);
		/// 	ADDFILE_FATALERROR = (Unknown error);
		/// 	ADDFILE_SUCCESS = (Operation success);
		ADDFILE_ERROR archiveNZ_write_add_file(FILE* _ArchiveFILE, std::wstring _FileToAdd, std::wstring _FilePathInArchive, int _CompressionLevel = 3, EncryptionMethod mEncryptMethod = EncryptionMethod::encChaCha20, uint64_t _BUFFER = 32 * 1024 * 1024)
		{
			if (_ArchiveFILE == NULL)
				return ADDFILE_ERROR::ADDFILE_FILEPOINTER_NULL;
			if (_FileToAdd == L"" && _FilePathInArchive != L"")//create dummy directory
			{
				MutexComp.lock();
				mProgressComp.CurrentFile = _FileToAdd;
				MutexComp.unlock();
				HEADER_write16(1);//1 Mean a directory
				HEADER_write16((uint16_t)mEncryptMethod);
				archiveNZ_write_wstring(_FilePathInArchive);
				FILETIME  fCreate;
				FILETIME  fMod;
				FILETIME  fLast;
				std::time_t _now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				TimetToFileTime(_now, &fCreate);
				TimetToFileTime(_now, &fMod);
				TimetToFileTime(_now, &fLast);
				HEADER_write64(fLast.dwLowDateTime);
				HEADER_write64(fLast.dwHighDateTime);
				HEADER_write64(fCreate.dwLowDateTime);
				HEADER_write64(fCreate.dwHighDateTime);
				HEADER_write64(fMod.dwLowDateTime);
				HEADER_write64(fMod.dwHighDateTime);
				HEADER_write64((DWORD)FILE_ATTRIBUTE_NORMAL);
				return ADDFILE_ERROR::ADDFILE_SUCCESS;
			}
			else if (std::filesystem::exists(_FileToAdd))
			{
				MutexComp.lock();
				mProgressComp.CurrentFile = _FileToAdd;
				MutexComp.unlock();
				if (std::filesystem::is_directory(_FileToAdd))
				{
					HEADER_write16(1);//1 Mean a directory
					HEADER_write16((uint16_t)mEncryptMethod);
					archiveNZ_write_wstring(_FilePathInArchive);
					struct _stati64 mStati64;
					int Analyse = _wstati64(_FileToAdd.c_str(), &mStati64);

					FILETIME  fCreate;
					FILETIME  fMod;
					FILETIME  fLast;

					TimetToFileTime(mStati64.st_ctime, &fCreate);
					TimetToFileTime(mStati64.st_mtime, &fMod);
					TimetToFileTime(mStati64.st_atime, &fLast);


					//CloseHandle(hHandle);
					HEADER_write64(fLast.dwLowDateTime);
					HEADER_write64(fLast.dwHighDateTime);
					HEADER_write64(fCreate.dwLowDateTime);
					HEADER_write64(fCreate.dwHighDateTime);
					HEADER_write64(fMod.dwLowDateTime);
					HEADER_write64(fMod.dwHighDateTime);
					DWORD FileAttr = GetFileAttributes(_FileToAdd.c_str());
					HEADER_write64(FileAttr);
					return ADDFILE_ERROR::ADDFILE_SUCCESS;
				}
				else
				{
					FILE* fIN = NULL;
					_wfopen_s(&fIN, _FileToAdd.c_str(), L"rb");
					if (fIN != NULL)
					{
						HEADER_write16(2);//1 Mean a file
						HEADER_write16((uint16_t)mEncryptMethod);
						archiveNZ_write_wstring(_FilePathInArchive);
						struct _stati64 mStati64;
						int Analyse = _wstati64(_FileToAdd.c_str(), &mStati64);
						FILETIME  fCreate;
						FILETIME  fMod;
						FILETIME  fLast;
						TimetToFileTime(mStati64.st_ctime, &fCreate);
						TimetToFileTime(mStati64.st_mtime, &fMod);
						TimetToFileTime(mStati64.st_atime, &fLast);
						HEADER_write64(fLast.dwLowDateTime);
						HEADER_write64(fLast.dwHighDateTime);
						HEADER_write64(fCreate.dwLowDateTime);
						HEADER_write64(fCreate.dwHighDateTime);
						HEADER_write64(fMod.dwLowDateTime);
						HEADER_write64(fMod.dwHighDateTime);

						DWORD FileAttr = GetFileAttributes(_FileToAdd.c_str());	//if (attr != INVALID_FILE_ATTRIBUTES) {bool isReadOnly = attr & FILE_ATTRIBUTE_READONLY;}
						HEADER_write64(FileAttr);
						HEADER_write64(mStati64.st_size);

						if (mStati64.st_size == 0)
						{//EmptyFile
						}
						else
						{
							{
								XXH64_hash_t hashOut;
								size_t const buffInSize = 512 * 1024/*ZSTD_CStreamInSize()*/;
								void* const buffIn = malloc(buffInSize);
								size_t const buffOutSize = 16 * 1024 * 1024/*ZSTD_COMPRESSBOUND(buffInSize)*//*ZSTD_CStreamOutSize()*/;
								void* const buffOut = malloc(buffOutSize);
								ZSTD_CCtx* const cctx = ZSTD_createCCtx();
								ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, _CompressionLevel);
								ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 1);
								ZSTD_CCtx_setParameter(cctx, ZSTD_c_nbWorkers, mMaxThread);
								size_t const toRead = buffInSize;
								for (;;) {
									size_t read = fread(buffIn, sizeof(std::byte), toRead, fIN);
									MutexComp.lock();
									mProgressComp.Current += read;
									MutexComp.unlock();
									int const lastChunk = (read < toRead);
									ZSTD_EndDirective const mode = lastChunk ? ZSTD_e_end : ZSTD_e_continue;
									ZSTD_inBuffer input = { buffIn, read, 0 };
									int finished;
									do {
										ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
										size_t const remaining = ZSTD_compressStream2(cctx, &output, &input, mode);
										if (output.pos > 0)
										{
											std::vector<std::byte> vCrypted;
											//Out  buffer is full : Encrypting
											if (mEncryptMethod == EncryptionMethod::encChaCha20)
											{
												chacha::buffered_cipher cipher(chacha::key_bits_256, chacha::rounds20, &mVectorKEY[0], mNonce/*Nonce de l'archive*/);
												cipher.transform((std::byte*)output.dst, (std::byte*)output.dst, output.pos);
												vCrypted.resize(output.pos);
												memcpy_s(&vCrypted[0], vCrypted.size(), output.dst, output.pos);
											}
											else if (mEncryptMethod == EncryptionMethod::encAES256)
											{
												std::vector<std::byte> vIv;
												vIv.resize(16);
												MEM_write64(&vIv[0], mNonce);//uint64_t is 8byte long
												MEM_write64(&vIv[8], mNonce);
#ifdef USEWINDOWS_BCRYPT
												//Windows NewGen BCrypt engine
												BCryptEncryptVector((std::byte*)output.dst, output.pos, vCrypted, vIv, mVectorKEY);
#else
												//TinyAES engine
												struct AES_ctx ctx;
												AES_init_ctx_iv(&ctx, reinterpret_cast<uint8_t*>(&mVectorKEY[0]), reinterpret_cast<uint8_t*>(&vIv[0]));
												vCrypted.resize(output.pos);
												memcpy_s(&vCrypted[0], output.pos, output.dst, output.pos);
												AES256AddPadding(vCrypted);
												AES_CBC_encrypt_buffer(&ctx, reinterpret_cast<uint8_t*>(&vCrypted[0]), vCrypted.size());
#endif // USEWINDOWS_BCRYPT
												vIv.clear();

											}
											else if (mEncryptMethod == EncryptionMethod::encNone)
											{
												vCrypted.resize(output.pos);
												memcpy_s(&vCrypted[0], vCrypted.size(), output.dst, output.pos);
											}
											HEADER_write64(vCrypted.size());//Data Compressed and crypt size
											hashOut = XXH3_64bits(&vCrypted[0], vCrypted.size());
											HEADER_write64(hashOut);//Compressed XXHash64
											fwrite(&vCrypted[0], sizeof(std::byte), vCrypted.size(), _ArchiveFILE);
											vCrypted.clear();
										}
										finished = lastChunk ? (remaining == 0) : (input.pos == input.size);
									} while (!finished);
									if (!(input.pos == input.size))
										OutputDebugString(L"Impossible: zstd only returns 0 when the input is completely consumed!");
									if (lastChunk) {
										break;
									}
								}
								ZSTD_freeCCtx(cctx);
								free(buffIn);
								free(buffOut);
							}
							HEADER_write64(UINT64_MAX);//EndOfCompression MARK COMP
						}
						fclose(fIN);
						return ADDFILE_ERROR::ADDFILE_SUCCESS;
					}
					else
					{
						char _erMsg[4096];
						strerror_s(_erMsg, errno);
						OutputDebugStringA(std::string(std::string(_erMsg) + "\n").c_str());
						return ADDFILE_ERROR::ADDFILE_CANTOPENFILE;
					}
				}
				return ADDFILE_ERROR::ADDFILE_FATALERROR;
			}
			else
			{
				return ADDFILE_ERROR::ADDFILE_FILENOTEXIST;
			}
			return ADDFILE_ERROR::ADDFILE_FATALERROR;
		}


		/// @brief      Create file footer identified by the MagicByte sequence. Help to know if file is complete when opening it for decompression operation.
		/// @param		_ArchiveFILE		Pointer to a FILE* representing output archive
		/// @return     Bool : True if write success, False if failed
		bool archiveNZ_write_footer(FILE* _ArchiveFILE)
		{
			if (_ArchiveFILE == NULL)
				return false;
			uint64_t headerHashIn = 0;
			uint64_t headerHashOut = 0;
			std::vector<std::byte> compressedHeader;
			CompAndCrypt(mHeader.Header, compressedHeader, headerHashIn, headerHashOut, 8, EncryptionMethod::encChaCha20);
			fwrite(&compressedHeader[0], sizeof(std::byte), compressedHeader.size(), _ArchiveFILE);
			FILE_write64(_ArchiveFILE, mHeader.Header.size());
			FILE_write64(_ArchiveFILE, compressedHeader.size());
			FILE_write64(_ArchiveFILE, headerHashIn);
			FILE_write64(_ArchiveFILE, headerHashOut);

			FILE_write16(_ArchiveFILE, UINT16_MAX);//End archive
			fwrite(NZArchive::magicByte, sizeof(NZArchive::magicByte), 1, _ArchiveFILE);
			return true;
		}

		/// @brief  	Write a wstring with Unicode support to the FILE* archive.
		///				Writing is sequenced as follow :
		///				uint64_t Uncompressed wstring size
		///				uint64_t Compressed wstring size
		///				std::vector<std::byte> raw data
		/// @param  _ArchiveFILE		Pointer to a FILE* representing output archive
		/// @param  _wstring		wstring to be written
		/// @return     BOOL 			True if write success, False if failed
		bool archiveNZ_write_wstring(std::wstring _wstring)
		{
			std::vector<std::byte> _vecByte;
			_vecByte.resize(_wstring.size() * 2);
			memcpy_s(&_vecByte[0], _vecByte.size(), &_wstring[0], _vecByte.size());
			HEADER_write32((uint32_t)_vecByte.size());
			mHeader.Header.resize(mHeader.Header.size() + _vecByte.size());
			memcpy_s(&mHeader.Header[mHeader.Position], _vecByte.size(), &_vecByte[0], _vecByte.size());
			mHeader.Position += _vecByte.size();
			return true;
		}

		/// @brief      Set the max value to be used for progression reporting.
		///		        It's hard to get progression if I don't know the goal, right ?
		///				Setting the the total size of operation is mandatory.
		/// @param  _MaxValue	Total size of operation
		void archiveNZ_write_set_max_value_for_progress(uint64_t _MaxValue)
		{
			MutexComp.lock();
			mProgressComp.Total = _MaxValue;
			MutexComp.unlock();
		}

		/// @brief      Retrieve a _ProgressReporting struct filled with
		///             percentage of progression,
		///				current position of operation,
		///				Total position (Total size of operation)
		/// @return     _ProgressReporting struct with progress information.
		_ProgressReporting archiveNZ_write_get_progress_reporting()
		{
			MutexComp.lock();
			mProgressComp.Progress = (mProgressComp.Total == 0) ? 0 : (double)mProgressComp.Current * 100.0 / (double)mProgressComp.Total;
			auto cpProgressReporting = mProgressComp;
			MutexComp.unlock();
			return cpProgressReporting;
		}

		/// @brief      Convert 0-9 Level of compression to 0-22 Zstd standard (0 mean fastest compression).
		/// @param  _lvl0_9	0-9 level of compression
		/// @return     0-22 value (0 mean fastest compression).
		int archiveNZ_write_Convert_0_9_to_0_22(int _lvl0_9)
		{
			if (_lvl0_9 <= 0)return 1;
			if (_lvl0_9 == 1)return 1;
			if (_lvl0_9 == 2)return 3;
			if (_lvl0_9 == 3)return 6;
			if (_lvl0_9 == 4)return 9;
			if (_lvl0_9 == 5)return 12;
			if (_lvl0_9 == 6)return 15;
			if (_lvl0_9 == 7)return 17;
			if (_lvl0_9 == 8)return 19;
			if (_lvl0_9 >= 9)return 20;
			return 0;
		}

		/// @brief      Write a vector to given FILE* archive at the given filepath emplacement in the archive with a given compression level.
		///             Buffer can be adapted.
		///				You can give FILETIME for each state Created, Modified, Last Access
		///				If FILETIME value not set, using current date time
		/// @param  _ArchiveFILE		Pointer to a FILE* representing output archive
		/// @param  _VectorToCompress	vector of byte to be added to the file
		/// @param  _FilePathInArchive	Path as wstring to the file emplacement in the archive
		/// @param  _CompressionLevel	From 1 (FAST) to 22 (MAX)
		/// @param  _BUFFER		Defaut 8mb, can be modified
		/// @param  _FILETime_C	FILETIME date and time format for Creation of file
		/// @param  _FILETime_M	FILETIME date and time format for Modification of file
		/// @param  _FILETime_A	FILETIME date and time format for Last access of file
		/// @param  _FileAttrib	DWORD File attribute variable
		/// @param  _IsDirectory	bool Write as a directory or a file ?
		/// @return     Return ADDFILE_XXXX constant representing operation state:
		///
		/// 	ADDFILE_FILEPOINTER_NULL = (Invalide FILE*); 
		/// 	ADDFILE_FATALERROR = (Unknown error);
		/// 	ADDFILE_SUCCESS = (Operation success);
		ADDFILE_ERROR archiveNZ_write_add_memory_vector(FILE* _ArchiveFILE, std::vector<std::byte>& _VectorToCompress, std::wstring _FilePathInArchive, int _CompressionLevel = 3, EncryptionMethod mEncryptCHACHA = EncryptionMethod::encChaCha20, uint64_t _BUFFER = 32 * 1024 * 1024, FILETIME _FILETime_C = { 0,0 }, FILETIME _FILETime_A = { 0,0 }, FILETIME _FILETime_M = { 0,0 }, DWORD _FileAttrib = FILE_ATTRIBUTE_NORMAL, bool _IsDirectory = false)
		{
			FILETIME ft;
			SYSTEMTIME st;
			GetSystemTime(&st); // gets current time
			SystemTimeToFileTime(&st, &ft); // converts to file time format
			if (_FILETime_C.dwHighDateTime == 0 && _FILETime_C.dwLowDateTime == 0)
				_FILETime_C = ft;
			if (_FILETime_A.dwHighDateTime == 0 && _FILETime_A.dwLowDateTime == 0)
				_FILETime_A = ft;
			if (_FILETime_M.dwHighDateTime == 0 && _FILETime_M.dwLowDateTime == 0)
				_FILETime_M = ft;
			if (_ArchiveFILE == NULL)
				return ADDFILE_ERROR::ADDFILE_FILEPOINTER_NULL;
			{
				MutexComp.lock();
				mProgressComp.CurrentFile = _FilePathInArchive;
				MutexComp.unlock();
				if (_IsDirectory)
				{
					HEADER_write16(1);//1 Mean a directory
					HEADER_write16((uint16_t)mEncryptCHACHA);
					archiveNZ_write_wstring(_FilePathInArchive);
					HEADER_write64(_FILETime_A.dwLowDateTime);
					HEADER_write64(_FILETime_A.dwHighDateTime);
					HEADER_write64(_FILETime_C.dwLowDateTime);
					HEADER_write64(_FILETime_C.dwHighDateTime);
					HEADER_write64(_FILETime_M.dwLowDateTime);
					HEADER_write64(_FILETime_M.dwHighDateTime);
					HEADER_write64(_FileAttrib);
					return ADDFILE_ERROR::ADDFILE_SUCCESS;
				}
				else
				{
					{
						HEADER_write16(2);//2 Mean a file
						HEADER_write16((uint16_t)mEncryptCHACHA);
						archiveNZ_write_wstring(_FilePathInArchive);

						HEADER_write64(_FILETime_A.dwLowDateTime);
						HEADER_write64(_FILETime_A.dwHighDateTime);
						HEADER_write64(_FILETime_C.dwLowDateTime);
						HEADER_write64(_FILETime_C.dwHighDateTime);
						HEADER_write64(_FILETime_M.dwLowDateTime);
						HEADER_write64(_FILETime_M.dwHighDateTime);
						HEADER_write64(_FileAttrib);
						HEADER_write64(_VectorToCompress.size());
						{
							std::vector<std::byte> CompressedBuffer;
							XXH64_hash_t hashIn;
							XXH64_hash_t hashOut;
							{
								CompAndCrypt(_VectorToCompress, CompressedBuffer, hashIn, hashOut, _CompressionLevel, mEncryptCHACHA);
								HEADER_write64(CompressedBuffer.size());//Data Compressed size
								HEADER_write64(_VectorToCompress.size());//Data Original Size
								HEADER_write64(hashOut);//Original XXHash64
								HEADER_write64(hashIn);//Compressed XXHash64
								fwrite(&CompressedBuffer[0], sizeof(std::byte), CompressedBuffer.size(), _ArchiveFILE);
								MutexComp.lock();
								mProgressComp.Current += _VectorToCompress.size();
								MutexComp.unlock();
							}
							HEADER_write64(UINT64_MAX);//EndOfCompression MARK COMP
							HEADER_write64(UINT64_MAX);//EndOfCompression MARK UNCOMP
						}
						return ADDFILE_ERROR::ADDFILE_SUCCESS;
					}
				}
				return ADDFILE_ERROR::ADDFILE_FATALERROR;
			}
			return ADDFILE_ERROR::ADDFILE_FATALERROR;
		}

		/// @brief	Structure containing information of a chunk of compressed bytes.
		/// @param	hashCompressed	Hash of compressed chunk
		/// @param	ChunkCompressed	Compressed chunk size
		struct FileChunk
		{
			uint64_t hashCompressed = 0;
			uint64_t ChunkCompressed = 0;
		};

		/// @brief      Structure containing information of a file entry.
		/// @param	FullPath			Path of the file
		/// @param	isDirectory		Entry is a folder
		/// @param	FiletimeA		File last access time
		/// @param	FiletimeC		File creation time
		/// @param	FiletimeM		File last modified time
		/// @param	FileAttribute		File attribute
		/// @param	PositionInArchive		Position of the file in the archive
		/// @param	IndexInArchive		Index of the file in archive header
		/// @param	EncryptionMethod	Method of encryption
		/// @param	FileChunks		Vector of compressed chunks from the file
		struct FileEntryRead
		{
			std::wstring FullPath = L"";
			bool isDirectory = false;
			uint64_t FileSize = 0;
			FILETIME FiletimeA;
			FILETIME FiletimeC;
			FILETIME FiletimeM;
			DWORD FileAttribute = 0;
			uint64_t PositionInArchive = 0;
			uint64_t CopySize = 0;
			uint64_t IndexInArchive = 0;
			EncryptionMethod EncryptionMethod = EncryptionMethod::encNone;
			std::vector<FileChunk> FileChunks;
		};

		/// @brief      Structure containing archive information.
		/// @param	NZArchiveVersion			Retrieve the version in a wstring of NZArchive used for this archive
		/// @param	FullArchiveIncludingHeader	Full size of the archive (including the header)
		/// @param	ArchiveHeaderSize		Size of the header
		/// @param	ArchiveSizeCompressed		Size of the archive compressed
		/// @param	ArchiveSizeUncompressed		Size of the archive uncompressed
		/// @param	passwordError			Is password incorrect ?
		/// @param	FileList				Vector of FileEntry
		/// @param	isNZArchiveFile			Is the file an NZArchive file ?
		/// @param	EncryptionMethod		Method of encryption
		struct ArchiveEntry
		{
			std::wstring NZArchiveVersion = L"";
			uint64_t FullArchiveIncludingHeader = 0;
			uint64_t ArchiveHeaderSize = 0;
			uint64_t ArchiveSizeCompressed = 0;
			uint64_t ArchiveSizeUncompressed = 0;
			bool passwordError = false;
			std::vector<FileEntryRead> FileList;
			bool isNZArchiveFile = false;
			EncryptionMethod EncryptionMethod = EncryptionMethod::encNone;
		};

		/// @brief      Return ArchiveEntry structure containing archive information and file list inside.
		/// @param  _ArchiveFILE		Pointer to a FILE* representing input archive
		/// @return     ArchiveEntry structure
		const ArchiveEntry archiveNZ_read_extract_file_list(FILE* _ArchiveFILE)
		{
			ArchiveEntry _ArchiveEntry;
			_ArchiveEntry.isNZArchiveFile = false;
			if (_ArchiveFILE == NULL)
				return _ArchiveEntry;
			/*Reading header and validating MagicByte*/
			std::vector<uint8_t> GetMagicKeyStart; GetMagicKeyStart.resize(8);
			std::vector<uint8_t> GetMagicKeyEnd; GetMagicKeyEnd.resize(8);
			_fseeki64(_ArchiveFILE, 0, SEEK_END);
			_ArchiveEntry.FullArchiveIncludingHeader = _ftelli64(_ArchiveFILE);
			if (_ArchiveEntry.FullArchiveIncludingHeader < 16)
				return _ArchiveEntry;
			_fseeki64(_ArchiveFILE, _ArchiveEntry.FullArchiveIncludingHeader - 8, SEEK_SET);
			fread_s(&GetMagicKeyEnd[0], GetMagicKeyEnd.size(), sizeof(NZArchive::magicByte), 1, _ArchiveFILE);
			_fseeki64(_ArchiveFILE, 0, SEEK_SET);
			fread_s(&GetMagicKeyStart[0], GetMagicKeyStart.size(), sizeof(NZArchive::magicByte), 1, _ArchiveFILE);
			if (!std::equal(GetMagicKeyStart.begin(), GetMagicKeyStart.end(), mvMagicKey.begin()))
				return ArchiveEntry();
			if (!std::equal(GetMagicKeyStart.begin(), GetMagicKeyStart.end(), GetMagicKeyEnd.begin()))
				return ArchiveEntry();

			mHeader.Header.clear();
			mHeader.Position = 0;
			//Grab the size of the compressed header
			_fseeki64(_ArchiveFILE, _ArchiveEntry.FullArchiveIncludingHeader
				- (8/*magic byte*/ + sizeof(uint16_t)/*EOA*/ + (4 * sizeof(uint64_t))/*Header size uncomp / Comp + hashIn + hashOut*/), SEEK_SET);
			std::vector<std::byte> compHEADER;
			mHeader.Header.resize(FILE_read64(_ArchiveFILE));
			compHEADER.resize(FILE_read64(_ArchiveFILE));
			uint64_t expectedHashOut = FILE_read64(_ArchiveFILE);
			uint64_t expectedHashIn = FILE_read64(_ArchiveFILE);
			_fseeki64(_ArchiveFILE, _ArchiveEntry.FullArchiveIncludingHeader
				- (8/*magic byte*/ + sizeof(uint16_t)/*EOA*/ + (4 * sizeof(uint64_t))/*Header size uncomp / Comp + hashIn + hashOut*/ + compHEADER.size() /*Header size*/), SEEK_SET);
			uint64_t rHashIn = 0;
			uint64_t rHashOut = 0;
			fread_s(&compHEADER[0], compHEADER.size(), sizeof(std::byte), compHEADER.size(), _ArchiveFILE);
			if (!DecompAnDecrypt(compHEADER, mHeader.Header, rHashIn, rHashOut, EncryptionMethod::encChaCha20))
			{
				_ArchiveEntry.passwordError = true;
				return _ArchiveEntry;//Password is wrong
			}
			if ((expectedHashIn != rHashIn) || (expectedHashOut != rHashOut))
				return ArchiveEntry();//Data corruption

			auto _Major = HEADER_read16();
			auto _Minus = HEADER_read16();
			auto _Detail = HEADER_read16();
			uint64_t virtualFilePos = sizeof(NZArchive::magicByte);//Start of data in NZArchive
			_ArchiveEntry.isNZArchiveFile = true;
			_ArchiveEntry.NZArchiveVersion = std::to_wstring(_Major) + L"." + std::to_wstring(_Minus) + L"." + std::to_wstring(_Detail);
			/*Parsing the file*/
			while (true)
			{
				if (mHeader.Position == mHeader.Header.size())
					break;//End of header reached
				FileEntryRead _FileEntryRead;
				//_FileEntryRead.CopyPositionStart = virtualFilePos;
				uint16_t EntryType = HEADER_read16();
				if (EntryType == 1)//Directory
				{
					_FileEntryRead.PositionInArchive = virtualFilePos;
					_FileEntryRead.EncryptionMethod = (EncryptionMethod)HEADER_read16();
					_FileEntryRead.FullPath = archiveNZ_read_wstring();
					if (_FileEntryRead.FullPath == L"")
						_ArchiveEntry.passwordError = true;
					_FileEntryRead.FiletimeA.dwLowDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FiletimeA.dwHighDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FiletimeC.dwLowDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FiletimeC.dwHighDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FiletimeM.dwLowDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FiletimeM.dwHighDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FileAttribute = (DWORD)HEADER_read64();
					_FileEntryRead.isDirectory = true;
					_FileEntryRead.CopySize = 0;
					_FileEntryRead.IndexInArchive = _ArchiveEntry.FileList.size();
					_ArchiveEntry.FileList.push_back(_FileEntryRead);
				}
				else if (EntryType == 2)//File
				{
					_FileEntryRead.PositionInArchive = virtualFilePos;
					_FileEntryRead.EncryptionMethod = (EncryptionMethod)HEADER_read16();
					_FileEntryRead.FullPath = archiveNZ_read_wstring();
					if (_FileEntryRead.FullPath == L"")
						_ArchiveEntry.passwordError = true;
					_FileEntryRead.FiletimeA.dwLowDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FiletimeA.dwHighDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FiletimeC.dwLowDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FiletimeC.dwHighDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FiletimeM.dwLowDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FiletimeM.dwHighDateTime = (DWORD)HEADER_read64();
					_FileEntryRead.FileAttribute = (DWORD)HEADER_read64();
					_FileEntryRead.FileSize = HEADER_read64();
					_FileEntryRead.isDirectory = false;
					_ArchiveEntry.ArchiveSizeUncompressed += _FileEntryRead.FileSize;
					if (_FileEntryRead.FileSize > 0)
						while (true)
						{
							FileChunk _FileChunk;
							_FileChunk.ChunkCompressed = HEADER_read64();
							//_FileChunk.ChunkUncompressed = HEADER_read64();
							if (_FileChunk.ChunkCompressed == UINT64_MAX/* && _FileChunk.ChunkUncompressed == UINT64_MAX*/)
								break;//END OF FILE >> EXIT WHILE
							_FileChunk.hashCompressed = HEADER_read64();
							//_FileChunk.hashUncompressed = HEADER_read64();
							_ArchiveEntry.ArchiveSizeCompressed += _FileChunk.ChunkCompressed;
							_FileEntryRead.FileChunks.push_back(_FileChunk);
							virtualFilePos += _FileChunk.ChunkCompressed;
						}
					else
					{
						//	Empty file
					}
					_FileEntryRead.CopySize = virtualFilePos - _FileEntryRead.PositionInArchive;
					_FileEntryRead.IndexInArchive = _ArchiveEntry.FileList.size();
					_ArchiveEntry.FileList.push_back(_FileEntryRead);
				}
				else if (EntryType == UINT16_MAX)//Footer reached
				{
					break;
				}
			}
#pragma endregion
			_ArchiveEntry.ArchiveHeaderSize = compHEADER.size();
			for (size_t i = 0; i < _ArchiveEntry.FileList.size(); i++)
			{
				{//Settings flags
					_ArchiveEntry.EncryptionMethod = _ArchiveEntry.FileList[i].EncryptionMethod;
					break;//Do not conusme more time for this
				}
			}
			return _ArchiveEntry;
		}

		/// @brief      Extract all file from FILE* archive to a specified path,
		///             if path don't exist, it will be created.
		/// @param  _ArchiveFILE		Pointer to a FILE* representing input archive
		/// @param  _PathToExtract		Path destination
		/// @return     OP_EXTRACT with given status
		const OP_EXTRACT archiveNZ_read_extract_all_files_to(FILE* _ArchiveFILE, std::wstring _PathToExtract)
		{
			if (_ArchiveFILE == NULL)
				return OP_EXTRACT::OP_EXTRACT_CANTOPENARCHIVE;
			auto _FileList = archiveNZ_read_extract_file_list(_ArchiveFILE);
			if (_FileList.passwordError)
				return OP_EXTRACT::OP_EXTRACT_CANTOPENPASSWORDERROR;
			MutexUncomp.lock();
			mProgressUncomp.Current = 0;
			mProgressUncomp.Total = _FileList.ArchiveSizeCompressed;
			MutexUncomp.unlock();
			uint64_t NbFileErrorCANTCREATEOUTPUT = 0;
			uint64_t NbFileErrorHASHERROR = 0;
			for (size_t i = 0; i < _FileList.FileList.size(); i++)
			{
				std::wstring FileToWrite = _PathToExtract + _FileList.FileList[i].FullPath;
				MutexUncomp.lock();
				mProgressUncomp.CurrentFile = FileToWrite;
				MutexUncomp.unlock();
				if (_FileList.FileList[i].isDirectory)
				{
					//CreateDirectory
					try
					{
						std::filesystem::create_directories(FileToWrite);
						SetFileAttributesW(FileToWrite.c_str(), _FileList.FileList[i].FileAttribute);
						HANDLE hHandle = CreateFile(FileToWrite.c_str()
							, FILE_WRITE_ATTRIBUTES
							, FILE_SHARE_WRITE
							, NULL
							, OPEN_EXISTING
							, FILE_ATTRIBUTE_NORMAL
							, NULL);
						SetFileTime(hHandle, &_FileList.FileList[i].FiletimeC, &_FileList.FileList[i].FiletimeA, &_FileList.FileList[i].FiletimeM);
						CloseHandle(hHandle);
					}
					catch (const std::exception&)
					{

					}
				}
				else
				{
					//CreateFile
					try
					{
						//CreateOutput Directories
						std::wstring FolderPath = FileToWrite;
						FolderPath = FolderPath.substr(0, FolderPath.find_last_of(L"\\"));
						std::filesystem::create_directories(FolderPath);
					}
					catch (const std::exception&)
					{

					}
					//Setings permissions free
					if (std::filesystem::exists(FileToWrite))
						if (!std::filesystem::is_directory(FileToWrite))
						{//File exist and is not a directory
							//Change attribute so it can be easily overwritten
							SetFileAttributes(FileToWrite.c_str(), FILE_ATTRIBUTE_NORMAL);
						}
					//Opening file for writing
					FILE* fOUT = NULL;
					_wfopen_s(&fOUT, FileToWrite.c_str(), L"wb");
					if (fOUT != NULL)
					{
						//Start Offset
						_fseeki64(_ArchiveFILE, _FileList.FileList[i].PositionInArchive, SEEK_SET);
						const size_t BuffSize = ZSTD_DStreamInSize();
						size_t const buffInSize = ZSTD_DStreamInSize();
						void* const buffIn = malloc(buffInSize);
						size_t const buffOutSize = ZSTD_DStreamOutSize();  /* Guarantee to successfully flush at least one complete compressed block in all circumstances. */
						void* const buffOut = malloc(buffOutSize);
						ZSTD_DCtx* const dctx = ZSTD_createDCtx();
						if (dctx == NULL)
							return OP_EXTRACT::OP_EXTRACT_HASHERRORS;
						size_t const toRead = buffInSize;
						size_t readPos = 0;

						for (size_t j = 0; j < _FileList.FileList[i].FileChunks.size(); j++)//Filelist is empty :: 0 Bytes size file
						{
							std::vector<std::byte> vecRead;
							vecRead.resize(_FileList.FileList[i].FileChunks[j].ChunkCompressed);
							fread_s(&vecRead[0], vecRead.size(), sizeof(std::byte), vecRead.size(), _ArchiveFILE);
							MutexUncomp.lock();
							mProgressUncomp.Current += vecRead.size();
							MutexUncomp.unlock();
							if (j > 0)
								printf("");

							XXH64_hash_t hashIn;
							hashIn = XXH3_64bits(&vecRead[0], vecRead.size());
							if ((hashIn != _FileList.FileList[i].FileChunks[j].hashCompressed))
							{
								NbFileErrorHASHERROR++;
								break;//ArchiveCorrupted, Not writing this one
							}
							if (_FileList.FileList[i].EncryptionMethod == EncryptionMethod::encChaCha20)
							{
								chacha::buffered_cipher cipher(chacha::key_bits_256, chacha::rounds20, &mVectorKEY[0], mNonce/*Nonce de l'archive*/);
								cipher.transform(&vecRead[0], &vecRead[0], vecRead.size());
							}
							else if (_FileList.FileList[i].EncryptionMethod == EncryptionMethod::encAES256)
							{
								std::vector<std::byte> vIv;
								vIv.resize(16);
								MEM_write64(&vIv[0], mNonce);
								MEM_write64(&vIv[8], mNonce);
#ifdef USEWINDOWS_BCRYPT
								//Windows NewGen BCrypt engine
								std::vector<std::byte> vDecrypted;
								BCryptDecryptVector(vecRead, vDecrypted, vIv, mVectorKEY);
								vecRead.clear();
								vecRead.resize(vDecrypted.size());
								memcpy_s(&vecRead[0], vDecrypted.size(), &vDecrypted[0], vDecrypted.size());
								vDecrypted.clear();
#else
								//TinyAES Engine 
								struct AES_ctx ctx;
								AES_init_ctx_iv(&ctx, reinterpret_cast<uint8_t*>(&mVectorKEY[0]), reinterpret_cast<uint8_t*>(&vIv[0]));
								AES_CBC_decrypt_buffer(&ctx, reinterpret_cast<uint8_t*>(&vecRead[0]), vecRead.size());
								AddAES256RemovePadding(vecRead);
#endif // USEWINDOWS_BCRYPT
								vIv.clear();
							}
							else if (_FileList.FileList[i].EncryptionMethod == EncryptionMethod::encNone)
							{
							}
							readPos = 0;
							while (true) {
								size_t read = (readPos + BuffSize >= vecRead.size()) ? vecRead.size() - readPos : BuffSize;
								memcpy_s(buffIn, read, &vecRead[readPos], read);
								readPos += read;
								ZSTD_inBuffer input = { buffIn, read, 0 };
								while (input.pos < input.size) {
									ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
									size_t const ret = ZSTD_decompressStream(dctx, &output, &input);
									if (output.pos > 0)
									{
										fwrite(buffOut, sizeof(std::byte), output.pos, fOUT);
									}
								}
								if (readPos == vecRead.size())
									break;//Exit
							}
						}
						fclose(fOUT);
						SetFileAttributesW(FileToWrite.c_str(), _FileList.FileList[i].FileAttribute);
						HANDLE hHandle = CreateFile(FileToWrite.c_str()
							, FILE_WRITE_ATTRIBUTES
							, FILE_SHARE_WRITE
							, NULL
							, OPEN_EXISTING
							, FILE_ATTRIBUTE_NORMAL
							, NULL);
						SetFileTime(hHandle, &_FileList.FileList[i].FiletimeC, &_FileList.FileList[i].FiletimeA, &_FileList.FileList[i].FiletimeM);
						CloseHandle(hHandle);
						SetFileAttributesW(FileToWrite.c_str(), _FileList.FileList[i].FileAttribute);
					}
					else
					{
						NbFileErrorCANTCREATEOUTPUT++;
					}
				}
			}
			if (NbFileErrorHASHERROR > 0)
				return OP_EXTRACT::OP_EXTRACT_HASHERRORS;
			if (NbFileErrorCANTCREATEOUTPUT > 0)
				return OP_EXTRACT::OP_EXTRACT_CANTCREATEOUTFILE;
			return OP_EXTRACT::OP_EXTRACT_SUCCESS;
		}

		/// @brief      Extract a single file from FILE* archive with a given index to a specified file path,
		///             FileIndex can be retrived from filelist.
		/// @param  _ArchiveFILE		Pointer to a FILE* representing input archive
		/// @param  _FileIndex		index of the file to be extracted
		/// @param  _FullPathToExtract	File path destination
		/// @param  UseCachedList	false: Open archive and get file list / true: Use filelist from previous call of the function
		/// @return     OP_EXTRACT with detailed information
		const OP_EXTRACT archiveNZ_read_extract_file_to_this_full_path(FILE* _ArchiveFILE, size_t _FileIndex, std::wstring _FullPathToExtract, bool UseCachedList = false)
		{
			if (_ArchiveFILE == NULL)
				return OP_EXTRACT::OP_EXTRACT_CANTOPENARCHIVE;
			auto _FileList = UseCachedList ? mCachedArchive : archiveNZ_read_extract_file_list(_ArchiveFILE);//TODO OPTIMIZE THE REOPENING OF ARCHIVE EACH TIME
			if (_FileList.passwordError)
				return OP_EXTRACT::OP_EXTRACT_CANTOPENPASSWORDERROR;
			mCachedArchive = _FileList;
			MutexUncomp.lock();
			mProgressUncomp.Current = 0;
			mProgressUncomp.Total = _FileList.FileList[_FileIndex].FileSize;
			MutexUncomp.unlock();
			std::wstring FileToWrite = _FullPathToExtract /*+ _FileList.FileList[_FileIndex].FullPath*/;
			MutexUncomp.lock();
			mProgressUncomp.CurrentFile = FileToWrite;
			MutexUncomp.unlock();
			if (_FileList.FileList[_FileIndex].isDirectory)
			{
				//CreateDirectory
				try
				{
					std::filesystem::create_directories(FileToWrite);
					SetFileAttributesW(FileToWrite.c_str(), _FileList.FileList[_FileIndex].FileAttribute);
					HANDLE hHandle = CreateFile(FileToWrite.c_str()
						, FILE_WRITE_ATTRIBUTES
						, FILE_SHARE_WRITE
						, NULL
						, OPEN_EXISTING
						, FILE_ATTRIBUTE_NORMAL
						, NULL);
					SetFileTime(hHandle, &_FileList.FileList[_FileIndex].FiletimeC, &_FileList.FileList[_FileIndex].FiletimeA, &_FileList.FileList[_FileIndex].FiletimeM);
					CloseHandle(hHandle);
				}
				catch (const std::exception&)
				{

				}
			}
			else
			{
				//CreateFile
				try
				{
					//CreateOutput Directories
					std::wstring FolderPath = FileToWrite;
					FolderPath = FolderPath.substr(0, FolderPath.find_last_of(L"\\"));
					std::filesystem::create_directories(FolderPath);
				}
				catch (const std::exception&)
				{

				}
				//Opening file for writing
				FILE* fOUT = NULL;
				SetFileAttributes(FileToWrite.c_str(), FILE_ATTRIBUTE_NORMAL);
				_wfopen_s(&fOUT, FileToWrite.c_str(), L"wb");
				if (fOUT != NULL)
				{
					bool ErrorDATACorrupt = false;
					//Start Offset
					_fseeki64(_ArchiveFILE, _FileList.FileList[_FileIndex].PositionInArchive, SEEK_SET);

					const size_t BuffSize = ZSTD_DStreamInSize() * 16;
					size_t const buffInSize = ZSTD_DStreamInSize() * 16;
					void* const buffIn = malloc(buffInSize);
					size_t const buffOutSize = ZSTD_DStreamOutSize() * 16;  /* Guarantee to successfully flush at least one complete compressed block in all circumstances. */
					void* const buffOut = malloc(buffOutSize);
					ZSTD_DCtx* const dctx = ZSTD_createDCtx();
					if (dctx == NULL)
						return OP_EXTRACT::OP_EXTRACT_HASHERRORS;
					size_t const toRead = buffInSize;
					size_t readPos = 0;

					for (size_t j = 0; j < _FileList.FileList[_FileIndex].FileChunks.size(); j++)//Filelist is empty :: 0 Bytes size file
					{
						std::vector<std::byte> vecRead;
						vecRead.resize(_FileList.FileList[_FileIndex].FileChunks[j].ChunkCompressed);
						fread_s(&vecRead[0], vecRead.size(), sizeof(std::byte), vecRead.size(), _ArchiveFILE);
						MutexUncomp.lock();
						mProgressUncomp.Current += vecRead.size();
						MutexUncomp.unlock();
						if (j > 0)
							printf("");

						XXH64_hash_t hashIn;
						hashIn = XXH3_64bits(&vecRead[0], vecRead.size());
						if ((hashIn != _FileList.FileList[_FileIndex].FileChunks[j].hashCompressed))
						{
							ErrorDATACorrupt = true;
							break;//ArchiveCorrupted, Not writing this one
						}
						if (_FileList.FileList[_FileIndex].EncryptionMethod == EncryptionMethod::encChaCha20)
						{
							chacha::buffered_cipher cipher(chacha::key_bits_256, chacha::rounds20, &mVectorKEY[0], mNonce/*Nonce de l'archive*/);
							cipher.transform(&vecRead[0], &vecRead[0], vecRead.size());
						}
						else if (_FileList.FileList[_FileIndex].EncryptionMethod == EncryptionMethod::encAES256)
						{
							std::vector<std::byte> vIv;
							vIv.resize(16);
							MEM_write64(&vIv[0], mNonce);
							MEM_write64(&vIv[8], mNonce);
#ifdef USEWINDOWS_BCRYPT
							//Windows NewGen BCrypt engine
							std::vector<std::byte> vDecrypted;
							BCryptDecryptVector(vecRead, vDecrypted, vIv, mVectorKEY);
							vecRead.clear();
							vecRead.resize(vDecrypted.size());
							memcpy_s(&vecRead[0], vDecrypted.size(), &vDecrypted[0], vDecrypted.size());
							vDecrypted.clear();
#else
							//TinyAES Engine
							struct AES_ctx ctx;
							AES_init_ctx_iv(&ctx, reinterpret_cast<uint8_t*>(&mVectorKEY[0]), reinterpret_cast<uint8_t*>(&vIv[0]));
							AES_CBC_decrypt_buffer(&ctx, reinterpret_cast<uint8_t*>(&vecRead[0]), vecRead.size());
							AddAES256RemovePadding(vecRead);
#endif // USEWINDOWS_BCRYPT
							vIv.clear();
						}
						else if (_FileList.FileList[_FileIndex].EncryptionMethod == EncryptionMethod::encNone)
						{
						}
						readPos = 0;
						while (true) {
							size_t read = (readPos + BuffSize >= vecRead.size()) ? vecRead.size() - readPos : BuffSize;
							memcpy_s(buffIn, read, &vecRead[readPos], read);
							readPos += read;
							ZSTD_inBuffer input = { buffIn, read, 0 };
							while (input.pos < input.size) {
								ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
								size_t const ret = ZSTD_decompressStream(dctx, &output, &input);
								if (output.pos > 0)
									fwrite(buffOut, sizeof(std::byte), output.pos, fOUT);
							}
							if (readPos == vecRead.size())
								break;//Exit
						}
					}
					ZSTD_freeDCtx(dctx);
					free(buffIn);
					free(buffOut);

					fclose(fOUT);
					if (ErrorDATACorrupt)
					{
						try
						{
							std::filesystem::remove(FileToWrite);
						}
						catch (const std::exception&)
						{

						}
						return OP_EXTRACT::OP_EXTRACT_HASHERRORS;
					}
					SetFileAttributesW(FileToWrite.c_str(), _FileList.FileList[_FileIndex].FileAttribute);
					HANDLE hHandle = CreateFile(FileToWrite.c_str()
						, FILE_WRITE_ATTRIBUTES
						, FILE_SHARE_WRITE
						, NULL
						, OPEN_EXISTING
						, FILE_ATTRIBUTE_NORMAL
						, NULL);
					SetFileTime(hHandle, &_FileList.FileList[_FileIndex].FiletimeC, &_FileList.FileList[_FileIndex].FiletimeA, &_FileList.FileList[_FileIndex].FiletimeM);
					CloseHandle(hHandle);
					SetFileAttributesW(FileToWrite.c_str(), _FileList.FileList[_FileIndex].FileAttribute);
				}
				else
				{//Can't create out file
					return OP_EXTRACT::OP_EXTRACT_CANTCREATEOUTFILE;
				}
			}
			return OP_EXTRACT::OP_EXTRACT_SUCCESS;
		}

		/// @brief	Extract a single file from FILE* archive with a given index to a given vector,
		///			FileIndex can be retrived from filelist.
		/// @param  _ArchiveFILE		Pointer to a FILE* representing input archive
		/// @param  _FileIndex		index of the file to be extracted
		/// @param  _OutVectorByte	Byte Vector with uncompressed file
		/// @return     OP_EXTRACT with detailed information
		const OP_EXTRACT archiveNZ_read_extract_file_to_byte_vector(FILE* _ArchiveFILE, size_t _FileIndex, std::vector<std::byte>& _OutVectorByte)
		{
			if (_ArchiveFILE == NULL)
				return OP_EXTRACT::OP_EXTRACT_CANTOPENARCHIVE;
			auto _FileList = archiveNZ_read_extract_file_list(_ArchiveFILE);
			if (_FileList.passwordError)
				return OP_EXTRACT::OP_EXTRACT_CANTOPENPASSWORDERROR;
			_OutVectorByte.clear();
			MutexUncomp.lock();
			mProgressUncomp.Current = 0;
			mProgressUncomp.Total = _FileList.FileList[_FileIndex].FileSize;
			MutexUncomp.unlock();
			std::wstring FileToWrite = _FileList.FileList[_FileIndex].FullPath;
			MutexUncomp.lock();
			mProgressUncomp.CurrentFile = FileToWrite;
			MutexUncomp.unlock();
			if (_FileList.FileList[_FileIndex].isDirectory)
			{
				//CreateDirectory
				//Useless for the moment
			}
			else
			{
				//CreateFile --- Vector
				_OutVectorByte.clear();
				_fseeki64(_ArchiveFILE, _FileList.FileList[_FileIndex].PositionInArchive, SEEK_SET);
				uint64_t vecPosition = 0;
				bool HashErrors = false;

				const size_t BuffSize = ZSTD_DStreamInSize();
				size_t const buffInSize = ZSTD_DStreamInSize();
				void* const buffIn = malloc(buffInSize);
				size_t const buffOutSize = ZSTD_DStreamOutSize();  /* Guarantee to successfully flush at least one complete compressed block in all circumstances. */
				void* const buffOut = malloc(buffOutSize);
				ZSTD_DCtx* const dctx = ZSTD_createDCtx();
				if (dctx == NULL)
					return OP_EXTRACT::OP_EXTRACT_HASHERRORS;
				size_t const toRead = buffInSize;
				size_t readPos = 0;

				for (size_t j = 0; j < _FileList.FileList[_FileIndex].FileChunks.size(); j++)//Filelist is empty :: 0 Bytes size file
				{
					std::vector<std::byte> vecRead;
					vecRead.resize(_FileList.FileList[_FileIndex].FileChunks[j].ChunkCompressed);
					fread_s(&vecRead[0], vecRead.size(), sizeof(std::byte), vecRead.size(), _ArchiveFILE);
					MutexUncomp.lock();
					mProgressUncomp.Current += vecRead.size();
					MutexUncomp.unlock();
					if (j > 0)
						printf("");

					XXH64_hash_t hashIn;
					hashIn = XXH3_64bits(&vecRead[0], vecRead.size());
					if ((hashIn != _FileList.FileList[_FileIndex].FileChunks[j].hashCompressed))
					{
						HashErrors = true;
						break;//ArchiveCorrupted, Not writing this one
					}
					if (_FileList.FileList[_FileIndex].EncryptionMethod == EncryptionMethod::encChaCha20)
					{
						chacha::buffered_cipher cipher(chacha::key_bits_256, chacha::rounds20, &mVectorKEY[0], mNonce/*Nonce de l'archive*/);
						cipher.transform(&vecRead[0], &vecRead[0], vecRead.size());
					}
					else if (_FileList.FileList[_FileIndex].EncryptionMethod == EncryptionMethod::encAES256)
					{
						std::vector<std::byte> vIv;
						vIv.resize(16);
						MEM_write64(&vIv[0], mNonce);
						MEM_write64(&vIv[8], mNonce);
#ifdef USEWINDOWS_BCRYPT
						//Windows NewGen BCrypt engine
						std::vector<std::byte> vDecrypted;
						BCryptDecryptVector(vecRead, vDecrypted, vIv, mVectorKEY);
						vecRead.clear();
						vecRead.resize(vDecrypted.size());
						memcpy_s(&vecRead[0], vDecrypted.size(), &vDecrypted[0], vDecrypted.size());
						vDecrypted.clear();
#else
						//TinyAES Engine
						struct AES_ctx ctx;
						AES_init_ctx_iv(&ctx, reinterpret_cast<uint8_t*>(&mVectorKEY[0]), reinterpret_cast<uint8_t*>(&vIv[0]));
						AES_CBC_decrypt_buffer(&ctx, reinterpret_cast<uint8_t*>(&vecRead[0]), vecRead.size());
						AddAES256RemovePadding(vecRead);
#endif // USEWINDOWS_BCRYPT
						vIv.clear();
					}
					else if (_FileList.FileList[_FileIndex].EncryptionMethod == EncryptionMethod::encNone)
					{
					}
					readPos = 0;
					while (true) {
						size_t read = (readPos + BuffSize >= vecRead.size()) ? vecRead.size() - readPos : BuffSize;
						memcpy_s(buffIn, read, &vecRead[readPos], read);
						readPos += read;
						ZSTD_inBuffer input = { buffIn, read, 0 };
						while (input.pos < input.size) {
							ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
							size_t const ret = ZSTD_decompressStream(dctx, &output, &input);
							if (output.pos > 0)
							{
								_OutVectorByte.resize(_OutVectorByte.size() + output.pos);
								memcpy_s(&_OutVectorByte[_OutVectorByte.size() - output.pos], output.pos, buffOut, output.pos);
							}
						}
						if (readPos == vecRead.size())
							break;//Exit
					}
				}
			}
			return OP_EXTRACT::OP_EXTRACT_SUCCESS;
		}

		/// @brief		Uncompress and decrypt the wstring in the FILE* archive to a wstring.
		///				Reading is sequenced as follow :
		///				Get uint64_t Uncompressed wstring size
		///				Get uint64_t Compressed wstring size
		///				Fill std::vector<std::byte> with Compressed data
		///				Uncompress and decrypt data then fill a wstring
		/// @param  _ArchiveFILE		Pointer to a FILE* representing output archive
		/// @return     wstring			Full wstring at given archive position
		std::wstring archiveNZ_read_wstring()
		{
			auto szWstring = HEADER_read32();
			std::vector<std::byte> _vecBUF;
			_vecBUF.resize(szWstring);
			memcpy_s(&_vecBUF[0], szWstring, &mHeader.Header[mHeader.Position], szWstring);
			mHeader.Position += szWstring;
			std::wstring Result = L"";
			Result.resize(szWstring / 2);
			memcpy_s(&Result[0], _vecBUF.size(), &_vecBUF[0], _vecBUF.size());
			return Result;
		}

		/// @brief	Retrieve a _ProgressReporting struct filled with percentage of progression, 
		///			current position of operation, Total position (Total size of operation)
		/// @return     _ProgressReporting struct with progress information.
		_ProgressReporting archiveNZ_read_get_progress_reporting()
		{
			MutexUncomp.lock();
			mProgressUncomp.Progress = (mProgressUncomp.Total == 0) ? 0 : (double)mProgressUncomp.Current * 100.0 / (double)mProgressUncomp.Total;
			auto cpProgressReporting = mProgressUncomp;
			MutexUncomp.unlock();
			return cpProgressReporting;
		}

		/// @brief      Reset reading progression (ideal for static call to NZArchive)
		void archiveNZ_read_reset_progress()
		{
			MutexUncomp.lock();
			mProgressUncomp.Current = 0;
			mProgressUncomp.CurrentFile = L"";
			mProgressUncomp.Progress = 0.0;
			mProgressUncomp.Total = 0;
			MutexUncomp.unlock();
		}
		/// @brief      Reset writing progression (ideal for static call to NZArchive)
		void archiveNZ_write_reset_progress()
		{
			MutexComp.lock();
			mProgressComp.Current = 0;
			mProgressComp.CurrentFile = L"";
			mProgressComp.Progress = 0.0;
			mProgressComp.Total = 0;
			MutexComp.unlock();
		}

		/// @brief      Set Archive encryption with given Key and Nonce,
		///             If both are empty, defaut one will be generated.
		/// @param  _vectorKEY32  vector of 32 byte containing raw key
		/// @param  _Nonce  arbitrary number value used as initialization encryption vector
		void archiveNZ_set_key(std::string _KEY32)
		{
			mVectorKEY.clear();
			mVectorKEY.resize(32);
			if (_KEY32.size() == 0)
			{//No Key provided, using blank vector
			}
			else if (_KEY32.size() >= 33)
			{//Superior to 32, Truncating vector
				memcpy_s(&mVectorKEY[0], mVectorKEY.size(), &_KEY32[0], mVectorKEY.size());
			}
			else//Standard Key Code
				memcpy_s(&mVectorKEY[0], _KEY32.size(), &_KEY32[0], _KEY32.size());
			mNonce = XXH3_64bits(&mVectorKEY[0], mVectorKEY.size());
			mvMagicKey.resize(8);
			memcpy_s(&mvMagicKey[0], mvMagicKey.size(), magicByte, mvMagicKey.size());
		}

		/// @brief	Set the number of thread used by ZSTD in multithread compression
		/// @param	MaxThread	Number of thread to be used
		void archiveNZ_set_number_of_thread(size_t MaxThread)
		{
			//Seems that setting param to 1 make all thread workers spawned
			if (MaxThread <= 1)//If less or equal to 1 thread, set to 0 meaning no added thread workers
				mMaxThread = 0;
			else
				mMaxThread = MaxThread;//Starting 2, it's really meaning what it is supposed to means ... 
		}

		/// @brief	Retrieve the number of thread used by ZSTD in multithread compression
		/// @return	SIZE_T	Number of thread
		size_t archiveNZ_get_number_of_thread()
		{
			return mMaxThread;
		}

		/// @brief  Compress a vector with ZSTD in stream mode (Meaning multithreading mode enabled).
		/// @param  _VecIn			Vector of std::byte with original data
		/// @param  _VecOut			Vector of std::byte with compressed data (vector will be cleared at the beginning of the process)
		/// @param  CompressionLevel	Compression level (From 1 to 22)
		/// @param  NumThread		Number of thread to be used for compression
		/// @return     ZSTD_stream_error, with detailed operation status
		ZSTD_stream_error ZSTD_stream_compress(std::vector<std::byte>& _VecIN, std::vector<std::byte>& _VecOUT, size_t CompressionLevel = 5, size_t NumThread = 0)
		{
			if (_VecIN.size() == 0)
				return ZSTD_stream_error::ZS_SourceEmpty;

			ZSTD_CCtx_params* const cctxParams = ZSTD_createCCtxParams();
			if (cctxParams == NULL)
				return ZSTD_stream_error::ZS_CCTXError;
			ZSTD_CCtxParams_setParameter(cctxParams, ZSTD_c_compressionLevel, CompressionLevel);
			ZSTD_CCtxParams_setParameter(cctxParams, ZSTD_c_checksumFlag, 1);
			ZSTD_CCtxParams_setParameter(cctxParams, ZSTD_c_nbWorkers, NumThread);
			ZSTD_CCtx* const cctx = ZSTD_createCCtx();/* Create the context. */
			if (cctx == NULL)
				return ZSTD_stream_error::ZS_CCTXError;
			ZSTD_CCtx_setParametersUsingCCtxParams(cctx, cctxParams);
			_VecOUT.clear();
			const size_t BuffSize = /*ZSTD_CStreamInSize()*/8 * 1024 * 1024;
			size_t const buffInSize = BuffSize;
			void* const buffIn = malloc(buffInSize);
			size_t const buffOutSize = /*ZSTD_CStreamOutSize()*/1 * 1024 * 1024;
			void* const buffOut = malloc(buffOutSize);
			size_t srcPos = 0;
			while (true)
			{
				size_t read = (srcPos + BuffSize >= _VecIN.size()) ? _VecIN.size() - srcPos : BuffSize;
				memcpy_s(buffIn, read, &_VecIN[srcPos], read);
				srcPos += read;
				int const lastChunk = (read < BuffSize);
				ZSTD_EndDirective const mode = lastChunk ? ZSTD_e_end : ZSTD_e_continue;
				ZSTD_inBuffer input = { buffIn, read, 0 };
				while (true)
				{
					ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
					size_t const remaining = ZSTD_compressStream2(cctx, &output, &input, mode);
					if (output.pos > 0)
					{
						_VecOUT.resize(_VecOUT.size() + output.pos);
						memcpy_s(&_VecOUT[_VecOUT.size() - output.pos], output.pos, buffOut, output.pos);
					}
					if (lastChunk ? (remaining == 0) : (input.pos == input.size))
						break;
				}
				if (!(input.pos == input.size))
					return ZSTD_stream_error::ZS_ErrorConsumingInput;// Impossible: zstd only returns 0 when the input is completely consumed!
				if (lastChunk)
					break;
			}
			ZSTD_freeCCtx(cctx);
			free(buffIn);
			free(buffOut);
			return ZSTD_stream_error::ZS_NoError;
		}

		/// @brief      Uncompress a vector with ZSTD in stream mode.
		/// @param  _VecIn		Vector of std::byte with compressed data
		/// @param  _VecOut		Vector of std::byte with uncompressed data (vector will be cleared at the beginning of the process)
		/// @return     ZSTD_stream_error, with detailed operation status
		ZSTD_stream_error ZSTD_stream_uncompress(std::vector<std::byte>& _VecIN, std::vector<std::byte>& _VecOUT)
		{
			if (_VecIN.size() == 0)
				return ZSTD_stream_error::ZS_SourceEmpty;
			const size_t BuffSize = ZSTD_DStreamInSize();
			size_t const buffInSize = ZSTD_DStreamInSize();
			void* const buffIn = malloc(buffInSize);
			size_t const buffOutSize = ZSTD_DStreamOutSize();  /* Guarantee to successfully flush at least one complete compressed block in all circumstances. */
			void* const buffOut = malloc(buffOutSize);
			ZSTD_DCtx* const dctx = ZSTD_createDCtx();
			if (dctx == NULL)
				return ZSTD_stream_error::ZS_DCTXError;
			size_t const toRead = buffInSize;
			size_t readPos = 0;
			_VecOUT.clear();
			while (true) {
				size_t read = (readPos + BuffSize >= _VecIN.size()) ? _VecIN.size() - readPos : BuffSize;
				memcpy_s(buffIn, read, &_VecIN[readPos], read);
				readPos += read;
				ZSTD_inBuffer input = { buffIn, read, 0 };
				while (input.pos < input.size) {
					ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
					size_t const ret = ZSTD_decompressStream(dctx, &output, &input);
					if (output.pos > 0)
					{
						_VecOUT.resize(_VecOUT.size() + output.pos);
						memcpy_s(&_VecOUT[_VecOUT.size() - output.pos], output.pos, buffOut, output.pos);
					}
				}
				if (readPos == _VecIN.size())
					break;//Exit
			}
			return ZSTD_stream_error::ZS_NoError;
		}

		/// @brief	Structure of the header stream
		/// @param	Header	Raw header in std::byte
		/// @param	Position	Position of the virtual header stream
		struct HeaderStream
		{
			std::vector<std::byte> Header;
			uint64_t Position = 0;
		};
		/// @brief      Header stream.
		HeaderStream mHeader;

#pragma region HEADER_Operation
		/// @brief      Read uint16_t (2 byte) data from a memory pointer with a size of 2 byte.
		uint16_t HEADER_read16()
		{
			uint16_t val; memcpy_s(&val, sizeof(val), &mHeader.Header[mHeader.Position], sizeof(val)); mHeader.Position += sizeof(val); return val;
		}
		/// @brief      Read uint32_t (4 byte) data from header with a size of 4 byte.
		uint32_t HEADER_read32()
		{
			uint32_t val; memcpy_s(&val, sizeof(val), &mHeader.Header[mHeader.Position], sizeof(val)); mHeader.Position += sizeof(val); return val;
		}
		/// @brief      Read uint64_t (8 byte) data from a header with a size of 8 byte.
		uint64_t HEADER_read64()
		{
			uint64_t val; memcpy_s(&val, sizeof(val), &mHeader.Header[mHeader.Position], sizeof(val)); mHeader.Position += sizeof(val); return val;
		}
		/// @brief      Write uint16_t (2 byte) data to a header with a size of 2 byte.
		void HEADER_write16(uint16_t value)
		{
			mHeader.Header.resize(mHeader.Header.size() + sizeof(value));
			memcpy_s(&mHeader.Header[mHeader.Position], sizeof(value), &value, sizeof(value));
			mHeader.Position += sizeof(value);
		}
		/// @brief      Write uint32_t (4 byte) data to a header with a size of 4 byte.
		void HEADER_write32(uint32_t value)
		{
			mHeader.Header.resize(mHeader.Header.size() + sizeof(value));
			memcpy_s(&mHeader.Header[mHeader.Position], sizeof(value), &value, sizeof(value));
			mHeader.Position += sizeof(value);
		}
		/// @brief      Write uint64_t (8 byte) data to a header with a size of 8 byte.
		void HEADER_write64(uint64_t value)
		{
			mHeader.Header.resize(mHeader.Header.size() + sizeof(value));
			memcpy_s(&mHeader.Header[mHeader.Position], sizeof(value), &value, sizeof(value));
			mHeader.Position += sizeof(value);
		}
#pragma endregion

		/// @brief      Initialize Archive with default settings generated.
		Archive()
		{
			mVectorKEY.clear();
			mVectorKEY.resize(32);
			mNonce = XXH3_64bits(&mVectorKEY[0], mVectorKEY.size());
			mvMagicKey.resize(8);
			memcpy_s(&mvMagicKey[0], mvMagicKey.size(), magicByte, mvMagicKey.size());
		}

		/// @brief      Unload Archive.
		~Archive()
		{
			mVectorKEY.clear();
			mNonce = 0;
		}

	private:
#ifdef USEWINDOWS_BCRYPT
		/// @brief	Encrypt a vector of std::byte using BCrypt engine
		/// @param	_VectorIn	Uncrypted Vector to encrypt
		/// @param	_VectorOut	Encrypted Vector with BCrypt
		/// @param	_IV		IV key
		/// @param	_AES256KEY	AES256 Key ()
		void BCryptEncryptVector(std::vector<std::byte>& _VectorIn, std::vector<std::byte>& _VectorOut, std::vector<std::byte> _IV, std::vector<std::byte> _AES256KEY)
		{
			NTSTATUS status;
			BCRYPT_ALG_HANDLE hAesAlg = NULL;
			BCRYPT_KEY_HANDLE hKey = NULL;
			std::vector<std::byte>pbKeyObject;
			std::vector<std::byte>pbIV;
			DWORD cbKeyObject, cbData, cbBlockLen, cbCipherText;
			// Open an algorithm handle.
			status = BCryptOpenAlgorithmProvider(
				&hAesAlg,
				BCRYPT_AES_ALGORITHM,
				NULL,
				0);
			// Calculate the size of the buffer to hold the KeyObject.
			status = BCryptGetProperty(
				hAesAlg,
				BCRYPT_OBJECT_LENGTH,
				(PBYTE)&cbKeyObject,
				sizeof(DWORD),
				&cbData,
				0);
			// Allocate the key object on the heap.
			pbKeyObject.resize(cbKeyObject);

			// Calculate the block length for the IV.
			status = BCryptGetProperty(
				hAesAlg,
				BCRYPT_BLOCK_LENGTH,
				(PBYTE)&cbBlockLen,
				sizeof(DWORD),
				&cbData,
				0);
			// Allocate a buffer for the IV. The buffer is consumed during the 
			// encrypt/decrypt process.
			pbIV.resize(cbBlockLen);
			_IV.resize(cbBlockLen);//Ensure _IV is large enough if API make this number greater then 16
			memcpy(&pbIV[0], &_IV[0], cbBlockLen);
			status = BCryptSetProperty(
				hAesAlg,
				BCRYPT_CHAINING_MODE,
				(PBYTE)BCRYPT_CHAIN_MODE_CBC,
				sizeof(BCRYPT_CHAIN_MODE_CBC),
				0);

			// Generate the key from supplied input key bytes.
			BCryptGenerateSymmetricKey(
				hAesAlg,
				&hKey,
				(PUCHAR)&pbKeyObject[0],
				cbKeyObject,
				(PUCHAR)&_AES256KEY[0],
				_AES256KEY.size(),
				0);

			// Get the output buffer size.
			BCryptEncrypt(
				hKey,
				(PUCHAR)&_VectorIn[0],
				_VectorIn.size(),
				NULL,
				(PUCHAR)&pbIV[0],
				cbBlockLen,
				NULL,
				0,
				&cbCipherText,
				BCRYPT_BLOCK_PADDING);
			_VectorOut.resize(cbCipherText);

			// Use the key to encrypt the plaintext buffer.
			// For block sized messages, block padding will add an extra block.
			status = BCryptEncrypt(
				hKey,
				(PUCHAR)&_VectorIn[0],
				_VectorIn.size(),
				NULL,
				(PUCHAR)&pbIV[0],
				cbBlockLen,
				(PUCHAR)&_VectorOut[0],
				cbCipherText,
				&cbData,
				BCRYPT_BLOCK_PADDING);

			// Destroy the key and reimport from saved BLOB.
			if (hAesAlg)
				BCryptCloseAlgorithmProvider(hAesAlg, 0);
			if (hKey)
				BCryptDestroyKey(hKey);
			pbKeyObject.clear();
			pbIV.clear();
		}
		/// @brief	Encrypt a vector of std::byte using BCrypt engine
		/// @param	_VectorIn	Uncrypted buffer to encrypt
		/// @param	_VectorOut	Encrypted Vector with BCrypt
		/// @param	_IV		IV key
		/// @param	_AES256KEY	AES256 Key ()
		void BCryptEncryptVector(void* _VectorIn, size_t szVectorIn, std::vector<std::byte>& _VectorOut, std::vector<std::byte> _IV, std::vector<std::byte> _AES256KEY)
		{
			NTSTATUS status;
			BCRYPT_ALG_HANDLE hAesAlg = NULL;
			BCRYPT_KEY_HANDLE hKey = NULL;
			std::vector<std::byte>pbKeyObject;
			std::vector<std::byte>pbIV;
			DWORD cbKeyObject, cbData, cbBlockLen, cbCipherText;
			// Open an algorithm handle.
			status = BCryptOpenAlgorithmProvider(
				&hAesAlg,
				BCRYPT_AES_ALGORITHM,
				NULL,
				0);
			// Calculate the size of the buffer to hold the KeyObject.
			status = BCryptGetProperty(
				hAesAlg,
				BCRYPT_OBJECT_LENGTH,
				(PBYTE)&cbKeyObject,
				sizeof(DWORD),
				&cbData,
				0);
			// Allocate the key object on the heap.
			pbKeyObject.resize(cbKeyObject);

			// Calculate the block length for the IV.
			status = BCryptGetProperty(
				hAesAlg,
				BCRYPT_BLOCK_LENGTH,
				(PBYTE)&cbBlockLen,
				sizeof(DWORD),
				&cbData,
				0);
			// Allocate a buffer for the IV. The buffer is consumed during the 
			// encrypt/decrypt process.
			pbIV.resize(cbBlockLen);
			_IV.resize(cbBlockLen);//Ensure _IV is large enough if API make this number greater then 16
			memcpy(&pbIV[0], &_IV[0], cbBlockLen);
			status = BCryptSetProperty(
				hAesAlg,
				BCRYPT_CHAINING_MODE,
				(PBYTE)BCRYPT_CHAIN_MODE_CBC,
				sizeof(BCRYPT_CHAIN_MODE_CBC),
				0);

			// Generate the key from supplied input key bytes.
			BCryptGenerateSymmetricKey(
				hAesAlg,
				&hKey,
				(PUCHAR)&pbKeyObject[0],
				cbKeyObject,
				(PUCHAR)&_AES256KEY[0],
				_AES256KEY.size(),
				0);

			// Get the output buffer size.
			BCryptEncrypt(
				hKey,
				(PUCHAR)_VectorIn,
				/*_VectorIn.size()*/szVectorIn,
				NULL,
				(PUCHAR)&pbIV[0],
				cbBlockLen,
				NULL,
				0,
				&cbCipherText,
				BCRYPT_BLOCK_PADDING);
			_VectorOut.resize(cbCipherText);

			// Use the key to encrypt the plaintext buffer.
			// For block sized messages, block padding will add an extra block.
			status = BCryptEncrypt(
				hKey,
				(PUCHAR)_VectorIn,
				szVectorIn/*_VectorIn.size()*/,
				NULL,
				(PUCHAR)&pbIV[0],
				cbBlockLen,
				(PUCHAR)&_VectorOut[0],
				cbCipherText,
				&cbData,
				BCRYPT_BLOCK_PADDING);

			// Destroy the key and reimport from saved BLOB.
			if (hAesAlg)
				BCryptCloseAlgorithmProvider(hAesAlg, 0);
			if (hKey)
				BCryptDestroyKey(hKey);
			pbKeyObject.clear();
			pbIV.clear();
		}
		/// @brief	Decrypt a vector of std::byte using BCrypt engine
		/// @param	_VectorIn	Encrypted Vector to decrypt
		/// @param	_VectorOut	Decrypted Vector with BCrypt
		/// @param	_IV		IV key
		/// @param	_AES256KEY	AES256 Key ()
		void BCryptDecryptVector(std::vector<std::byte>& _VectorIn, std::vector<std::byte>& _VectorOut, std::vector<std::byte> _IV, std::vector<std::byte> _AES256KEY)
		{
			BCRYPT_ALG_HANDLE hAesAlg = NULL;
			BCRYPT_KEY_HANDLE hKey = NULL;
			std::vector<std::byte>pbKeyObject;
			std::vector<std::byte>pbIV;
			DWORD cbKeyObject, cbData, cbBlockLen, cbPlainText;
			// Open an algorithm handle.
			NTSTATUS status;
			status = BCryptOpenAlgorithmProvider(
				&hAesAlg,
				BCRYPT_AES_ALGORITHM,
				NULL,
				0);
			// Calculate the size of the buffer to hold the KeyObject.
			status = BCryptGetProperty(
				hAesAlg,
				BCRYPT_OBJECT_LENGTH,
				(PBYTE)&cbKeyObject,
				sizeof(DWORD),
				&cbData,
				0);
			// Allocate the key object on the heap.
			pbKeyObject.resize(cbKeyObject);

			// Calculate the block length for the IV.
			status = BCryptGetProperty(
				hAesAlg,
				BCRYPT_BLOCK_LENGTH,
				(PBYTE)&cbBlockLen,
				sizeof(DWORD),
				&cbData,
				0);
			// Allocate a buffer for the IV. The buffer is consumed during the 
			// encrypt/decrypt process.
			pbIV.resize(cbBlockLen);
			_IV.resize(cbBlockLen);//Ensure _IV is large enough if API make this number greater then 16
			memcpy(&pbIV[0], &_IV[0], cbBlockLen);

			status = BCryptSetProperty(
				hAesAlg,
				BCRYPT_CHAINING_MODE,
				(PBYTE)BCRYPT_CHAIN_MODE_CBC,
				sizeof(BCRYPT_CHAIN_MODE_CBC),
				0);

			// Generate the key from supplied input key bytes.
			BCryptGenerateSymmetricKey(
				hAesAlg,
				&hKey,
				(PUCHAR)&pbKeyObject[0],
				cbKeyObject,
				(PUCHAR)&_AES256KEY[0],
				_AES256KEY.size(),
				0);

			_VectorOut.clear();
			//
			// Get the output buffer size.
			//
			status = BCryptDecrypt(
				hKey,
				(PUCHAR)&_VectorIn[0],
				_VectorIn.size(),
				NULL,
				(PUCHAR)&pbIV[0],
				cbBlockLen,
				NULL,
				0,
				&cbPlainText,
				BCRYPT_BLOCK_PADDING);
			_VectorOut.resize(cbPlainText);
			status = BCryptDecrypt(
				hKey,
				(PUCHAR)&_VectorIn[0],
				_VectorIn.size(),
				NULL,
				(PUCHAR)&pbIV[0],
				cbBlockLen,
				(PUCHAR)&_VectorOut[0],
				cbPlainText,
				&cbPlainText,
				BCRYPT_BLOCK_PADDING);
			_VectorOut.resize(cbPlainText);
			if (hAesAlg)
				BCryptCloseAlgorithmProvider(hAesAlg, 0);
			if (hKey)
				BCryptDestroyKey(hKey);
			pbKeyObject.clear();
			pbIV.clear();
		}
#else
		/// @brief	Add padding to Input block to be encrypted with TinyAES to be compatible with BCrypt CBC AES256 mode
		/// @param	vTOPAD	Vector to be padded 
		/// @param	PadSize	Size of padding (AES256 is 16 block by defaut)
		void AES256AddPadding(std::vector<std::byte>& vTOPAD, int PadSize = 16)
		{
			if (vTOPAD.size() == 0)
				return;
			int ChrToAdd = vTOPAD.size() % PadSize;
			if (ChrToAdd == 0)
				ChrToAdd = PadSize;
			else
				ChrToAdd = PadSize - ChrToAdd;
			vTOPAD.resize(vTOPAD.size() + ChrToAdd);
			memset(&vTOPAD[vTOPAD.size() - ChrToAdd], (uint8_t)ChrToAdd, ChrToAdd);
			return;
		}
		/// @brief	Remove padding to Input block after decryption with TinyAES to be compatible with BCrypt CBC AES256 mode
		/// @param	vTPUNPAD	Vector to be unpadded 
		void AddAES256RemovePadding(std::vector<std::byte>& vTPUNPAD)
		{
			if (vTPUNPAD.size() == 0)
				return;
			int Pad2Remove = (int)vTPUNPAD[vTPUNPAD.size() - 1];
			vTPUNPAD.resize(vTPUNPAD.size() - Pad2Remove);
			return;
		}
#endif // USEWINDOWS_BCRYPT

		/// @brief      Convert _stati64 time_t to FILETIME eg. TimetToFileTime(buf.st_modec, &pFileTime)
		void TimetToFileTime(time_t t, LPFILETIME pft)
		{
			LONGLONG time_value = Int32x32To64(t, 10000000) + 116444736000000000;
			pft->dwLowDateTime = (DWORD)time_value;
			pft->dwHighDateTime = time_value >> 32;
		}

		ArchiveEntry mCachedArchive;//Local cached archiveEntry 

		/// @brief      Local pointer for thread management in Zstd (0 or 1 mean single thread,more is number of thread).
		size_t mMaxThread = 0;

		/// @brief      Mutex used to lock Compression progression when called asynchronously.
		std::mutex MutexComp;
		/// @brief      Mutex used to lock Uncompression progression when called asynchronously.
		std::mutex MutexUncomp;
		/// @brief      Compression progression report.
		_ProgressReporting mProgressComp;
		/// @brief      Uncompression progression report.
		_ProgressReporting mProgressUncomp;
		/// @brief      vector of 8 byte containing the magic key sequence identifying NZAʀᴄʜɪᴠᴇ format.
		std::vector<uint8_t> mvMagicKey;
		/// @brief      vector of 32 byte containing raw key.
		std::vector<std::byte> mVectorKEY;
		/// @brief      Uint64_t Nonce : Arbitrary number used as initialization encryption vector defaut when no password is provided.
		uint64_t mNonce = XXH3_64bits(std::wstring(L"").data(), std::wstring(L"").size());

	};

	/*	_ _ _ _         _       _        _____     _                _____ __    _____ _____ _____
		| | | |_|___ ___| |_ ___|_|___   |  |  |___| |___ ___ ___   |     |  |  |  _  |   __|   __|
		| | | | |  _| -_|   | .'| |  _|  |     | -_| | . | -_|  _|  |   --|  |__|     |__   |__   |
		|_____|_|_| |___|_|_|__,|_|_|    |__|__|___|_|  _|___|_|    |_____|_____|__|__|_____|_____|
													 |_|
	*/

	/// @brief	Wirehair helper class
	class WireHairHelper
	{
	public:
		// Generation error codelist
		enum class GenerateError : uint8_t {
			G_Success, G_CantOpenNPARFile, G_CantOpenArchiveFile, G_CantInitWirehair, G_UnknownError, G_UserInterrupted
		};
		// Control and repair error codelist
		enum class CandRError : uint8_t {
			C_CantOpenNPARFile, C_CantOpenArchiveFile, C_ArchiveIsNotSameSize, C_NeedNoRepair, C_CantCreateRepairFile, C_UnknownError, C_RepairSuccess, C_RepairFailedNotEnoughBlockToRepair, G_UserInterrupted
		};
		// Progression status
		enum class Progression : uint8_t {
			P_None, P_CreateTable, P_AnalysingFile, P_Repairing, P_Error, P_Success
		};


		/// @brief	Set path of NPAR file and archive file
		/// @param	FileToProceed	Full path of the file to proceed
		/// @param	FileNPAR	Length of the random wstring sequence
		void SetPATH(std::wstring FileToProceed, std::wstring FileNPAR)
		{
			mFiletoprocess = FileToProceed;
			mFilePar = FileNPAR;
			mCreateProgress = 0.0;
			mCreateType = Progression::P_None;
			mRepairProgress = 0.0;
			mRepairType = Progression::P_None;
		}

		/// @brief	Helper initialization
		WireHairHelper()
		{
			const WirehairResult initResult = wirehair_init();
			mFiletoprocess = L"";
			mFilePar = L"";
		}

		/// @brief	Helper liberation
		~WireHairHelper()
		{}

		/// @brief	Generate a given percentage of recovery block for the source file
		/// @param	PercentRedundancy	Percentage of recovery block to generate basing of the source file
		/// @return	GenerateError with the generation status
		GenerateError GenerateNPARfile(int PercentRedundancy)
		{
			mtxCreate.lock();
			mCreateProgress = 0.0;
			mCreateType = Progression::P_CreateTable;
			mtxCreate.unlock();
			FILE* fSRC = NULL;
			FILE* fDST = NULL;
			_wfopen_s(&fSRC, mFiletoprocess.c_str(), L"rb");
			if (fSRC == NULL)
			{
				mtxCreate.lock();
				mCreateProgress = 0.0;
				mCreateType = Progression::P_Error;
				mtxCreate.unlock();
				return GenerateError::G_CantOpenArchiveFile;
			}
			_wfopen_s(&fDST, mFilePar.c_str(), L"wb");
			if (fDST == NULL)
			{
				mtxCreate.lock();
				mCreateProgress = 0.0;
				mCreateType = Progression::P_Error;
				mtxCreate.unlock();
				fclose(fSRC);
				return GenerateError::G_CantOpenNPARFile;
			}
			uint64_t FILESize = std::filesystem::file_size(mFiletoprocess);
			uint64_t RemainSize = FILESize;
			uint64_t BLOCKid = -1;
			FILE_write64(fDST, PercentRedundancy);//Percentage
			FILE_write64(fDST, kMessageBytes);//Defaut message (buffer) size
			FILE_write64(fDST, kPacketSize);//Defaut packet size
			FILE_write64(fDST, FILESize);//Filesize

			while (true)//For each 32MB block
			{
				BLOCKid++;
				uint64_t BUFFER = kMessageBytes;
				if (RemainSize <= kMessageBytes)
					BUFFER = RemainSize;
				std::vector<uint8_t> message(BUFFER);
				fread_s(&message[0], BUFFER, sizeof(std::byte), BUFFER, fSRC);
				WirehairCodec encoder = wirehair_encoder_create(nullptr, &message[0], BUFFER, kPacketSize);
				if (!encoder)
				{
					mtxCreate.lock();
					mCreateProgress = 0.0;
					mCreateType = Progression::P_Error;
					mtxCreate.unlock();
					fclose(fDST);
					fclose(fSRC);
					return GenerateError::G_CantInitWirehair;
				}
				int nbBlockBase = BUFFER / kPacketSize;
				if (BUFFER % kPacketSize > 0) nbBlockBase++;//Round up block size, if null, at least one block is created with content
				FILE_write64(fDST, BLOCKid);//BlockNumber
				FILE_write64(fDST, nbBlockBase);//BaseBlock
				for (size_t i = 0; i < nbBlockBase; i++)//Create packet hash table
				{
					uint64_t szPacket = ((message.size() - (kPacketSize * i)) >= kPacketSize) ? kPacketSize : message.size() - (kPacketSize * i);
					uint64_t hashPacket = XXH3_64bits(&message[kPacketSize * i], szPacket);
					FILE_write64(fDST, hashPacket);//BaseBlock
				}
				int nbBlockToCreate = (int)((((double)BUFFER / 100.0) * (double)PercentRedundancy) / (double)kPacketSize);
				FILE_write64(fDST, nbBlockToCreate);//BaseBlock
				FILE_write64(fDST, XXH3_64bits(&message[0], message.size()));//HASHSOURCE
				std::vector<uint8_t> Fountain;
				for (size_t i = 0; i < nbBlockToCreate; i++)//100 equal 10 percent of 32Mb (3,2Mb)
				{
					std::vector<uint8_t> block(kPacketSize);	// Encode a packet
					uint32_t writeLen = 0;
					WirehairResult encodeResult = wirehair_encode(
						encoder, // Encoder object
						nbBlockBase + i, // ID of block to generate (nbblockbase = original file, after is redundant data)
						&block[0], // Output buffer
						kPacketSize, // Output buffer size
						&writeLen); // Returned block length
					Fountain.resize(Fountain.size() + block.size());
					memcpy_s(&Fountain[Fountain.size() - block.size()], block.size(), &block[0], block.size());
				}
				FILE_write64(fDST, XXH3_64bits(&Fountain[0], Fountain.size()));//HASHSOURCE
				FILE_write64(fDST, Fountain.size());//HASHSOURCE
				fwrite(&Fountain[0], sizeof(std::byte), Fountain.size(), fDST);
				mtxCreate.lock();
				mCreateProgress = ((double)_ftelli64(fSRC) * 100.0) / (double)FILESize;
				mtxCreate.unlock();
				wirehair_free(encoder);
				RemainSize -= BUFFER;
				if (RemainSize == 0)//End reached
					break;
			}
			fclose(fDST);
			fclose(fSRC);
			mtxCreate.lock();
			mCreateProgress = 100.0;
			mCreateType = Progression::P_Success;
			mtxCreate.unlock();
			return GenerateError::G_Success;
		}

		/// @brief	Return the progression of the generation operation 
		/// @param	_CreateType	(OUT) Retrieve the operation step
		/// @return	DOUBLE with the progresion value
		double GenerateProgress(Progression& _CreateType)
		{
			double lProgress = 0.0;
			mtxCreate.lock();
			lProgress = mCreateProgress;
			_CreateType = mCreateType;
			mtxCreate.unlock();
			return lProgress;
		}

		/// @brief	Return the progression of the Control/repairing operation 
		/// @param	ProgType	(OUT) Retrieve the operation step
		/// @return	DOUBLE with the progresion value
		double ControlAndRepairProgress(Progression& ProgType)
		{
			double lProgress = 0.0;
			mtxRepair.lock();
			lProgress = mRepairProgress;
			ProgType = mRepairType;
			mtxRepair.unlock();
			return lProgress;
		}

		/// @brief	Structure of the parity block
		/// @param	PacketHash	Vector with the hash of each packet
		/// @param	BaseOffset	Offset in parity table
		/// @param	BLOCKid		Id for this block
		/// @param	nbBlockBase	How many base block are present
		/// @param	nbBlockToCreate	How many block to be created based on percentage value
		/// @param	HashSource	Hash of the source block
		/// @param	HashRecovery	Hash of the recovery block
		/// @param	DataSize		Size of the block
		/// @param	BlockCorrupted	Is the block corrupted
		/// @param	needRepairing	Block need repairing (used in reparation progress independently of BlockCorrupted)
		struct TableNPARblock
		{
			std::vector<uint64_t>PacketHash;
			uint64_t BaseOffset;
			uint64_t BLOCKid;
			uint64_t nbBlockBase;
			uint64_t nbBlockToCreate;
			uint64_t HashSource;
			uint64_t HashRecovery;
			uint64_t DataSize;
			bool BlockCorrupted = false;
			bool needRepairing = false;
		};


		/// @brief	Structure of the parity table 
		/// @param	Block			Vector parity block
		/// @param	PercentRedundancy	Percentage of redundancy
		/// @param	kMessageBytes		Maximum size for a block
		/// @param	kPacketSize		Size of the packet
		/// @param	FILESize			Size of the complete file
		struct TableNPAR
		{
			uint64_t PercentRedundancy;//Percentage
			uint64_t kMessageBytes;//Message size (Buffer)
			uint64_t kPacketSize;//Packet size
			uint64_t FILESize;//File size
			std::vector<TableNPARblock> Block;
		};

		/// @brief	Control and repair if needed the file set with SetFile
		/// @return	CandRError with the control/reparation status
		CandRError ControlAndRepair()
		{
			/*	First thing first, let's parse the table*/
			TableNPAR mTable;
			uint64_t ArcFileSize;
			FILE* fTable = NULL;
			mtxRepair.lock();
			mRepairProgress = 0.0;
			mRepairType = Progression::P_None;
			mtxRepair.unlock();
			_wfopen_s(&fTable, mFilePar.c_str(), L"rb");
			if (fTable != NULL)
			{
				_fseeki64(fTable, 0, SEEK_END);
				uint64_t TableFileSize = _ftelli64(fTable);
				_fseeki64(fTable, 0, SEEK_SET);
				mTable.PercentRedundancy = FILE_read64(fTable);
				mTable.kMessageBytes = FILE_read64(fTable);
				mTable.kPacketSize = FILE_read64(fTable);
				mTable.FILESize = FILE_read64(fTable);
				mtxRepair.lock();
				mRepairProgress = 0.0;
				mRepairType = Progression::P_CreateTable;
				mtxRepair.unlock();

				while (true)
				{
					bool _STOP = false;
					MutexGET(mtxStop, mStop, _STOP);
					if (_STOP)
					{
						fclose(fTable);
						return CandRError::G_UserInterrupted;
					}
					TableNPARblock mBlock;
					mBlock.BLOCKid = FILE_read64(fTable);
					mBlock.nbBlockBase = FILE_read64(fTable);
					mBlock.PacketHash.resize(mBlock.nbBlockBase);
					for (size_t i = 0; i < mBlock.nbBlockBase; i++)
						mBlock.PacketHash[i] = FILE_read64(fTable);
					mBlock.nbBlockToCreate = FILE_read64(fTable);
					mBlock.HashSource = FILE_read64(fTable);
					mBlock.HashRecovery = FILE_read64(fTable);
					mBlock.DataSize = FILE_read64(fTable);
					mBlock.BaseOffset = _ftelli64(fTable);
					std::vector<std::byte> vRead(mBlock.DataSize);
					fread_s(&vRead[0], mBlock.DataSize, sizeof(std::byte), mBlock.DataSize, fTable);
					uint64_t HashData = XXH3_64bits(&vRead[0], vRead.size());
					vRead.clear();
					if (HashData != mBlock.HashRecovery)
						mBlock.BlockCorrupted = true;
					mTable.Block.push_back(mBlock);
					mtxRepair.lock();
					mRepairProgress = FastPERCd(mBlock.BaseOffset, TableFileSize);
					mRepairType = Progression::P_CreateTable;
					mtxRepair.unlock();
					if (_ftelli64(fTable) == TableFileSize)//End of file
						break;
				}
				_fseeki64(fTable, 0, SEEK_SET);//Rewinding file index
				/*	Table is parsed, now going to check source file	*/

				FILE* fArchive = NULL;
				_wfopen_s(&fArchive, mFiletoprocess.c_str(), L"rb");
				if (fArchive != NULL)
				{
					mtxRepair.lock();
					mRepairProgress = 0.0;
					mRepairType = Progression::P_AnalysingFile;
					mtxRepair.unlock();
					_fseeki64(fArchive, 0, SEEK_END);
					ArcFileSize = _ftelli64(fArchive);
					_fseeki64(fArchive, 0, SEEK_SET);
					if (ArcFileSize != mTable.FILESize)
					{
						mtxRepair.lock();
						mRepairProgress = 0.0;	mRepairType = Progression::P_Error;
						mtxRepair.unlock();
						return CandRError::C_ArchiveIsNotSameSize;
					}
					for (size_t i = 0; i < mTable.Block.size(); i++)
					{
						bool _STOP = false;
						MutexGET(mtxStop, mStop, _STOP);
						if (_STOP)
						{
							fclose(fTable);
							fclose(fArchive);
							return CandRError::G_UserInterrupted;
						}
						_fseeki64(fArchive, mTable.kMessageBytes * i, SEEK_SET);
						std::vector<std::byte> vRead((i == (mTable.Block.size() - 1) ?/*LastPart*/ArcFileSize - _ftelli64(fArchive) : mTable.kMessageBytes));
						fread_s(&vRead[0], vRead.size(), sizeof(std::byte), vRead.size(), fArchive);
						uint64_t HashData = XXH3_64bits(&vRead[0], vRead.size());
						vRead.clear();
						if (HashData != mTable.Block[i].HashSource)
							mTable.Block[i].needRepairing = true;
						mtxRepair.lock();
						mRepairProgress = FastPERCd(i, mTable.Block.size());
						mRepairType = Progression::P_AnalysingFile;
						mtxRepair.unlock();
					}
				}
				else
				{
					fclose(fTable);
					mtxRepair.lock();
					mRepairProgress = 0.0;	mRepairType = Progression::P_Error;
					mtxRepair.unlock();
					return CandRError::C_CantOpenArchiveFile;
				}
				bool _NeedRecovery = false;
				for (size_t i = 0; i < mTable.Block.size(); i++)
					if (mTable.Block[i].needRepairing) _NeedRecovery = true;
				if (_NeedRecovery == false)
				{
					fclose(fArchive);
					fclose(fTable);
					mtxRepair.lock();
					mRepairProgress = 100.0;	mRepairType = Progression::P_Success;
					mtxRepair.unlock();
					return CandRError::C_NeedNoRepair;
				}
				/*	We are going to repair corrupted block	*/
				_fseeki64(fArchive, 0, SEEK_SET);//Resetting file pointer to origin
				_fseeki64(fTable, 0, SEEK_SET);
				FILE* fREPAIRED = NULL;
				_wfopen_s(&fREPAIRED, std::wstring(mFiletoprocess + L".repaired").c_str(), L"wb");
				bool FailedToRecover = false;
				mtxRepair.lock();
				mRepairProgress = 0.0;
				mRepairType = Progression::P_Repairing;
				mtxRepair.unlock();

				if (fREPAIRED != NULL)
				{
					for (size_t i = 0; i < mTable.Block.size(); i++)
					{
						bool _STOP = false;
						MutexGET(mtxStop, mStop, _STOP);
						if (_STOP)
						{
							fclose(fTable);
							fclose(fArchive);
							return CandRError::G_UserInterrupted;
						}
						mtxRepair.lock();
						mRepairProgress = FastPERCd(i, mTable.Block.size());
						mRepairType = Progression::P_Repairing;
						mtxRepair.unlock();

						uint64_t BUFFER = ((mTable.FILESize - _ftelli64(fArchive)) >= kMessageBytes) ? kMessageBytes : mTable.FILESize - _ftelli64(fArchive);
						std::vector<std::byte> vReadFromArc(BUFFER);
						fread_s(&vReadFromArc[0], vReadFromArc.size(), sizeof(std::byte), vReadFromArc.size(), fArchive);
						if (!mTable.Block[i].needRepairing)
						{//Need no repairing, Copying whole chunk
							fwrite(&vReadFromArc[0], sizeof(std::byte), vReadFromArc.size(), fREPAIRED);
						}
						else
						{
							//Creating table for this block
							std::vector<std::vector<std::uint8_t>> RepairTable(mTable.Block[i].nbBlockToCreate);
							_fseeki64(fTable, mTable.Block[i].BaseOffset, SEEK_SET);//Resetting file pointer to origin
							for (size_t j = 0; j < mTable.Block[i].nbBlockToCreate; j++)
							{
								RepairTable[j].resize(kPacketSize);
								fread_s(&RepairTable[j][0], RepairTable[j].size(), sizeof(std::byte), RepairTable[j].size(), fTable);
							}
							//We feed Original Block to Decoder
							WirehairCodec encoder = wirehair_encoder_create(nullptr, &vReadFromArc[0], vReadFromArc.size(), kPacketSize);
							WirehairCodec decoder = wirehair_decoder_create(nullptr, vReadFromArc.size(), kPacketSize);
							WirehairResult encodeResult;
							WirehairResult decodeResult;
							for (size_t j = 0; j < mTable.Block[i].nbBlockBase; j++)
							{
								//Hashing
								uint64_t szPacket = ((vReadFromArc.size() - (kPacketSize * j)) >= kPacketSize) ? kPacketSize : vReadFromArc.size() - (kPacketSize * j);
								uint64_t hashPacket = XXH3_64bits(&vReadFromArc[kPacketSize * j], szPacket);
								if (hashPacket == mTable.Block[i].PacketHash[j])
								{//Same hash, feeding
									std::vector<uint8_t> block(kPacketSize);
									uint32_t writeLen = 0;
									encodeResult = wirehair_encode(
										encoder, // Encoder object
										j, // ID of block to generate
										&block[0], // Output buffer
										kPacketSize, // Output buffer size
										&writeLen); // Returned block length
									printf("");
									decodeResult = wirehair_decode(
										decoder, // Decoder object
										j, // ID of block that was encoded
										&block[0], // Input block
										writeLen); // Block length
								}
							}
							for (size_t j = 0; j < RepairTable.size(); j++)
							{
								decodeResult = wirehair_decode(
									decoder, // Decoder object
									mTable.Block[i].nbBlockBase + j, // ID of block that was encoded
									&RepairTable[j][0], // Input block
									RepairTable[j].size()); // Block length
								if (decodeResult == Wirehair_Success)// Decoder has enough data to recover now
									break;
								if (decodeResult != Wirehair_NeedMore)//!!! wirehair_decode failed:
								{//Writing unrepaired block, sorry...
									fwrite(&vReadFromArc[0], sizeof(std::byte), vReadFromArc.size(), fREPAIRED);
									FailedToRecover = true;
								}
							}
							if (decodeResult == Wirehair_NeedMore)
							{
								fwrite(&vReadFromArc[0], sizeof(std::byte), vReadFromArc.size(), fREPAIRED);
								FailedToRecover = true;
							}
							else
							{
								std::vector<uint8_t> decoded(vReadFromArc.size());
								decodeResult = wirehair_recover(
									decoder,
									&decoded[0],
									vReadFromArc.size());
								if (decodeResult != Wirehair_Success)
								{//Writing unrepaired block, sorry...
									fwrite(&vReadFromArc[0], sizeof(std::byte), vReadFromArc.size(), fREPAIRED);
									FailedToRecover = true;
								}
								else
								{
									fwrite(&decoded[0], sizeof(std::byte), decoded.size(), fREPAIRED);
								}
								decoded.clear();
							}
							for (size_t j = 0; j < RepairTable.size(); j++)
								RepairTable[j].clear();
							RepairTable.clear();
							wirehair_free(decoder);
							wirehair_free(encoder);

						}
						vReadFromArc.clear();
					}
					fclose(fREPAIRED);
				}
				else
				{
					fclose(fArchive);
					fclose(fTable);
					mtxRepair.lock();
					mRepairProgress = 0.0;	mRepairType = Progression::P_Error;
					mtxRepair.unlock();
					return CandRError::C_CantCreateRepairFile;
				}
				fclose(fArchive);
				fclose(fTable);
				uint64_t szRepaired = 0;
				_wfopen_s(&fREPAIRED, std::wstring(mFiletoprocess + L".repaired").c_str(), L"rb");
				if (fREPAIRED != NULL)
				{
					_fseeki64(fREPAIRED, 0, SEEK_END);
					szRepaired = _ftelli64(fREPAIRED);
					fclose(fREPAIRED);
				}

				if (szRepaired == ArcFileSize)
				{//Wether file is fully recovered or not, we keep it because size is the same, even if damaged, maybe you can recover more files this way...

					int result = _wremove(mFiletoprocess.c_str());	// 0 = Done	else Error
					if (result == 0)
					{
						result = _wrename(std::wstring(mFiletoprocess + L".repaired").c_str(), mFiletoprocess.c_str());	// 0 = Done	else Error
					}
				}
				else
				{
					int result = _wremove(std::wstring(mFiletoprocess + L".repaired").c_str());	// 0 = Done	else Error
				}
				if (FailedToRecover)
				{
					mtxRepair.lock();
					mRepairProgress = 0.0;	mRepairType = Progression::P_Error;
					mtxRepair.unlock();
					return CandRError::C_RepairFailedNotEnoughBlockToRepair;
				}
				else
				{
					mtxRepair.lock();
					mRepairProgress = 100.0;	mRepairType = Progression::P_Success;
					mtxRepair.unlock();
					return CandRError::C_RepairSuccess;
				}
			}
			else
			{
				mtxRepair.lock();
				mRepairProgress = 0.0;	mRepairType = Progression::P_Error;
				mtxRepair.unlock();
				return CandRError::C_CantOpenNPARFile;
			}
			mtxRepair.lock();
			mRepairProgress = 0.0;	mRepairType = Progression::P_Error;
			mtxRepair.unlock();
			return CandRError::C_UnknownError;
		}

		void StopOperation()
		{
			mtxStop.lock();
			mStop = true;
			mtxStop.unlock();
		}

#ifdef EXPERIMENT_WIREHAIR
		/// @brief	Structure of the parity table 
		/// @param	OrigFile			File to corrupt
		/// @param	MessedFile			File corrupted (Never overwrite original one))
		/// @param	CorruptionPercentage			Percentage of corruption
		/// @param	CorruptOneBlockOfTwo			Corrupt each block or even one only (With this, there good block and bad block)
		void CorruptFileVoluntary(std::wstring OrigFile, std::wstring MessedFile, double CorruptionPercentage, bool CorruptOneBlockOfTwo = true)
		{
			//Personnal purpose, no error checking
			FILE* fSRC = NULL;
			FILE* fMES = NULL;
			_wfopen_s(&fSRC, OrigFile.c_str(), L"rb");
			_wfopen_s(&fMES, MessedFile.c_str(), L"wb");
			uint64_t FileSIZE = std::filesystem::file_size(OrigFile);
			bool CorruptBOOL = true;
			while (true)
			{
				uint64_t BUFFER = ((FileSIZE - _ftelli64(fSRC)) >= kMessageBytes) ? kMessageBytes : FileSIZE - _ftelli64(fSRC);
				std::vector<std::byte> vRead(BUFFER);
				fread_s(&vRead[0], vRead.size(), sizeof(std::byte), vRead.size(), fSRC);
				uint64_t CorruptRange = (CorruptionPercentage * ((double)((double)BUFFER / (double)kPacketSize) / 100.0)) * kPacketSize;
				if (CorruptOneBlockOfTwo)
				{
					if (CorruptBOOL)
						memset(&vRead[0], '\0', CorruptRange);
					CorruptBOOL = !CorruptBOOL;
				}
				else
					memset(&vRead[0], '\0', CorruptRange);
				fwrite(&vRead[0], sizeof(std::byte), vRead.size(), fMES);
				if (_ftelli64(fSRC) == std::filesystem::file_size(OrigFile))
					break;
			}
			fclose(fSRC);
			fclose(fMES);
		}
#endif
	private:
		std::mutex mtxCreate;								// Interthread lock for progression information
		double mCreateProgress = 0.0;						// Creation progression
		Progression mCreateType = Progression::P_None;		//Progression detailed step

		std::mutex mtxRepair;								// Interthread lock for progression information
		double mRepairProgress = 0.0;						//Control/Reparaion progress
		Progression mRepairType = Progression::P_None;		//Progression detailed step

		std::wstring mFiletoprocess = L"";					//File to process
		std::wstring mFilePar = L"";						//NParity file to create/use
		static const int kMessageBytes = 256 * 1024 * 1024;	//Size of a block
		static const int kPacketSize = 256 * 1024;			//Size of a packet within a block

		std::mutex mtxStop;									// Interthread lock for stop operation
		bool mStop = false;									// Stopping operation ?
	};
}