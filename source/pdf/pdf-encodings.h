#define _notdef NULL

const unsigned short pdf_doc_encoding[256] =
{
	/* 0x0 to 0x17 except \t, \n and \r are really undefined */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x02d8, 0x02c7, 0x02c6, 0x02d9, 0x02dd, 0x02db, 0x02da, 0x02dc,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x0000,
	0x2022, 0x2020, 0x2021, 0x2026, 0x2014, 0x2013, 0x0192, 0x2044,
	0x2039, 0x203a, 0x2212, 0x2030, 0x201e, 0x201c, 0x201d, 0x2018,
	0x2019, 0x201a, 0x2122, 0xfb01, 0xfb02, 0x0141, 0x0152, 0x0160,
	0x0178, 0x017d, 0x0131, 0x0142, 0x0153, 0x0161, 0x017e, 0x0000,
	0x20ac, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
	0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x0000, 0x00ae, 0x00af,
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
	0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
	0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
	0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
	0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
	0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
	0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};

const char *pdf_standard[256] = {
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"space", "exclam", "quotedbl", "numbersign", "dollar", "percent",
	"ampersand", "quoteright", "parenleft", "parenright", "asterisk",
	"plus", "comma", "hyphen", "period", "slash", "zero", "one", "two",
	"three", "four", "five", "six", "seven", "eight", "nine", "colon",
	"semicolon", "less", "equal", "greater", "question", "at", "A", "B",
	"C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
	"Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft",
	"backslash", "bracketright", "asciicircum", "underscore", "quoteleft",
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n",
	"o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
	"braceleft", "bar", "braceright", "asciitilde", _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"exclamdown", "cent", "sterling", "fraction", "yen", "florin",
	"section", "currency", "quotesingle", "quotedblleft", "guillemotleft",
	"guilsinglleft", "guilsinglright", "fi", "fl", _notdef, "endash",
	"dagger", "daggerdbl", "periodcentered", _notdef, "paragraph",
	"bullet", "quotesinglbase", "quotedblbase", "quotedblright",
	"guillemotright", "ellipsis", "perthousand", _notdef, "questiondown",
	_notdef, "grave", "acute", "circumflex", "tilde", "macron", "breve",
	"dotaccent", "dieresis", _notdef, "ring", "cedilla", _notdef,
	"hungarumlaut", "ogonek", "caron", "emdash", _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, "AE", _notdef,
	"ordfeminine", _notdef, _notdef, _notdef, _notdef, "Lslash", "Oslash",
	"OE", "ordmasculine", _notdef, _notdef, _notdef, _notdef, _notdef,
	"ae", _notdef, _notdef, _notdef, "dotlessi", _notdef, _notdef,
	"lslash", "oslash", "oe", "germandbls", _notdef, _notdef, _notdef,
	_notdef
};

const char *pdf_mac_roman[256] = {
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"space", "exclam", "quotedbl", "numbersign", "dollar", "percent",
	"ampersand", "quotesingle", "parenleft", "parenright", "asterisk",
	"plus", "comma", "hyphen", "period", "slash", "zero", "one", "two",
	"three", "four", "five", "six", "seven", "eight", "nine", "colon",
	"semicolon", "less", "equal", "greater", "question", "at", "A", "B",
	"C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
	"Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft",
	"backslash", "bracketright", "asciicircum", "underscore", "grave", "a",
	"b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft",
	"bar", "braceright", "asciitilde", _notdef, "Adieresis", "Aring",
	"Ccedilla", "Eacute", "Ntilde", "Odieresis", "Udieresis", "aacute",
	"agrave", "acircumflex", "adieresis", "atilde", "aring", "ccedilla",
	"eacute", "egrave", "ecircumflex", "edieresis", "iacute", "igrave",
	"icircumflex", "idieresis", "ntilde", "oacute", "ograve",
	"ocircumflex", "odieresis", "otilde", "uacute", "ugrave",
	"ucircumflex", "udieresis", "dagger", "degree", "cent", "sterling",
	"section", "bullet", "paragraph", "germandbls", "registered",
	"copyright", "trademark", "acute", "dieresis", _notdef, "AE", "Oslash",
	_notdef, "plusminus", _notdef, _notdef, "yen", "mu", _notdef, _notdef,
	_notdef, _notdef, _notdef, "ordfeminine", "ordmasculine", _notdef,
	"ae", "oslash", "questiondown", "exclamdown", "logicalnot", _notdef,
	"florin", _notdef, _notdef, "guillemotleft", "guillemotright",
	"ellipsis", "space", "Agrave", "Atilde", "Otilde", "OE", "oe",
	"endash", "emdash", "quotedblleft", "quotedblright", "quoteleft",
	"quoteright", "divide", _notdef, "ydieresis", "Ydieresis", "fraction",
	"currency", "guilsinglleft", "guilsinglright", "fi", "fl", "daggerdbl",
	"periodcentered", "quotesinglbase", "quotedblbase", "perthousand",
	"Acircumflex", "Ecircumflex", "Aacute", "Edieresis", "Egrave",
	"Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute",
	"Ocircumflex", _notdef, "Ograve", "Uacute", "Ucircumflex", "Ugrave",
	"dotlessi", "circumflex", "tilde", "macron", "breve", "dotaccent",
	"ring", "cedilla", "hungarumlaut", "ogonek", "caron"
};

const char *pdf_mac_expert[256] = {
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"space", "exclamsmall", "Hungarumlautsmall", "centoldstyle",
	"dollaroldstyle", "dollarsuperior", "ampersandsmall", "Acutesmall",
	"parenleftsuperior", "parenrightsuperior", "twodotenleader",
	"onedotenleader", "comma", "hyphen", "period", "fraction",
	"zerooldstyle", "oneoldstyle", "twooldstyle", "threeoldstyle",
	"fouroldstyle", "fiveoldstyle", "sixoldstyle", "sevenoldstyle",
	"eightoldstyle", "nineoldstyle", "colon", "semicolon", _notdef,
	"threequartersemdash", _notdef, "questionsmall", _notdef, _notdef,
	_notdef, _notdef, "Ethsmall", _notdef, _notdef, "onequarter",
	"onehalf", "threequarters", "oneeighth", "threeeighths", "fiveeighths",
	"seveneighths", "onethird", "twothirds", _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, "ff", "fi", "fl", "ffi", "ffl",
	"parenleftinferior", _notdef, "parenrightinferior", "Circumflexsmall",
	"hypheninferior", "Gravesmall", "Asmall", "Bsmall", "Csmall", "Dsmall",
	"Esmall", "Fsmall", "Gsmall", "Hsmall", "Ismall", "Jsmall", "Ksmall",
	"Lsmall", "Msmall", "Nsmall", "Osmall", "Psmall", "Qsmall", "Rsmall",
	"Ssmall", "Tsmall", "Usmall", "Vsmall", "Wsmall", "Xsmall", "Ysmall",
	"Zsmall", "colonmonetary", "onefitted", "rupiah", "Tildesmall",
	_notdef, _notdef, "asuperior", "centsuperior", _notdef, _notdef,
	_notdef, _notdef, "Aacutesmall", "Agravesmall", "Acircumflexsmall",
	"Adieresissmall", "Atildesmall", "Aringsmall", "Ccedillasmall",
	"Eacutesmall", "Egravesmall", "Ecircumflexsmall", "Edieresissmall",
	"Iacutesmall", "Igravesmall", "Icircumflexsmall", "Idieresissmall",
	"Ntildesmall", "Oacutesmall", "Ogravesmall", "Ocircumflexsmall",
	"Odieresissmall", "Otildesmall", "Uacutesmall", "Ugravesmall",
	"Ucircumflexsmall", "Udieresissmall", _notdef, "eightsuperior",
	"fourinferior", "threeinferior", "sixinferior", "eightinferior",
	"seveninferior", "Scaronsmall", _notdef, "centinferior", "twoinferior",
	_notdef, "Dieresissmall", _notdef, "Caronsmall", "osuperior",
	"fiveinferior", _notdef, "commainferior", "periodinferior",
	"Yacutesmall", _notdef, "dollarinferior", _notdef, _notdef,
	"Thornsmall", _notdef, "nineinferior", "zeroinferior", "Zcaronsmall",
	"AEsmall", "Oslashsmall", "questiondownsmall", "oneinferior",
	"Lslashsmall", _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"Cedillasmall", _notdef, _notdef, _notdef, _notdef, _notdef, "OEsmall",
	"figuredash", "hyphensuperior", _notdef, _notdef, _notdef, _notdef,
	"exclamdownsmall", _notdef, "Ydieresissmall", _notdef, "onesuperior",
	"twosuperior", "threesuperior", "foursuperior", "fivesuperior",
	"sixsuperior", "sevensuperior", "ninesuperior", "zerosuperior",
	_notdef, "esuperior", "rsuperior", "tsuperior", _notdef, _notdef,
	"isuperior", "ssuperior", "dsuperior", _notdef, _notdef, _notdef,
	_notdef, _notdef, "lsuperior", "Ogoneksmall", "Brevesmall",
	"Macronsmall", "bsuperior", "nsuperior", "msuperior", "commasuperior",
	"periodsuperior", "Dotaccentsmall", "Ringsmall", _notdef, _notdef,
	_notdef, _notdef
};

const char *pdf_win_ansi[256] = {
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"space", "exclam", "quotedbl", "numbersign", "dollar", "percent",
	"ampersand", "quotesingle", "parenleft", "parenright", "asterisk",
	"plus", "comma", "hyphen", "period", "slash", "zero", "one", "two",
	"three", "four", "five", "six", "seven", "eight", "nine", "colon",
	"semicolon", "less", "equal", "greater", "question", "at", "A", "B",
	"C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
	"Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft",
	"backslash", "bracketright", "asciicircum", "underscore", "grave", "a",
	"b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft",
	"bar", "braceright", "asciitilde", "bullet", "Euro", "bullet",
	"quotesinglbase", "florin", "quotedblbase", "ellipsis", "dagger",
	"daggerdbl", "circumflex", "perthousand", "Scaron", "guilsinglleft",
	"OE", "bullet", "Zcaron", "bullet", "bullet", "quoteleft",
	"quoteright", "quotedblleft", "quotedblright", "bullet", "endash",
	"emdash", "tilde", "trademark", "scaron", "guilsinglright", "oe",
	"bullet", "zcaron", "Ydieresis", "space", "exclamdown", "cent",
	"sterling", "currency", "yen", "brokenbar", "section", "dieresis",
	"copyright", "ordfeminine", "guillemotleft", "logicalnot", "hyphen",
	"registered", "macron", "degree", "plusminus", "twosuperior",
	"threesuperior", "acute", "mu", "paragraph", "periodcentered",
	"cedilla", "onesuperior", "ordmasculine", "guillemotright",
	"onequarter", "onehalf", "threequarters", "questiondown", "Agrave",
	"Aacute", "Acircumflex", "Atilde", "Adieresis", "Aring", "AE",
	"Ccedilla", "Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave",
	"Iacute", "Icircumflex", "Idieresis", "Eth", "Ntilde", "Ograve",
	"Oacute", "Ocircumflex", "Otilde", "Odieresis", "multiply", "Oslash",
	"Ugrave", "Uacute", "Ucircumflex", "Udieresis", "Yacute", "Thorn",
	"germandbls", "agrave", "aacute", "acircumflex", "atilde", "adieresis",
	"aring", "ae", "ccedilla", "egrave", "eacute", "ecircumflex",
	"edieresis", "igrave", "iacute", "icircumflex", "idieresis", "eth",
	"ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis",
	"divide", "oslash", "ugrave", "uacute", "ucircumflex", "udieresis",
	"yacute", "thorn", "ydieresis"
};

const char *pdf_glyph_name_from_koi8u[256] = {
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"space", "exclam", "quotedbl", "numbersign", "dollar", "percent",
	"ampersand", "quotesingle", "parenleft", "parenright", "asterisk",
	"plus", "comma", "hyphen", "period", "slash", "zero", "one", "two",
	"three", "four", "five", "six", "seven", "eight", "nine", "colon",
	"semicolon", "less", "equal", "greater", "question", "at", "A", "B",
	"C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
	"Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft",
	"backslash", "bracketright", "asciicircum", "underscore", "grave", "a",
	"b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft",
	"bar", "braceright", "asciitilde", _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, "integraltp", _notdef, "bulletoperator",
	"radical", "approxequal", "lessequal", "greaterequal",
	"nonbreakingspace", "integralbt", "degree", "twosuperior",
	"periodcentered", "divide", _notdef, _notdef, _notdef, "iocyrillic",
	"ecyrillic", _notdef, "icyrillic", "yicyrillic", _notdef, _notdef,
	_notdef, _notdef, _notdef, "gheupturncyrillic", _notdef, _notdef,
	_notdef, _notdef, _notdef, "afii10023", "afii10053", _notdef,
	"afii10055", "afii10056", _notdef, _notdef, _notdef, _notdef, _notdef,
	"afii10050", _notdef, "copyright", "iucyrillic", "afii10065",
	"becyrillic", "tsecyrillic", "decyrillic", "iecyrillic", "efcyrillic",
	"gecyrillic", "khacyrillic", "iicyrillic", "iishortcyrillic",
	"kacyrillic", "elcyrillic", "emcyrillic", "encyrillic", "ocyrillic",
	"pecyrillic", "iacyrillic", "ercyrillic", "escyrillic", "tecyrillic",
	"ucyrillic", "zhecyrillic", "vecyrillic", "softsigncyrillic",
	"yericyrillic", "zecyrillic", "shacyrillic", "ereversedcyrillic",
	"shchacyrillic", "checyrillic", "hardsigncyrillic", "afii10048",
	"afii10017", "afii10018", "afii10040", "afii10021", "afii10022",
	"afii10038", "afii10020", "afii10039", "afii10026", "afii10027",
	"afii10028", "afii10029", "afii10030", "afii10031", "afii10032",
	"afii10033", "afii10049", "afii10034", "afii10035", "afii10036",
	"afii10037", "afii10024", "afii10019", "afii10046", "afii10045",
	"afii10025", "afii10042", "afii10047", "afii10043", "afii10041",
	"afii10044",
};

static const struct { unsigned short u, c; } koi8u_from_unicode[] = {
	{0x00a0,154}, {0x00a9,191}, {0x00b0,156}, {0x00b2,157}, {0x00b7,158},
	{0x00f7,159}, {0x0401,179}, {0x0404,180}, {0x0406,182}, {0x0407,183},
	{0x0410,225}, {0x0411,226}, {0x0412,247}, {0x0413,231}, {0x0414,228},
	{0x0415,229}, {0x0416,246}, {0x0417,250}, {0x0418,233}, {0x0419,234},
	{0x041a,235}, {0x041b,236}, {0x041c,237}, {0x041d,238}, {0x041e,239},
	{0x041f,240}, {0x0420,242}, {0x0421,243}, {0x0422,244}, {0x0423,245},
	{0x0424,230}, {0x0425,232}, {0x0426,227}, {0x0427,254}, {0x0428,251},
	{0x0429,253}, {0x042a,255}, {0x042b,249}, {0x042c,248}, {0x042d,252},
	{0x042e,224}, {0x042f,241}, {0x0430,193}, {0x0431,194}, {0x0432,215},
	{0x0433,199}, {0x0434,196}, {0x0435,197}, {0x0436,214}, {0x0437,218},
	{0x0438,201}, {0x0439,202}, {0x043a,203}, {0x043b,204}, {0x043c,205},
	{0x043d,206}, {0x043e,207}, {0x043f,208}, {0x0440,210}, {0x0441,211},
	{0x0442,212}, {0x0443,213}, {0x0444,198}, {0x0445,200}, {0x0446,195},
	{0x0447,222}, {0x0448,219}, {0x0449,221}, {0x044a,223}, {0x044b,217},
	{0x044c,216}, {0x044d,220}, {0x044e,192}, {0x044f,209}, {0x0451,163},
	{0x0454,164}, {0x0456,166}, {0x0457,167}, {0x0490,189}, {0x0491,173},
	{0x2219,149}, {0x221a,150}, {0x2248,151}, {0x2264,152}, {0x2265,153},
	{0x2320,147}, {0x2321,155}, {0x2500,128}, {0x2502,129}, {0x250c,130},
	{0x2510,131}, {0x2514,132}, {0x2518,133}, {0x251c,134}, {0x2524,135},
	{0x252c,136}, {0x2534,137}, {0x253c,138}, {0x2550,160}, {0x2551,161},
	{0x2552,162}, {0x2554,165}, {0x2557,168}, {0x2558,169}, {0x2559,170},
	{0x255a,171}, {0x255b,172}, {0x255d,174}, {0x255e,175}, {0x255f,176},
	{0x2560,177}, {0x2561,178}, {0x2563,181}, {0x2566,184}, {0x2567,185},
	{0x2568,186}, {0x2569,187}, {0x256a,188}, {0x256c,190}, {0x2580,139},
	{0x2584,140}, {0x2588,141}, {0x258c,142}, {0x2590,143}, {0x2591,144},
	{0x2592,145}, {0x2593,146}, {0x25a0,148}
};

const char *pdf_glyph_name_from_iso8859_7[256] = {
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"space", "exclam", "quotedbl", "numbersign", "dollar", "percent",
	"ampersand", "quotesingle", "parenleft", "parenright", "asterisk",
	"plus", "comma", "hyphen", "period", "slash", "zero", "one", "two",
	"three", "four", "five", "six", "seven", "eight", "nine", "colon",
	"semicolon", "less", "equal", "greater", "question", "at", "A", "B",
	"C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
	"Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft",
	"backslash", "bracketright", "asciicircum", "underscore", "grave", "a",
	"b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft",
	"bar", "braceright", "asciitilde", _notdef,
	/* the block drawing characters have been omitted */
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"nonbreakingspace", "quoteleft", "quoteright", "sterling", "euro",
	_notdef, "brokenbar", "section", "dieresis", "copyright",
	"ypogegrammeni", "guillemotleft", "logicalnot", "softhyphen", _notdef,
	"horizontalbar", "degree", "plusminus", "twosuperior", "threesuperior",
	"tonos", "dieresistonos", "Alphatonos", "periodcentered",
	"Epsilontonos", "Etatonos", "Iotatonos", "guillemotright",
	"Omicrontonos", "onehalf", "Upsilontonos", "Omegatonos",
	"iotadieresistonos", "Alpha", "Beta", "Gamma", "Deltagreek", "Epsilon",
	"Zeta", "Eta", "Theta", "Iota", "Kappa", "Lambda", "Mu", "Nu", "Xi",
	"Omicron", "Pi", "Rho", _notdef, "Sigma", "Tau", "Upsilon", "Phi",
	"Chi", "Psi", "Omegagreek", "Iotadieresis", "Upsilondieresis",
	"alphatonos", "epsilontonos", "etatonos", "iotatonos",
	"upsilondieresistonos", "alpha", "beta", "gamma", "delta", "epsilon",
	"zeta", "eta", "theta", "iota", "kappa", "lambda", "mugreek", "nu",
	"xi", "omicron", "pi", "rho", "sigmafinal", "sigma", "tau", "upsilon",
	"phi", "chi", "psi", "omega", "iotadieresis", "upsilondieresis",
	"omicrontonos", "upsilontonos", "omegatonos", _notdef,
};

static const struct { unsigned short u, c; } iso8859_7_from_unicode[] = {
	{0x00a0,160}, {0x00a3,163}, {0x00a6,166}, {0x00a7,167}, {0x00a8,168},
	{0x00a9,169}, {0x00ab,171}, {0x00ac,172}, {0x00ad,173}, {0x00b0,176},
	{0x00b1,177}, {0x00b2,178}, {0x00b3,179}, {0x00b7,183}, {0x00bb,187},
	{0x00bd,189}, {0x037a,170}, {0x0384,180}, {0x0385,181}, {0x0386,182},
	{0x0388,184}, {0x0389,185}, {0x038a,186}, {0x038c,188}, {0x038e,190},
	{0x038f,191}, {0x0390,192}, {0x0391,193}, {0x0392,194}, {0x0393,195},
	{0x0394,196}, {0x0395,197}, {0x0396,198}, {0x0397,199}, {0x0398,200},
	{0x0399,201}, {0x039a,202}, {0x039b,203}, {0x039c,204}, {0x039d,205},
	{0x039e,206}, {0x039f,207}, {0x03a0,208}, {0x03a1,209}, {0x03a3,211},
	{0x03a4,212}, {0x03a5,213}, {0x03a6,214}, {0x03a7,215}, {0x03a8,216},
	{0x03a9,217}, {0x03aa,218}, {0x03ab,219}, {0x03ac,220}, {0x03ad,221},
	{0x03ae,222}, {0x03af,223}, {0x03b0,224}, {0x03b1,225}, {0x03b2,226},
	{0x03b3,227}, {0x03b4,228}, {0x03b5,229}, {0x03b6,230}, {0x03b7,231},
	{0x03b8,232}, {0x03b9,233}, {0x03ba,234}, {0x03bb,235}, {0x03bc,236},
	{0x03bd,237}, {0x03be,238}, {0x03bf,239}, {0x03c0,240}, {0x03c1,241},
	{0x03c2,242}, {0x03c3,243}, {0x03c4,244}, {0x03c5,245}, {0x03c6,246},
	{0x03c7,247}, {0x03c8,248}, {0x03c9,249}, {0x03ca,250}, {0x03cb,251},
	{0x03cc,252}, {0x03cd,253}, {0x03ce,254}, {0x2015,175}, {0x2018,161},
	{0x2019,162}, {0x20ac,164},
};
