#include <library/storage/bitarray.hpp>

#include <library/log.hpp>
#include <cstring>

namespace library
{
	BitArray::BitArray(int elements)
	{
		size = segsize(elements);
		bits = std::unique_ptr<bitarray_t[]> (new bitarray_t[size]);
	}
	
	BitArray::BitArray(const BitArray& ba)
	{
		size = ba.size;
		bits = std::unique_ptr<bitarray_t[]> (new bitarray_t[this->size]);
		if (size)
		{
			// copy over, there are elements
			memcpy(bits.get(), ba.bits.get(), sizeof(bitarray_t) * size);
		}
	}
	
	BitArray& BitArray::operator= (const BitArray& ba)
	{
		size = ba.size;
		bits = std::unique_ptr<bitarray_t[]> (new bitarray_t[this->size]);
		if (size)
		{
			memcpy(bits.get(), ba.bits.get(), sizeof(bitarray_t) * size);
		}
		return *this;
	}
	
	BitArray::bitarray_t BitArray::segsize(int size) const
	{
		return (size >> SEG_SHR) + 1;
	}
	
	void BitArray::clear()
	{
		memset(bits.get(), 0, sizeof(bitarray_t) * size);
	}
	
}
