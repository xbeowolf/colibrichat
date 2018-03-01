/******************************************************************************
*                                                                             *
* Cyclic redundancy code                                                      *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2010. All rights reserved.       *
*                                                                             *
******************************************************************************/

#include "stdafx.h"

#include "CRC.h"

// calculate a 32 bit crc of a string
// featuring Julio Jerez
static CRC32 tableJJ[] = {
	0x00000001, 0x2C11F801, 0xDFD8F60E, 0x6C8FA2B7, 
	0xB573754C, 0x1522DCDD, 0x21615D3A, 0xE1B307F3, 
	0x12AFA158, 0x53D18179, 0x70950126, 0x941702EF, 
	0x756FE824, 0x694801D5, 0x36DF4DD2, 0x63D80FAB, 
	0xB8AE95B0, 0x902439F1, 0x090C6F3E, 0x2B7C6A27, 
	0x8344B5FC, 0x67D3C5CD, 0x22F5516A, 0x2FB00E63, 
	0xFC761508, 0x87B00169, 0x27EBA056, 0x8CC0B85F, 
	0xE33D3ED4, 0x95DA08C5, 0x13E5C802, 0x9DD9E41B, 
	0xD4577F60, 0x3DD6B7E1, 0x096AF46E, 0x1A00CD97, 
	0x4B10E2AC, 0x22EAAABD, 0x683F119A, 0x62D070D3, 
	0xA8D034B8, 0xAA363D59, 0x58CECB86, 0x40F589CF, 
	0x4F630184, 0x38918BB5, 0xB85B8E32, 0x0A6A948B, 
	0x9A099510, 0x402871D1, 0x11E7859E, 0xEE73CD07, 
	0x4142FB5C, 0x39D68BAD, 0x0FE19DCA, 0xE35B2F43, 
	0x75590068, 0x66433549, 0x929182B6, 0x71EC773F, 
	0xBBAC3034, 0xF2BD8AA5, 0x5743A062, 0x5AB120FB, 
	0x5ABFD6C0, 0xDDD867C1, 0xDC3522CE, 0xD0EC6877, 
	0xE106000C, 0xB7C6689D, 0xED3FF5FA, 0xC75749B3, 
	0x126B7818, 0x1A75E939, 0x0546C5E6, 0x8A9C80AF, 
	0x48A3CAE4, 0x756D0595, 0x7060FE92, 0xA594896B, 
	0x12354470, 0x896599B1, 0xDAC6CBFE, 0xCB419FE7, 
	0x9C44F0BC, 0xAFA9418D, 0xB87D1A2A, 0x428BC023, 
	0x33229BC8, 0xC92D5929, 0xB1C19516, 0x0FBCA61F, 
	0xE594D194, 0x716EFC85, 0x0036A8C2, 0xD7BBCDDB, 
	0x16E4DE20, 0xD10F07A1, 0x68CF812E, 0x390A7357, 
	0x8BAACD6C, 0x2C2E167D, 0x3E7C0A5A, 0x167F9293, 
	0x3D596B78, 0x08888519, 0x9994F046, 0x0FC3E78F, 
	0x008A4444, 0x87526F75, 0xB0079EF2, 0x238DEE4B, 
	0xCA09A3D0, 0x4ED3B191, 0xFA42425E, 0x379DE2C7, 
	0x1EA2961C, 0x1FC3E76D, 0x90DFC68A, 0x0279C103, 
	0xF9AAE728, 0xF2666D09, 0xEF13D776, 0x92E944FF, 
	0x364F22F4, 0x37665E65, 0x05D6E122, 0x7131EABB, 
	0x479E9580, 0x98729781, 0x4BD20F8E, 0x1612EE37, 
	0xCB574ACC, 0x5499B45D, 0x360B4EBA, 0x33814B73, 
	0x43720ED8, 0x146610F9, 0x45514AA6, 0x0B23BE6F, 
	0x026E6DA4, 0xD1B9C955, 0x94676F52, 0xCE8EC32B, 
	0x165EB330, 0x2F6AB971, 0x92F1E8BE, 0xC54095A7, 
	0xBEB3EB7C, 0x5C9E7D4D, 0x5921A2EA, 0xB45D31E3, 
	0xB8C9E288, 0x5FE670E9, 0xC02049D6, 0xC42A53DF, 
	0x6F332454, 0x661BB045, 0x2B3C4982, 0xDF4B779B, 
	0xD7C4FCE0, 0x70FB1761, 0xADD4CDEE, 0x47BDD917, 
	0x8C63782C, 0x8181423D, 0xFA05C31A, 0xDD947453, 
	0x6A8D6238, 0x1A068CD9, 0x4413D506, 0x5374054F, 
	0xC5A84704, 0xB41B1335, 0x06986FB2, 0x4CCF080B, 
	0xF80C7290, 0x8622B151, 0x536DBF1E, 0x21E1B887, 
	0xDED0F0DC, 0xB4B1032D, 0x1D5AAF4A, 0xC56E12C3, 
	0x8C578DE8, 0xCBA564C9, 0xA67EEC36, 0x0837D2BF, 
	0x3D98D5B4, 0x1B06F225, 0xFF7EE1E2, 0x3640747B, 
	0x5E301440, 0x53A08741, 0x436FBC4E, 0xC9C333F7, 
	0x2727558C, 0x7F5CC01D, 0xFC83677A, 0xAFF10D33, 
	0x24836598, 0x3161F8B9, 0xDD748F66, 0x5B6CBC2F, 
	0xAD8FD064, 0x89EE4D15, 0xBBB2A012, 0xA086BCEB, 
	0x1BEAE1F0, 0x69F39931, 0x764DC57E, 0x17394B67, 
	0x4D51A63C, 0xF273790D, 0x35A2EBAA, 0x7EE463A3, 
	0xBC2BE948, 0x2B9B48A9, 0x2FC7BE96, 0x5FC9C19F, 
	0x3AD83714, 0x6FA02405, 0xDDB6AA42, 0xE648E15B, 
	0x1DB7DBA0, 0xF55AE721, 0x4D3ADAAE, 0xB3DAFED7, 
	0x5FFAE2EC, 0x96A42DFD, 0xFB9C3BDA, 0x21CF1613, 
	0x0F2C18F8, 0xAE705499, 0x650B79C6, 0x31C5E30F, 
	0x097D09C4, 0xAAAB76F5, 0x34CE0072, 0x27EDE1CB, 
	0xDAD20150, 0xADD57111, 0xC229FBDE, 0x8AFF4E47, 
	0x448E0B9C, 0x5C5DDEED, 0x4612580A, 0x05F82483, 
	0xBC1EF4A8, 0xB1C01C89, 0xF592C0F6, 0x6798207F, 
	0xEC494874, 0x795F45E5, 0xECFBA2A2, 0xBB9CBE3B, 
	0xF567104f, 0x47289407, 0x25683fa6, 0x2fde5836, 
};

CRC32 CRCJJ(const char *name, CRC32 crc)
{
	if (!name) return crc;

	while (*name) {
		crc = tableJJ[(crc >> 24) ^ (*name++)] ^ (crc << 8);
	}
	return crc;
}

CRC32 wCRCJJ(const wchar_t *name, CRC32 crc)
{
	if (!name) return crc;

	unsigned char c;
	for (TCHAR *ptr = (TCHAR*)name; *ptr; ptr++) {
		c = (unsigned char)(*ptr ^ (*ptr >> 8));
		crc = tableJJ[(crc >> 24) ^ c] ^ (crc << 8);
	}
	return crc;
}

#define DO1 crc = table[(unsigned char)(crc) ^ *data++] ^ (crc >> 8)
#define DO8 DO1; DO1; DO1; DO1; DO1; DO1; DO1; DO1

struct watchdog {
	watchdog(const CRC32 CRC_POLY, CRC32* table) {
		unsigned i, j;
		unsigned r;
		for (i = 0; i < 256; i++){
			for (r = i, j = 8; j; j--)
				if (r & 1) r = (r >> 1) ^ CRC_POLY;
				else r >>= 1;
				table[i] = r;
		}
	};
};

static CRC32 _CRC32(const unsigned char* data, size_t len, CRC32 crc, const CRC32* table)
{
	if (!data) return crc;

	while (len >= 8) {
		DO8;
		len -= 8;
	}
	while (len--) {
		DO1;
	}
	return crc;
}

static CRC16 _CRC16(const unsigned char* data, size_t len, CRC16 crc, const CRC16* table)
{
	if (!data) return crc;

	while (len >= 8) {
		DO8;
		len -= 8;
	}
	while (len--) {
		DO1;
	}

	return crc;
}

CRC32 CRC32IEEE(const void* data, size_t len, CRC32 crc)
{
	static CRC32 table[256];
	static const watchdog wd(0xEDB88320, table);

	return _CRC32((const unsigned char*)data, len, crc, table);
}

CRC32 CRC32C(const void* data, size_t len, CRC32 crc)
{
	static CRC32 table[256];
	static const watchdog wd(0x82F63B78, table);

	return _CRC32((const unsigned char*)data, len, crc, table);
}

CRC32 CRC32K(const void* data, size_t len, CRC32 crc)
{
	static CRC32 table[256];
	static const watchdog wd(0xEB31D82E, table);

	return _CRC32((const unsigned char*)data, len, crc, table);
}

CRC32 CRC32Q(const void* data, size_t len, CRC32 crc)
{
	static CRC32 table[256];
	static const watchdog wd(0xD5828281, table);

	return _CRC32((const unsigned char*)data, len, crc, table);
}

CRC32 CRC32IEEEtbl(const void* data, size_t len, CRC32 crc)
{
	static CRC32 table[256] = {
		0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
		0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
		0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
		0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,

		0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
		0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
		0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
		0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,

		0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
		0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
		0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
		0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,

		0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
		0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
		0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
		0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,

		0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
		0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
		0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
		0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,

		0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
		0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
		0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
		0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,

		0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
		0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
		0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
		0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,

		0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
		0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
		0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
		0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,

		0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
		0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
		0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
		0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,

		0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
		0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
		0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
		0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,

		0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
		0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
		0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
		0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,

		0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
		0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
		0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
		0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,

		0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
		0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
		0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
		0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,

		0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
		0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
		0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
		0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,

		0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
		0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
		0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
		0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,

		0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
		0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
		0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
		0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
	};

	return _CRC32((const unsigned char*)data, len, crc, table);
}

CRC16 CRC16CCITTtbl(const void* data, size_t len, CRC16 crc)
{
	static CRC16 table[256] = {
		0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
		0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
		0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
		0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
		0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
		0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
		0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
		0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
		0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
		0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
		0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
		0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
		0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
		0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
		0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
		0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
		0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
		0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
		0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
		0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
		0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
		0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
		0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
		0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
		0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
		0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
		0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
		0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
		0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
		0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
		0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
		0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78,
	};

	return _CRC16((const unsigned char*)data, len, crc, table);
}