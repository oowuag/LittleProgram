#include <hash_map>

class HashKey
{
public:
    HashKey(const std::string& key)
    {
        _key=key;
    }
    HashKey(const char* key)
    {
        _key=key;
    }

    // for boost and stdext
    size_t hash() const
    {
        // your own hash function here
        size_t h = 0;
        std::string::const_iterator p, p_end;
        for(p = _key.begin(), p_end = _key.end(); p != p_end; ++p)
        {
            h = 31 * h + (*p);
        }
        return h;
    }
    std::string _key;
};

// for stdext
namespace stdext
{
    template<>
    class hash_compare<HashKey>
    {
    public :
        static const size_t bucket_size = 4;
        static const size_t min_buckets = 8;

        size_t operator()(const HashKey &mc) const
        {
            return mc.hash();
        }

        bool operator()(const HashKey &mc1, const HashKey &mc2) const
        {
            return (strcmp(mc1._key.c_str(), mc2._key.c_str()) < 0);
        }
    };
}

int main(int argc, char* argv[])
{

    stdext::hash_map<HashKey, int> test;
    test["one"] = 1;
    test["two"] = 2;

    int a = test["one"];
    int b = test["two"];


    return 0;
}