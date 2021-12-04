#include <vector>

namespace SOE
{
	namespace RC4
	{
	    static bool Initiated = false;
		static int i = 0;
		static int j = 0;
		static std::vector<int> Table;
		static std::vector<unsigned char> key = {
							0x17, 0xbd, 0x08, 0x6b, 0x1b, 0x94, 0xf0, 0x2f, 0xf0, 0xec, 0x53, 0xd7, 0x63, 0x58, 0x9b,
							0x5f
		};

		static void Swap(int a, int b)
		{
			int k = Table[a];
			Table[a] = Table[b];
			Table[b] = k;
		}

        static void Init(std::vector<unsigned char>& key)
		{
			int k = key.size();
			i = 0;
			while (i < 256)
			{
				Table[i] = i;
				i++;
			}

			i = 0;
			j = 0;
			while (i < 0x0100)
			{
				j = (((j + Table[i]) + key[(i % k)]) % 256);
				Swap(i, j);
				i++;
			}

			i = 0;
			j = 0;
		}

		static std::vector<unsigned char> Parse(unsigned char* bytes, unsigned int size)
		{
		    if (!Initiated) {
		        Init(key);
		        Initiated = true;
		    }
		     
			int k = 0;
			std::vector<unsigned char> result(size);
			int pos = 0;

			for (int a = 0; a < size; a++)
			{
				i = ((i + 1) % 256);
				j = ((j + Table[i]) % 256);
				Swap(i, j);
				k = ((Table[i] + Table[j]) % 256);
				result[pos++] = static_cast<unsigned char>(bytes[a] ^ Table[k]);
			}

			return result;
		}

	};
}