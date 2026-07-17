/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Texture loader interface
Written by Andrew Lucas
*/

#include "hud.h"
#include "textureloader.h"
#include "bsprenderer.h"
#include "propmanager.h"
#if defined(_WIN32)
#include "windows.h"
#endif
#include "gl/glext.h"

#include "FranUtils/FranUtils_String.hpp"
#include "FranUtils/FranUtils_FileSystem.hpp"

#pragma warning(disable : 4018)

/*
====================
Init

====================
*/
void CTextureLoader::Init()
{
	glCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)wglGetProcAddress("glCompressedTexImage2DARB");
}

/*
====================
VidInit

====================
*/
void CTextureLoader::VidInit()
{
	for (cl_texture_t& texture : m_dequeTextures)
	{
		// Only delete texture ids we actually own.
		if (texture.iIndex >= BASE_EXT_TEXTURE_ID)
			glDeleteTextures(1, &texture.iIndex);
	}

	m_dequeTextures.clear();
}

/*
====================
Shutdown

====================
*/
void CTextureLoader::Shutdown()
{
	VidInit();
}

/*
====================
IsPowerOfTwo

====================
*/
bool CTextureLoader::IsPowerOfTwo(int iWidth, int iHeight)
{
	int iWidthT = iWidth;
	while (iWidthT != 1)
	{
		if ((iWidthT % 2) != 0)
			return false;
		iWidthT /= 2;
	}

	int iHeightT = iHeight;
	while (iHeightT != 1)
	{
		if ((iHeightT % 2) != 0)
			return false;
		iHeightT /= 2;
	}
	return true;
}

/*
====================
LoadTexture

====================
*/
cl_texture_t* CTextureLoader::LoadTexture(const std::string& strFile, int iAltIndex, bool bPrompt, bool bNoMip, bool bBorder)
{
	if (strFile.size() < 4)
		return nullptr;

	// Try and find a match
	cl_texture_t* pTexture = nullptr;

	if (iAltIndex == 0)
		pTexture = HasTexture(strFile);

	if (pTexture != nullptr)
	{
		// Just return regular ones if already loaded
		return pTexture;
	}

	int iType = 0;
	byte* pFile = nullptr;

	// Some files need to be .tga
	if (strFile.ends_with("dds"))
		pFile = (byte*)gEngfuncs.COM_LoadFile(strFile.c_str(), 5, nullptr);

	if (pFile == nullptr)
	{
		// Check for .tga then
		std::string strAlt = strFile.substr(0, strFile.size() - 3) + "tga";

		pFile = (byte*)gEngfuncs.COM_LoadFile(strAlt.c_str(), 5, nullptr);
		iType = 1;
	}

	if (pFile == nullptr)
	{
		if (bPrompt)
			gEngfuncs.Con_Printf("Failed to load image: %s\n", strFile.c_str());
		else
			gEngfuncs.Con_DPrintf("Failed to load image: %s\n", strFile.c_str());
		return nullptr;
	}

	//
	// Allocate cache
	//
	pTexture = &m_dequeTextures.emplace_back();

	if (iAltIndex == 0)
	{
		pTexture->iIndex = current_ext_texture_id;
		current_ext_texture_id++;
	}
	else
	{
		pTexture->iIndex = iAltIndex;
	}

	pTexture->strName = strFile;

	// Load DDS file
	if (iType == 0)
	{
		if (!LoadDDSFile(pFile, pTexture, bNoMip))
		{
			gEngfuncs.Con_Printf("Error! Failed to load: %s.\n", strFile.c_str());
			gEngfuncs.COM_FreeFile(pFile);

			m_dequeTextures.pop_back();
			return nullptr;
		}
	}
	else if (iType == 1)
	{
		if (!LoadTGAFile(pFile, pTexture, bNoMip, bBorder))
		{
			gEngfuncs.Con_Printf("Error! Failed to load: %s.\n", strFile.c_str());
			gEngfuncs.COM_FreeFile(pFile);

			m_dequeTextures.pop_back();
			return nullptr;
		}
	}

	gEngfuncs.COM_FreeFile(pFile);
	return pTexture;
}

/*
====================
LoadTGAFile

====================
*/
bool CTextureLoader::LoadTGAFile(byte* pFile, cl_texture_t* pTexture, bool bNoMip, bool bBorder)
{
	// Set basic information
	tga_header_t* pHeader = (tga_header_t*)pFile;
	if (pHeader->datatypecode != 2 && pHeader->datatypecode != 10 || pHeader->bitsperpixel != 24 && pHeader->bitsperpixel != 32)
	{
		gEngfuncs.Con_Printf("Error! %s is using a non-supported format. Only 24 bit and 32 bit true color formats are supported.\n", pTexture->strName.c_str());
		return false;
	}

	pTexture->iWidth = ByteToUShort(pHeader->width);
	pTexture->iHeight = ByteToUShort(pHeader->height);

	if (!IsPowerOfTwo(pTexture->iWidth, pTexture->iHeight))
	{
		gEngfuncs.Con_Printf("Error! %s is not a power of two texture!\n", pTexture->strName.c_str());
		return false;
	}

	// Allocate data
	pTexture->iBpp = pHeader->bitsperpixel / 8;
	int iSize = pTexture->iWidth * pTexture->iHeight;
	int iImageSize = iSize * pTexture->iBpp;

	std::vector<byte> originalData(iImageSize, 0);
	byte* pOriginal = originalData.data();

	// Load based on type
	byte* pCurrent = pFile + 18;
	if (pHeader->datatypecode == 2)
	{
		// Uncompressed TGA
		if (pTexture->iBpp == 3)
		{
			for (int i = 0; i < iImageSize; i += 3)
			{
				pOriginal[i] = pCurrent[i + 2];
				pOriginal[i + 1] = pCurrent[i + 1];
				pOriginal[i + 2] = pCurrent[i];
			}
		}
		else if (pTexture->iBpp == 4)
		{
			for (int i = 0; i < iImageSize; i += 4)
			{
				pOriginal[i] = pCurrent[i + 2];
				pOriginal[i + 1] = pCurrent[i + 1];
				pOriginal[i + 2] = pCurrent[i];
				pOriginal[i + 3] = pCurrent[i + 3];
			}
		}
	}
	else
	{
		// RLE Compression
		int i = 0;
		if (pTexture->iBpp == 3)
		{
			while (i < iImageSize)
			{
				if ((*pCurrent & 0x80) != 0)
				{
					byte bLength = *pCurrent - 127;
					pCurrent++;

					for (int j = 0; j < bLength; j++, i += pTexture->iBpp)
					{
						pOriginal[i] = pCurrent[2];
						pOriginal[i + 1] = pCurrent[1];
						pOriginal[i + 2] = pCurrent[0];
					}

					pCurrent += pTexture->iBpp;
				}
				else
				{
					byte bLength = *pCurrent + 1;
					pCurrent++;

					for (int j = 0; j < bLength; j++, i += pTexture->iBpp, pCurrent += pTexture->iBpp)
					{
						pOriginal[i] = pCurrent[2];
						pOriginal[i + 1] = pCurrent[1];
						pOriginal[i + 2] = pCurrent[0];
					}
				}
			}
		}
		else
		{
			while (i < iImageSize)
			{
				if ((*pCurrent & 0x80) != 0)
				{
					byte bLength = *pCurrent - 127;
					pCurrent++;

					for (int j = 0; j < bLength; j++, i += pTexture->iBpp)
					{
						pOriginal[i] = pCurrent[2];
						pOriginal[i + 1] = pCurrent[1];
						pOriginal[i + 2] = pCurrent[0];
						pOriginal[i + 3] = pCurrent[3];
					}

					pCurrent += pTexture->iBpp;
				}
				else
				{
					byte bLength = *pCurrent + 1;
					pCurrent++;

					for (int j = 0; j < bLength; j++, i += pTexture->iBpp, pCurrent += pTexture->iBpp)
					{
						pOriginal[i] = pCurrent[2];
						pOriginal[i + 1] = pCurrent[1];
						pOriginal[i + 2] = pCurrent[0];
						pOriginal[i + 3] = pCurrent[3];
					}
				}
			}
		}
	}

	// Flip vertically
	std::vector<byte> flippedData(iImageSize);
	byte* pFlipped = flippedData.data();
	for (int i = 0; i < pTexture->iHeight; i++)
	{
		GLubyte* dst = pFlipped + i * pTexture->iWidth * pTexture->iBpp;
		GLubyte* src = pOriginal + (pTexture->iHeight - i - 1) * pTexture->iWidth * pTexture->iBpp;
		memcpy(dst, src, sizeof(GLubyte) * pTexture->iWidth * pTexture->iBpp);
	}

	// Add border if asked to
	if (bBorder)
	{
		byte* pCurrent = pFlipped;
		for (int i = 0; i < pTexture->iHeight; i++)
		{
			for (int j = 0; j < pTexture->iWidth; j++)
			{
				if (i == 0 || i == (pTexture->iHeight - 1) || j == 0 || j == (pTexture->iWidth - 1))
				{
					pCurrent[0] = 0;
					pCurrent[1] = 0;
					pCurrent[2] = 0;
				}

				if (pTexture->iBpp == 3)
					pCurrent += 3;
				else
					pCurrent += 4;
			}
		}
	}

	glBindTexture(GL_TEXTURE_2D, pTexture->iIndex);

	if (bNoMip)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (pTexture->iBpp == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, pTexture->iWidth, pTexture->iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pFlipped);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pTexture->iWidth, pTexture->iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pFlipped);

	return true;
}

/*
====================
LoadDDSFile

====================
*/
bool CTextureLoader::LoadDDSFile(byte* pFile, cl_texture_t* pTexture, bool bNoMip)
{
	dds_header_t* pHeader = (dds_header_t*)pFile;
	unsigned int iFlags = ByteToUInt(pHeader->bFlags);
	unsigned int iMagic = ByteToUInt(pHeader->bMagic);
	unsigned int iFourCC = ByteToUInt(pHeader->bPFFourCC);
	unsigned int iPFFlags = ByteToUInt(pHeader->bPFFlags);
	unsigned int iLinSize = ByteToUInt(pHeader->bPitchOrLinearSize);
	unsigned int iSize = ByteToUInt(pHeader->bSize);

	pTexture->iWidth = ByteToUInt(pHeader->bWidth);
	pTexture->iHeight = ByteToUInt(pHeader->bHeight);

	if (!IsPowerOfTwo(pTexture->iWidth, pTexture->iHeight))
	{
		gEngfuncs.Con_Printf("Error! %s is not a power of two texture!\n", pTexture->strName.c_str());
		return false;
	}

	if (iMagic != DDS_MAGIC)
		return false; // Not DDS file

	if (iSize != 124)
		return false; // Not correct size

	if ((iFlags & DDSD_PIXELFORMAT) == 0u)
		return false; // Not correct format

	if ((iFlags & DDSD_CAPS) == 0u)
		return false; // Not correct format

	if ((iPFFlags & DDPF_FOURCC) == 0u)
		return false; // Not correct type

	if (iFourCC != D3DFMT_DXT1 && iFourCC != D3DFMT_DXT5)
	{
		gEngfuncs.Con_Printf("Error! Incorrect compression on: %s! Only DXT1 and DXT5 are supported!\n", pTexture->strName.c_str());
		return false; // Not correct compression
	}

	// Copy data over
	std::vector<byte> textureData(pFile + 128, pFile + 128 + iLinSize);

	glBindTexture(GL_TEXTURE_2D, pTexture->iIndex);
	if (bNoMip)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Upload to OpenGL
	glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, (iFourCC == D3DFMT_DXT1) ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, pTexture->iWidth, pTexture->iHeight, 0, iLinSize, textureData.data());

	return true;
}

/*
====================
HasTexture

====================
*/
cl_texture_t* CTextureLoader::HasTexture(const std::string& strFile)
{
	for (cl_texture_t& texture : m_dequeTextures)
	{
		if (strFile == texture.strName)
			return &texture;
	}
	return nullptr;
}

/*
====================
LoadWADFiles

====================
*/
void CTextureLoader::LoadWADFiles()
{
	std::string wadString = gPropManager.ValueForKey(&gPropManager.m_pBSPEntities[0], "wad");
	if (wadString.empty())
		return;

	std::istringstream wadStream(wadString);
	std::string wadEntry;

	while (std::getline(wadStream, wadEntry, ';'))
	{
		std::string strFile = FilenameFromPath(wadEntry) + ".wad";

		int iSize = 0;
		byte* pFile = gEngfuncs.COM_LoadFile(strFile.c_str(), 5, &iSize);
		if (!pFile)
			continue;

		wadinfo_t* pInfo = reinterpret_cast<wadinfo_t*>(pFile);
		if (std::string_view(pInfo->identification, 4) != "WAD3")
		{
			gEngfuncs.COM_FreeFile(pFile);
			continue;
		}

		wadfile_t& wadFile = m_vectorWADFiles.emplace_back();
		wadFile.wadfile = pFile;
		wadFile.info = reinterpret_cast<wadinfo_t*>(wadFile.wadfile);

		const lumpinfo_t* pLumpTable = reinterpret_cast<const lumpinfo_t*>(wadFile.wadfile + wadFile.info->infotableofs);
		wadFile.lumps.assign(pLumpTable, pLumpTable + wadFile.info->numlumps);
	}
}

/*
====================
FreeWADFiles

====================
*/
void CTextureLoader::FreeWADFiles()
{
	for (wadfile_t& wadFile : m_vectorWADFiles)
		gEngfuncs.COM_FreeFile(wadFile.wadfile);

	m_vectorWADFiles.clear();
}

/*
====================
LoadWADTexture

====================
*/
cl_texture_t* CTextureLoader::LoadWADTexture(const std::string& strTexture, int iAltIndex)
{
	std::string strName;

	for (wadfile_t& wadFile : m_vectorWADFiles)
	{
		byte* pFile = wadFile.wadfile;
		for (lumpinfo_t& lump : wadFile.lumps)
		{
			if (lump.type != 0 && ((lump.type & 0x43) == 0))
				continue;

			strName = std::string(lump.name);
			FranUtils::StringUtils::LowerCase_Ref(strName);

			if (strName == strTexture)
			{
				cl_texture_t* pTexture = &m_dequeTextures.emplace_back();

				// Fill in data
				pTexture->strName = strTexture;
				pTexture->iWidth = ByteToUInt(pFile + lump.filepos + 16);
				pTexture->iHeight = ByteToUInt(pFile + lump.filepos + 20);
				pTexture->iBpp = 4;

				// Get offsets
				int iIndexOffset = ByteToUInt(pFile + lump.filepos + 24);
				int iMip3Offset = ByteToUInt(pFile + lump.filepos + 36);

				byte* pPalette;
				if ((lump.type & 0x43) != 0)
					pPalette = pFile + lump.filepos + iMip3Offset + ((pTexture->iWidth / 8) * (pTexture->iHeight / 8)) + 2;
				else
					pPalette = pFile + lump.filepos + iIndexOffset + (pTexture->iWidth * pTexture->iHeight) + 2;

				if (iAltIndex != 0)
					pTexture->iIndex = iAltIndex;
				byte* pPixels = pFile + lump.filepos + iIndexOffset;
				LoadPallettedTexture(pPixels, pPalette, pTexture);
				return pTexture;
			}
		}
	}

	return nullptr;
}


/*
====================
LoadPallettedTexture

====================
*/
void CTextureLoader::LoadPallettedTexture(byte* data, byte* pal, cl_texture_t* pTexture)
{
	int row1[1024], row2[1024], col1[1024], col2[1024];
	byte *pix1, *pix2, *pix3, *pix4;
	byte alpha1, alpha2, alpha3, alpha4;

	// convert texture to power of 2
	int outwidth;
	for (outwidth = 1; outwidth < pTexture->iWidth; outwidth <<= 1)
		;
	if (outwidth > 1024)
		outwidth = 1024;

	int outheight;
	for (outheight = 1; outheight < pTexture->iHeight; outheight <<= 1)
		;
	if (outheight > 1024)
		outheight = 1024;

	std::vector<byte> outData(outwidth * outheight * 4);
	byte* out = outData.data();

	for (int i = 0; i < outwidth; i++)
	{
		col1[i] = (int)((i + 0.25) * (pTexture->iWidth / (float)outwidth));
		col2[i] = (int)((i + 0.75) * (pTexture->iWidth / (float)outwidth));
	}

	for (int i = 0; i < outheight; i++)
	{
		row1[i] = (int)((i + 0.25) * (pTexture->iHeight / (float)outheight)) * pTexture->iWidth;
		row2[i] = (int)((i + 0.75) * (pTexture->iHeight / (float)outheight)) * pTexture->iWidth;
	}

	for (int i = 0; i < outheight; i++)
	{
		for (int j = 0; j < outwidth; j++, out += 4)
		{
			pix1 = &pal[data[row1[i] + col1[j]] * 3];
			pix2 = &pal[data[row1[i] + col2[j]] * 3];
			pix3 = &pal[data[row2[i] + col1[j]] * 3];
			pix4 = &pal[data[row2[i] + col2[j]] * 3];
			alpha1 = 0xFF;
			alpha2 = 0xFF;
			alpha3 = 0xFF;
			alpha4 = 0xFF;

			if (pTexture->strName[0] == '{')
			{
				if (data[row1[i] + col1[j]] == 0xFF)
				{
					pix1[0] = 0;
					pix1[1] = 0;
					pix1[2] = 0;
					alpha1 = 0;
				}

				if (data[row1[i] + col2[j]] == 0xFF)
				{
					pix2[0] = 0;
					pix2[1] = 0;
					pix2[2] = 0;
					alpha2 = 0;
				}

				if (data[row2[i] + col1[j]] == 0xFF)
				{
					pix3[0] = 0;
					pix3[1] = 0;
					pix3[2] = 0;
					alpha3 = 0;
				}

				if (data[row2[i] + col2[j]] == 0xFF)
				{
					pix4[0] = 0;
					pix4[1] = 0;
					pix4[2] = 0;
					alpha4 = 0;
				}
			}

			out[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0]) >> 2;
			out[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1]) >> 2;
			out[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2]) >> 2;
			out[3] = (alpha1 + alpha2 + alpha3 + alpha4) >> 2;
		}
	}

	if (pTexture->iIndex == 0u)
	{
		pTexture->iIndex = current_ext_texture_id;
		current_ext_texture_id++;
	}

	glBindTexture(GL_TEXTURE_2D, pTexture->iIndex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, outwidth, outheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, outData.data());
}

/*
====================
LoadTextureScript

====================
*/
void CTextureLoader::LoadTextureScript()
{
	m_vectorTextureEntries.clear();

	std::ifstream inputStream{};
	std::stringstream strStream{};
	FranUtils::FileSystem::OpenInputFile("gfx/textures/texture_flags.txt", inputStream);

	if (!inputStream)
	{
		gEngfuncs.Con_Printf("Could not load gfx/textures/texture_flags.txt!\n");
		return;
	}

	strStream << inputStream.rdbuf();
	inputStream.close();

	std::istringstream fileStream(strStream.str());
	std::string strModel;
	std::string strTexture;
	std::string strFlag;
	int iFlags = 0;

	while (fileStream >> strModel >> strTexture)
	{
		iFlags = 0;

		FranUtils::StringUtils::LowerCase_Ref(strModel);
		FranUtils::StringUtils::LowerCase_Ref(strTexture);

		while (fileStream >> strFlag)
		{
			FranUtils::StringUtils::LowerCase_Ref(strFlag);

			if (strFlag == "alternate")
				iFlags |= TEXFLAG_ALTERNATE;
			else if (strFlag == "fullbright")
				iFlags |= TEXFLAG_FULLBRIGHT;
			else if (strFlag == "none")
				iFlags |= TEXFLAG_NONE;
			else if (strFlag == "nomipmap")
				iFlags |= TEXFLAG_NOMIPMAP;
			else if (strFlag == "eraseflags")
				iFlags |= TEXFLAG_ERASE;
			else
				break; // Invalid flag
		}

		if (iFlags != 0)
		{
			texentry_t& entry = m_vectorTextureEntries.emplace_back();
			entry.strModel = strModel;
			entry.strTexture = strTexture;
			entry.iFlags = iFlags;
		}
	}
}

/*
====================
TextureHasFlag

====================
*/
bool CTextureLoader::TextureHasFlag(const std::string& model, const std::string& texture, int flag) const
{
	for (const texentry_t& entry : m_vectorTextureEntries)
	{
		if ((entry.strModel == model) && (entry.strTexture == texture) && ((entry.iFlags & flag) != 0))
		{
			return true;
		}
	}

	return false;
}

/*
====================
WriteTGA

====================
*/
void CTextureLoader::WriteTGA(byte* pixels, int bpp, int width, int height, const std::string& strPath)
{
	int iSize = width * height * bpp;
	std::vector<byte> fileData(iSize + 18, 0);
	byte* pBuf = fileData.data();

	tga_header_t* pHeader = (tga_header_t*)pBuf;
	pHeader->datatypecode = 2;
	pHeader->bitsperpixel = bpp * 8;
	pHeader->width[0] = (width & 0xFF);
	pHeader->width[1] = ((width >> 8) & 0xFF);
	pHeader->height[0] = (height & 0xFF);
	pHeader->height[1] = ((height >> 8) & 0xFF);

	for (int i = 0; i < height; i++)
	{
		GLubyte* dst = pBuf + 18 + i * width * bpp;
		GLubyte* src = pixels + (height - i - 1) * width * bpp;
		memcpy(dst, src, sizeof(byte) * width * bpp);

		if (bpp == 4)
		{
			for (int j = 0; j < width * bpp; j += bpp)
			{
				dst[j] = src[j + 2];
				dst[j + 1] = src[j + 1];
				dst[j + 2] = src[j];
				dst[j + 3] = src[j + 3];
			}
		}
		else
		{
			for (int j = 0; j < width * bpp; j += bpp)
			{
				dst[j] = src[j + 2];
				dst[j + 1] = src[j + 1];
				dst[j + 2] = src[j];
			}
		}
	}

	std::string strFullPath = std::string(gEngfuncs.pfnGetGameDirectory()) + "/imagedump/" + strPath + ".tga";

	std::ofstream outputStream(strFullPath, std::ios::binary);
	if (!outputStream)
		return;

	outputStream.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());
}
