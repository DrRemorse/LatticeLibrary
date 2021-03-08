#include <library/bitmap/bitmap.hpp>

#include <library/log.hpp>
#include <library/bitmap/colortools.hpp>
#include "lodepng.h"
#include <fstream>
#include <memory>
#include <cassert>

#define GL_RGB  0x1907
#define GL_RGBA 0x1908
#define GL_BGRA 0x80E1

namespace library
{
	Bitmap::Bitmap(int w, int h):
		buffer(w * h), width(w), height(h), format(GL_BGRA) {}

	Bitmap::Bitmap(int w, int h, rgba8_t color):
		buffer(w * h, color), width(w), height(h), format(GL_BGRA) {}

	Bitmap::Bitmap(int w, int h, const rgba8_t* data, int glformat):
    buffer(data, data + w * h * sizeof(rgba8_t)),
		width(w), height(h), format(glformat) {}

  Bitmap::Bitmap(const std::string file, Bitmap::bitmap_type btype)
	{
    switch (btype) {
		case BMP:
			loadBMP(file);
      break;
		case PNG:
			loadPNG(file);
      break;
    default:
      throw std::string("Bitmap::load(): Unsupported format");
		}
	}

	// assignment operator
	Bitmap& Bitmap::operator= (const Bitmap& bmp)
	{
    buffer = bmp.buffer;
    this->width  = bmp.width;
		this->height = bmp.height;
		this->format = bmp.format;
		// extra info (in case the bitmap was linearized)
		this->tilesX = bmp.tilesX;
		this->tilesY = bmp.tilesY;
		return *this;
	}

	void Bitmap::loadBMP(const std::string& file)
	{
		#pragma pack(push, 1)
		// packed: make sure bytes are packed tight, 1-byte memory alignment
		struct bmpheader_t
		{
			char magic[2];
			int filesize;
			short res1;
			short res2;
			int pixeldata;
		};

		struct bmpdata_t
		{
			int headersize;
			int w;
			int h;
			short planes;
			short bits;
			// don't care about the rest
		};
		#pragma pack(pop)

		bmpheader_t bhead;
		bmpdata_t bdata;
		std::ifstream File (file.c_str(), std::ios::in | std::ios::binary);

		if (!File)
		{	// Log: could not open file
			logger << Log::ERR << "Could not open file: " << file << Log::ENDL;
      throw std::runtime_error("Missing bitmap file: " + file);
		}

		File.read ( (char*) &bhead, sizeof(bmpheader_t) );
		File.read ( (char*) &bdata, sizeof(bmpdata_t) );

		this->width  = bdata.w;
		this->height = bdata.h;
		this->format = GL_BGRA;
		this->tilesX = 1;
		this->tilesY = 1;

		int pixels = width * height;

		// allocate 32-bits * w * h pixels
    buffer.resize(pixels);

		// seek to pixeldata relative to beginning of file
		File.seekg(bhead.pixeldata);

		if (bdata.bits == 24)
		{
			// width of image on file in bytes
			int pad = (width * 3) & 3; // 4-byte padding
			if (pad) pad = 4 - pad;
			// width of image in memory in bytes
			const int SCANLINE = width * 3 + pad; // pitch

			auto scanbuffer = std::unique_ptr<uint8_t>(new uint8_t[SCANLINE]);

			for (int y = 0; y < height; y++)
			{
				auto* buf = (uint8_t*) buffer.data() + (height-1 - y) * width * sizeof(rgba8_t);
				auto* tmp = scanbuffer.get();
				// read entire scanline
				File.read((char*) tmp, SCANLINE);

				for (int x = 0; x < width; x++)
				{
					buf[0] = tmp[0];
					buf[1] = tmp[1];
					buf[2] = tmp[2];
					buf[3] = 255;
					buf += 4; tmp += 3;
				}

			}
		}
		else if (bdata.bits == 32)
		{
			// 32-bits (8 per channel)
			auto* scanpos = this->buffer.data() + height * width;

			while (scanpos > this->buffer.data())
			{
				scanpos -= width;
				// read entire scanline
				File.read( (char*) scanpos, width * sizeof(rgba8_t) );
			}
		}
		else
		{
			logger << Log::ERR << "Bitmap::loadBMP(): Invalid bits per pixel: " << bdata.bits << Log::ENDL;
			//throw std::string("Bitmap::loadBMP(): Invalid bits value (" + file + ")");
		}
		File.close();
	}

	void Bitmap::loadPNG(const std::string& file)
	{
		std::vector<unsigned char> image, png;
		// load file from disk
		if (lodepng::load_file(png, file.c_str()) == false)
		{
			logger << Log::ERR << "Bitmap::loadPNG(): File not found: " << file << Log::ENDL;
      throw std::runtime_error("Missing image file: " + file);
		}
		// decode it
		unsigned w, h;
		// throw if there are any decoding errors
    auto error = lodepng::decode(image, w, h, png);
		if (error)
		{
			logger << Log::ERR << "Bitmap::loadPNG(): Error " << error << ": " << lodepng_error_text(error) << Log::ENDL;
			throw std::runtime_error("Image decode error for file: " + file);
		}
		this->format = GL_RGBA;
		this->width  = w;
		this->height = h;

		this->buffer.resize(w * h);
		memcpy (buffer.data(), image.data(), sizeof(rgba8_t) * w * h);
	}

	void Bitmap::blit(
        Bitmap& dest,
        const int srcX,  const int srcY,
        const int width, const int height,
        const int dstX,  const int dstY) const
	{
		// copy from this at (x, y) with size (w, h) to destination at (dstX, dstY)
		for (int y = 0; y < height; y++)
		{
			auto* src = this->buffer.data() + (y + srcY) * this->width + srcX;
			auto* dst = dest.buffer.data()  + (y + dstY) * dest.getWidth() + dstX;
      std::copy(src, src + width, dst);
			//memcpy (dst, src, width * sizeof(rgba8_t));
		}
		dest.format = this->format;
	}

	// splits a bitmap in a 1d continous memory array
	// of tiles used by GL_TEXTURE_2D_ARRAY (GL 3.x)
	void Bitmap::parse2D(const int tw, const int th, bool invert_y)
	{
		// nearest tilew/h multiple floor
		const int maxx = int(this->width  / tw) * tw;
		const int maxy = int(this->height / th) * th;

		// buffers
    std::vector<rgba8_t> newBuffer(maxx * maxy);
    auto* n = newBuffer.data();

		// for each tile
		for (int y = th-1; y < maxy; y += th)
		for (int x = 0; x < maxx; x += tw)
		{
			auto* p = this->buffer.data() + y * this->width + x;

			// blit internal tile
			for (int ty = 0; ty < th; ty++)
			{
				for (int tx = 0; tx < tw; tx++)
					*n++ = *p++;

				p -= this->width + tw;
			} // ty

		} // (x, y)

		// replace with new buffer
    this->buffer = newBuffer;
		// use members for texture_2d_array data
		this->tilesX = maxx / tw;
		this->tilesY = maxy / th;
		this->width  = tw;
		this->height = th;
	}

	// clear a bitmap with a given color
	void Bitmap::clear(rgba8_t color)
	{
    for (auto& value : buffer) value = color;
	}

	// replace a color on the bitmap with another
	void Bitmap::replace(rgba8_t color, rgba8_t replacecolor)
	{
    for (auto& value : buffer) {
        if (value == color) value = replacecolor;
    }
	}

	// rotate 90 degrees
	Bitmap Bitmap::rotate90() const
	{
		Bitmap bmp(this->height, this->width);
		for (int y = 0; y < this->height; y++)
		for (int x = 0; x < this->width; x++)
		{
			bmp.setPixel(this->height-1 - y, x, getPixel(x, y));
		}
		return bmp;
	}
	// flip x around itself
	Bitmap Bitmap::flipX() const
	{
		Bitmap bmp(this->width, this->height);
		for (int y = 0; y < this->height; y++)
		for (int x = 0; x < this->width;  x++)
		{
			bmp.setPixel(this->width-1 - x, y, getPixel(x, y));
		}
		return bmp;
	}
	// flip y around itself
	Bitmap Bitmap::flipY() const
	{
		Bitmap bmp(this->width, this->height);
		for (int y = 0; y < this->height; y++)
		for (int x = 0; x < this->width;  x++)
		{
			bmp.setPixel(x, this->height-1 - y, getPixel(x, y));
		}
		return bmp;
	}

	bool Bitmap::isValid() const
	{
		return this->width > 0 && this->height > 0;
	}

  void Bitmap::convert_to_tilesheet(const int tile_size, const uint32_t color)
  {
    this->width  = tile_size;
    this->height = tile_size;
    // initialize buffer to a single colored tile
    buffer = std::vector<rgba8_t> (tile_size * tile_size, color);
    this->format = GL_RGBA;
    this->tilesX = 1;
    this->tilesY = 1;
  }
  void Bitmap::add_tile(const Bitmap& other, int tx, int ty)
  {
    // we have to assume same tile size
    const int srcX = tx * this->getWidth();
    const int srcY = ty * this->getHeight();
    assert(srcX + this->getWidth() <= other.getWidth());
    assert(srcY + this->getHeight() <= other.getHeight());
    // add single tile
    const size_t end = buffer.size();
    this->tilesX ++;
    buffer.resize(getTileCount() * getWidth() * getHeight());

    auto* scan = this->buffer.data() + end;

    for (int y = this->getHeight()-1; y >= 0; y--)
    {
      auto* src = other.buffer.data() + (y + srcY) * other.width + srcX;
      std::copy(src, src + this->getWidth(), scan);
      scan += this->getWidth();
    }
  }
}
