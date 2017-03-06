#ifndef _MORTON_CODE_H_
#define _MORTON_CODE_H_

namespace Nvidia {

/** 
 * Count the number of consecutive leading zero bits, starting at the most significant bit (bit 31) of x.
 * 
 * Returns a value between 0 and 32 inclusive representing the number of zero bits.
 *
 * (Copied from NVIDIA's website.)
 */
static int __clz(unsigned int v) {
    if (v == 0) return 32;

    unsigned int mask = 0x80000000u;
    int count = 0;
    while ((v & mask) == 0) {
        count++;
        mask >>= 1;
    }
    return count;
}

static unsigned int expandBits(unsigned int v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

static unsigned int morton3D(float x, float y, float z)
{
	x = std::min(std::max(x * 1024.0f, 0.0f), 1023.0f);
	y = std::min(std::max(y * 1024.0f, 0.0f), 1023.0f);
	z = std::min(std::max(z * 1024.0f, 0.0f), 1023.0f);
    unsigned int xx = expandBits((unsigned int)x);
    unsigned int yy = expandBits((unsigned int)y);
    unsigned int zz = expandBits((unsigned int)z);
    return xx * 4 + yy * 2 + zz;
}

static int findSplit(unsigned int* sortedMortonCodes,
                     int           first,
                     int           last)
{
    // Identical Morton codes => split the range in the middle.

    unsigned int firstCode = sortedMortonCodes[first];
    unsigned int lastCode = sortedMortonCodes[last];

    if (firstCode == lastCode)
        return (first + last) >> 1;

    // Calculate the number of highest bits that are the same
    // for all objects, using the count-leading-zeros intrinsic.

    int commonPrefix = __clz(firstCode ^ lastCode);

    // Use binary search to find where the next bit differs.
    // Specifically, we are looking for the highest object that
    // shares more than commonPrefix bits with the first one.

    int split = first; // initial guess
    int step = last - first;

    do
    {
        step = (step + 1) >> 1; // exponential decrease
        int newSplit = split + step; // proposed new position

        if (newSplit < last)
        {
            unsigned int splitCode = sortedMortonCodes[newSplit];
            int splitPrefix = __clz(firstCode ^ splitCode);
            if (splitPrefix > commonPrefix)
                split = newSplit; // accept proposal
        }
    }
    while (step > 1);

    return split;
}

struct MortonEntry {
    unsigned int code;
    int index;

    MortonEntry() {}
    MortonEntry(unsigned int code, int index) : code(code), index(index) {}
};

static int morton_compare(const void * a, const void * b)
{
    return ( (int)(((MortonEntry*)a)->code) - (int)(((MortonEntry*)b)->code) );
}

static void sort_morton_array(unsigned int *morton, int n, MortonEntry *&entries) {
    entries = new MortonEntry[n];
    for (int i = 0; i < n; ++i) {
        entries[i] = MortonEntry(morton[i], i);
    }
    std::qsort(entries, n, sizeof(MortonEntry), morton_compare);
}

} // end namespace Nvidia

#endif
