#include <vector>

namespace SOE
{
	class RC4
	{
	private:
		int i = 0;
		int j = 0;
		std::vector<int> Table;

	public:
		RC4()
		{
			this->Table = std::vector<int>(256);
		}

		RC4(std::vector<unsigned char>& key)
		{
			this->Table = std::vector<int>(256);

			this->Init(key);
		}

		void Init(std::vector<unsigned char>& key)
		{
			int k = key.size();
			this->i = 0;
			while (this->i < 256)
			{
				this->Table[this->i] = this->i;
				this->i++;
			}

			this->i = 0;
			this->j = 0;
			while (this->i < 0x0100)
			{
				this->j = (((this->j + this->Table[this->i]) + key[(this->i % k)]) % 256);
				this->Swap(this->i, this->j);
				this->i++;
			}

			this->i = 0;
			this->j = 0;
		}

		void Swap(int a, int b)
		{
			int k = this->Table[a];
			this->Table[a] = this->Table[b];
			this->Table[b] = k;
		}

		std::vector<unsigned char> Parse(unsigned char* bytes, unsigned int size)
		{
			int k = 0;
			std::vector<unsigned char> result(size);
			int pos = 0;

			for (int a = 0; a < size; a++)
			{
				this->i = ((this->i + 1) % 256);
				this->j = ((this->j + this->Table[this->i]) % 256);
				this->Swap(this->i, this->j);
				k = ((this->Table[this->i] + this->Table[this->j]) % 256);
				result[pos++] = static_cast<unsigned char>(bytes[a] ^ this->Table[k]);
			}

			return result;
		}

	};
}