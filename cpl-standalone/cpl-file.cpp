/*
 * cpl-file.cpp
 * Core Provenance Library
 *
 * Copyright 2012
 *      The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Contributor(s): Peter Macko
 */

#include "stdafx.h"

#include <cplxx.h>
#include <cpl-file.h>

#include "cpl-platform.h"

#ifdef _WINDOWS
#elif defined(__APPLE__)
#include <CommonCrypto/CommonDigest.h>
#else
#include <openssl/sha.h>
#endif


/***************************************************************************/
/** Helpers                                                               **/
/***************************************************************************/

/**
 * Compute the SHA1 fingerprint of a file.
 *
 * @param name the file name, can be either an absolute or a relative path
 * @param out the output buffer
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_file_sha1(const char* name, cpl_sha1_t* out)
{
	if (name == NULL || out == NULL) return CPL_E_INVALID_ARGUMENT;

	unsigned char* buffer = (unsigned char*) alloca(4096);
	if (buffer == NULL) return CPL_E_INSUFFICIENT_RESOURCES;

#ifdef _WINDOWS
	FILE* f = NULL;
	errno_t err = fopen_s(&f, name, "rb");
	if (err != 0) f = NULL;
#else
	FILE* f = fopen(name, "rb");
#endif
	if (f == NULL) return CPL_E_NOT_FOUND;

#ifdef _WINDOWS
	DWORD _user_size = 256;
	DWORD _program_size = 4096;
	char* _user = new char[_user_size];
	if (_user == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	BOOL _ok = GetUserName(_user, &_user_size);
	if (!_ok) {
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			delete[] _user;
			_user = new char[_user_size];
			if (_user == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
			_ok = GetUserName(_user, &_user_size);
		}
	}
	if (!_ok) {
		delete[] _user;
		fclose(f);
		return CPL_E_PLATFORM_ERROR;
	}
	HCRYPTPROV hCryptProv;
	HCRYPTHASH hHash;
	if (!CryptAcquireContext(&hCryptProv, _user,
		NULL, PROV_RSA_FULL, 0)) {
		if (GetLastError() == NTE_BAD_KEYSET) {
			if (!CryptAcquireContext(&hCryptProv, _user,
				NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
				fclose(f);
				delete[] _user;
				return CPL_E_PLATFORM_ERROR;
			}
		}
		else {
			fclose(f);
			delete[] _user;
			return CPL_E_PLATFORM_ERROR;
		}
	}
	delete[] _user;
	if (!CryptCreateHash(hCryptProv, CALG_SHA1,
		0, 0, &hHash)) {
		fclose(f);
		CryptReleaseContext(hCryptProv, 0);
		return CPL_E_PLATFORM_ERROR;
	}
#elif defined(__APPLE__)
	CC_SHA1_CTX ctx;
	if (!CC_SHA1_Init(&ctx)) {
		fclose(f);
		return CPL_E_PLATFORM_ERROR;
	}
#else
	SHA_CTX ctx;
	if (!SHA1_Init(&ctx)) {
		fclose(f);
		return CPL_E_PLATFORM_ERROR;
	}
#endif
	
	while (!feof(f)) {
		size_t l = fread(buffer, 1, 4096, f);
		if (ferror(f)) {
			fclose(f);
#ifdef _WINDOWS
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);
#endif
			return CPL_E_PLATFORM_ERROR;
		}

#ifdef _WINDOWS
		if (!CryptHashData(hHash, buffer, l, 0)) {
			fclose(f);
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);
			return CPL_E_PLATFORM_ERROR;
		}
#elif defined(__APPLE__)
		if (!CC_SHA1_Update(&ctx, buffer, l)) {
			fclose(f);
			return CPL_E_PLATFORM_ERROR;
		}
#else
		if (!SHA1_Update(&ctx, buffer, l)) {
			fclose(f);
			return CPL_E_PLATFORM_ERROR;
		}
#endif
	}
	fclose(f);

#ifdef _WINDOWS
	DWORD hl = sizeof(*out);
	BOOL ok = CryptGetHashParam(hHash, HP_HASHVAL,
		(BYTE*) out->bytes, &hl, 0);
	CryptDestroyHash(hHash);
	CryptReleaseContext(hCryptProv, 0);
	if (!ok) return CPL_E_PLATFORM_ERROR;
#elif defined(__APPLE__)
	if (!CC_SHA1_Final((unsigned char*) out->bytes, &ctx)) {
		return CPL_E_PLATFORM_ERROR;
	}
#else
	if (!SHA1_Final((unsigned char*) out->bytes, &ctx)) {
		return CPL_E_PLATFORM_ERROR;
	}
#endif

	return CPL_OK;
}



/***************************************************************************/
/** File Provenance API                                                   **/
/***************************************************************************/

/// Hex digits
static const char __HEX[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
							   'A', 'B', 'C', 'D', 'E', 'F'};


/**
 * Lookup or create a provenance object associated with the given file.
 *
 * @param name the file name, can be either an absolute or a relative path
 * @param type the object type
 * @param flags a logical combination of CPL_F_* flags
 * @param out_id the pointer to store the ID of the provenance object
 * @param out_version the pointer to store the version of the provenance object
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_file(const char* name,
				const int flags,
				cpl_id_t* out_id,
				cpl_version_t* out_version)
{
	cpl_return_t ret = CPL_E_INTERNAL_ERROR;


	// Get the real path of the file

#ifdef _WINDOWS
	DWORD bl = 1024;
	DWORD old_bl = bl;
	char* path = (char*) malloc(bl);
	if (path == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	bl = GetFullPathName(name, bl, path, NULL);
	if (bl > old_bl) {
		free(path);
		bl += 4; old_bl = bl;
		path = (char*) malloc(bl);
		if (path == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
		bl = GetFullPathName(name, bl, path, NULL);
	}
	if (bl <= 0 || bl > old_bl) {
		free(path);
		path = NULL;
	}
#else
	char* path = realpath(name, NULL);
#endif
	if (path == NULL) {
#ifdef _WINDOWS
		FILE* f = NULL;
		errno_t err = fopen_s(&f, name, "rb");
		if (err != 0) f = NULL;
#else
		FILE* f = fopen(name, "rb");
#endif
		if (f == NULL) return CPL_E_NOT_FOUND;
		fclose(f);
		return CPL_E_PLATFORM_ERROR;
	}


	// If the user wants to always create the file, do so
	
	if ((flags & CPL_F_ALWAYS_CREATE) != 0) {
		cpl_id_t id = CPL_NONE;
		ret = cpl_create_object(CPL_O_FILESYSTEM, path,CPL_T_FILE,CPL_NONE,&id);
		if (out_version != NULL) *out_version = 0;

		if ((flags & CPL_F_OPEN_BY_CONTENT) != 0) {
			cpl_sha1_t sha;
			ret = cpl_file_sha1(name, &sha);
			if (!CPL_IS_OK(ret)) {
				free(path);
				return ret;
			}
			char sha_str[sizeof(sha) * 2 + 8];
			for (size_t i = 0; i < sizeof(sha); i++) {
				sha_str[i*2    ] = __HEX[(sha.bytes[i] >> 4) & 0xF];
				sha_str[i*2 + 1] = __HEX[(sha.bytes[i]     ) & 0xF];
			}
			sha_str[sizeof(sha)*2] = '\0';

			cpl_return_t r = cpl_add_property(id, CPL_P_SHA1, sha_str);
			if (!CPL_IS_OK(r)) {
				free(path);
				return r;
			}

			if (out_version != NULL) {
				r = cpl_get_version(id, out_version);
				if (!CPL_IS_OK(r)) {
					free(path);
					return r;
				}
			}
		}

		if (out_id != NULL) *out_id = id;

		free(path);
		if (ret == CPL_S_OK) ret = CPL_S_OBJECT_CREATED;
		return ret;
	}


	// See if the user wants to open the file by content
	
	if ((flags & CPL_F_OPEN_BY_CONTENT) != 0) {
		cpl_sha1_t sha;
		ret = cpl_file_sha1(name, &sha);
		if (!CPL_IS_OK(ret)) {
			free(path);
			return ret;
		}
		char sha_str[sizeof(sha) * 2 + 8];
		for (size_t i = 0; i < sizeof(sha); i++) {
			sha_str[i*2    ] = __HEX[(sha.bytes[i] >> 4) & 0xF];
			sha_str[i*2 + 1] = __HEX[(sha.bytes[i]     ) & 0xF];
		}
		sha_str[sizeof(sha)*2] = '\0';

		std::vector<cpl_id_version_t> v;
		ret = cpl_lookup_by_property(CPL_P_SHA1, sha_str,
				cpl_cb_collect_property_lookup_vector, &v);

		if (!v.empty()) {

			// Get the most recent version for each ID
			
			cpl_hash_map_id_t<cpl_version_t>::type m;
			for (size_t i = 0; i < v.size(); i++) {
				cpl_hash_map_id_t<cpl_version_t>::type::iterator itr;
				itr = m.find(v[i].id);
				if (itr == m.end()) {
					m[v[i].id] = v[i].version;
				}
				else {
					if (itr->second < v[i].version) {
						m[v[i].id] = v[i].version;
					}
				}
			}

			if (m.empty()) {
				free(path);
				return CPL_E_INTERNAL_ERROR;
			}


			// If there are still multiple options, get the latest one

			cpl_id_version_t r;
			cpl_hash_map_id_t<cpl_version_t>::type::iterator itr = m.begin();
			r.id = itr->first;
			r.version = itr->second;

			if (m.size() > 1) {
				unsigned long t;
				cpl_version_info_t* vinfo;
				ret = cpl_get_version_info(r.id, r.version, &vinfo);
				if (!CPL_IS_OK(ret)) {
					free(path);
					return ret;
				}
				t = vinfo->creation_time;
				cpl_free_version_info(vinfo);

				for (itr++; itr != m.end(); itr++) {
					ret = cpl_get_version_info(itr->first, itr->second, &vinfo);
					if (!CPL_IS_OK(ret)) {
						free(path);
						return ret;
					}
					if (t < vinfo->creation_time) {
						t = vinfo->creation_time;
						r.id = itr->first;
						r.version = itr->second;
					}
					cpl_free_version_info(vinfo);
				}
			}


			// Open
			
			if (out_id != NULL) *out_id = r.id;
			if (out_version != NULL) *out_version = r.version;

			free(path);
			return ret;
		}


		// Create the object if it does not exist
		
		if ((flags & CPL_F_CREATE_IF_DOES_NOT_EXIST) != 0) {
			cpl_id_t id = CPL_NONE;
			ret = cpl_create_object(CPL_O_FILESYSTEM, path, CPL_T_FILE,
									CPL_NONE, &id);
			if (!CPL_IS_OK(ret)) {
				free(path);
				return ret;
			}

			cpl_return_t r = cpl_add_property(id, CPL_P_SHA1, sha_str);
			if (!CPL_IS_OK(r)) {
				free(path);
				return r;
			}

			if (out_version != NULL) {
				r = cpl_get_version(id, out_version);
				if (!CPL_IS_OK(r)) {
					free(path);
					return r;
				}
			}

			if (out_id != NULL) *out_id = id;

			free(path);
			if (ret == CPL_S_OK) ret = CPL_S_OBJECT_CREATED;
			return ret;
		}


		// Fail

		free(path);
		if (CPL_IS_OK(ret)) ret = CPL_E_NOT_FOUND;
		return ret;
	}


	// Open by path
	
	cpl_id_t id = CPL_NONE;
	if ((flags & CPL_F_CREATE_IF_DOES_NOT_EXIST) != 0) {
		ret = cpl_lookup_or_create_object(CPL_O_FILESYSTEM, path, CPL_T_FILE,
										  CPL_NONE, &id);
	}
	else {
		ret = cpl_lookup_object(CPL_O_FILESYSTEM, path, CPL_T_FILE, &id);
	}
	if (!CPL_IS_OK(ret)) {
		free(path);
		return ret;
	}

	if (ret == CPL_S_OBJECT_CREATED) {
		if (out_version != NULL) *out_version = 0;
	}
	else {
		if (out_version != NULL) {
			cpl_return_t r = cpl_get_version(id, out_version);
			if (!CPL_IS_OK(r)) {
				free(path);
				return r;
			}
		}
	}

	if (out_id != NULL) *out_id = id;

	free(path);
	return ret;
}

