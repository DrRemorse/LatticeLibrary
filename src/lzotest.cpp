#ifdef LZO_TEST
#include <library/compression/lzo.hpp>
#include <iostream>
#include <string>
#include <cassert>
#include <cstring>

using namespace library;

LZO lzotest;

void test_compression()
{
	const int CLEN = 10000;
	
	// our data (don't care to initialize it)
	lzo_bytep cfrom = new lzo_byte[CLEN];
	
	// initialize compression buffer with worst-case (exact copy)
	lzotest.init(CLEN + 100);
	
	if (lzotest.compressHard(cfrom, CLEN * sizeof(lzo_byte)))
	{
		std::cout << "Compressed " << CLEN << " bytes to " << lzotest.getDataLength() << std::endl;
		std::cout << "Ratio: " << (int)((float)CLEN / (float)lzotest.getDataLength()) << ":1" << std::endl;
		std::cout << std::endl;
	}
	else
	{
		throw std::string("Failed to compress data using lzo2_1x_999");
	}
	
	// validate results
	lzo_bytep cto = lzotest.getData();
	assert(cto != nullptr);
	
	// copy compressed data to new memory
	const int DLEN = lzotest.getDataLength();
	lzo_bytep decomp = new lzo_byte[DLEN];
	memcpy(decomp, lzotest.getData(), DLEN * sizeof(lzo_byte));
	
	// decompressing using the compressed datalength
	lzotest.decompress(decomp, DLEN);
	
	// validate decompressed data
	for (int i = 0; i < CLEN; i++)
		assert(lzotest.getData()[i] == cfrom[i]);
	
	// cleanup
	delete[] cfrom;
	delete[] decomp;
}
#endif
