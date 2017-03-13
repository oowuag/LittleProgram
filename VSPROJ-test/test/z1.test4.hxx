#include <stdio.h>
#include <string>
#include <map>
using namespace std;


struct Pinyin2Zhuyin
{
    string pinyin;
    string zhuyin;
};

	// map of Pinyin to Zhuyin
static Pinyin2Zhuyin pinyin2zhuyin_array[] = {
		// Consonant
		{"b", "ㄅ"}, {"p", "ㄆ"}, {"m", "ㄇ"}, {"f", "ㄈ"},
		{"d", "ㄉ"}, {"t", "ㄊ"}, {"n", "ㄋ"}, {"l", "ㄌ"},
		{"g", "ㄍ"}, {"k", "ㄎ"}, {"h", "ㄏ"},
        {"j", "ㄐ"}, {"q", "ㄑ"}, {"x", "ㄒ"},
		{"zh", "ㄓ"}, {"ch", "ㄔ"}, {"sh", "ㄕ"}, {"r", "ㄖ"},
        {"z", "ㄗ"}, {"c", "ㄘ"}, {"s", "ㄙ"},
        {"y", "一"}, {"w", "ㄨ"},
		// Rhymes
		{"a", "ㄚ"}, {"o", "ㄛ"}, {"e", "ㄜ"},
		{"i", "一"}, {"u", "ㄨ"}, {"v", "ㄩ"},
		{"ai", "ㄞ"}, {"ei", "ㄟ"}, {"ui", "ㄨㄟ"},
		{"ao", "ㄠ"}, {"ou", "ㄡ"}, {"iu", "一ㄡ"},
		{"an", "ㄢ"}, {"en", "ㄣ"}, {"in", "一ㄣ"},
		{"ang", "ㄤ"}, {"eng", "ㄥ"}, {"ing", "一ㄥ"},
		{"ong", "ㄨㄥ"}, {"ie", "一ㄝ"}, {"er", "ㄦ"},
		{"ue", "ㄩㄝ"}, {"ve", "ㄩㄝ"}, // 'ue' is same as 've', for typo
		{"un", "ㄨㄣ"}, {"vn", "ㄩㄣ"}, {"ia", "一ㄚ"},
		{"ua", "ㄨㄚ"}, {"uan", "ㄨㄢ"}, {"van", "ㄩㄢ"},
		{"uai", "ㄨㄞ"}, {"uo", "ㄨㄛ"}, {"iong", "ㄩㄥ"},
		{"iang", "一ㄤ"}, {"uang", "ㄨㄤ"}, {"ian", "一ㄢ"},
		{"iao", "一ㄠ"}
	};

static string zhuyinTones[] = {"˙", "", "ˊ", "ˇ", "ˋ"};

static std::map<string, string> map_p2z;

void split(const string& s, string& consonant, string& rhymes, unsigned char& tone);
string encodeZhuyin(string& consonant, string& rhymes,  unsigned char& tone);

// encodeZhuyin encode the input string into Zhuyin
// for example: encodeZhuyin("min2") outputs 'ㄇ一ㄣˊ'
// return an empty string in case an error
string EncodeZhuyin(const string& s)
{
    string consonant;
    string rhymes;
    unsigned char tone = 0;
    split(s, consonant, rhymes, tone);

	string out = encodeZhuyin(consonant, rhymes, tone);
    return out;
}


bool isRhymes(char b)
{
	return b == 'a' || b == 'e' || b == 'i' || b == 'o' || b == 'u' || b == 'v';
}

bool isConsonant(char b)
{
	return b >= 'a' && b <= 'z' && !isRhymes(b);
}

// split the input string into consonant, rhymes and tone
// for example: 'zhang1' will be split to consonant 'zh', rhymes 'ang' and
//              tone '1'
// returns an empty rhymes in case an error
void split(const string& s, string& consonant, string& rhymes, unsigned char& tone)
{	
    size_t i = 0;
	for (; i < s.length(); i++) {
		char c = s[i];
		if (!isConsonant(c)) {
			break;
		}
	}
    consonant = s.substr(0, i);

	for (; i < s.length(); i++) {
		char c = s[i];
		if (c < 'a' || c > 'z') {
			break;
		}
	}
    rhymes = s.substr(consonant.length(), i);

	// rhymes could not be empty, and the length of tone is at most 1
	if (rhymes.length() == 0 || (s.length() - i) > 2) {
        consonant = "";
        rhymes = "";
        tone = 0;
		return;
	}

	if (i < s.length()) {
		tone = s[i] - '0';
		if (tone < 0 || tone > 4) {
            consonant = "";
            rhymes = "";
            tone = 0;
		    return;
		}
	}
}


// encodePinyin encode the input consonant, rhymes and tone into Zhuyin
// for example: encodePinyin("m", "in", 2) outputs 'ㄇ一ㄣˊ'
// return an empty string in case an error
string encodeZhuyin(string& consonant, string& rhymes,  unsigned char& tone)
{
	if (rhymes.length() == 0) {
		return "";
	}

	if (rhymes[0] == 'u' && consonant.length() > 0) {
		char c = consonant[0];
		if (c == 'j' || c == 'q' || c == 'x' || c == 'y') {
			rhymes = "v" + rhymes.substr(1);
		}
	}

	// the special cases for 'Zheng3 Ti3 Ren4 Du2'
	if (rhymes == "i") {
		if (consonant == "zh" || consonant == "ch" || consonant == "sh" ||
			consonant == "r" || consonant == "z" || consonant == "c" ||
			consonant == "s" || consonant == "y") {
			rhymes = "";
		}
	} else if (consonant == "w") {
		if (rhymes == "u") {
			consonant = "";
		}
	} else if (consonant == "y") {
		if (rhymes == "v" || rhymes == "e" || rhymes == "ve" || rhymes == "in" ||
			rhymes == "van" || rhymes == "ing" || rhymes == "vn") {
			consonant = "";
		}
    } else {
    }

	// consonant must be valid
	if (consonant.length() > 0) {
        consonant = map_p2z[consonant];
	}

	// rhymes must be valid
	if (rhymes.length() > 0) {
        rhymes = map_p2z[rhymes];
	}

	return consonant + rhymes; // + zhuyinTones[tone];
}


int main()
{
    int array_len = sizeof(pinyin2zhuyin_array) / sizeof(Pinyin2Zhuyin);
    for (int i = 0; i < array_len; i++) {
        map_p2z[pinyin2zhuyin_array[i].pinyin] = pinyin2zhuyin_array[i].zhuyin;
    }

    string inStr = "han";
    printf("%s:%s\n", inStr.c_str(), EncodeZhuyin(inStr).c_str());

    return 0;
}