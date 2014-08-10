#include "adts_parser.h"
#include <stdlib.h>
#include <stdio.h>

#define ADTS_MPEG_VERSION_MASK 0x8
#define ADTS_PROTECTION_MASK 0x1
#define ADTS_PROFILE_MASK  0xC0
#define ADTS_SAMPLE_FREQ_MASK 0x3C
#define ADTS_CHANNEL_CONF_MASK1 0x1
#define ADTS_CHANNEL_CONF_MASK2  0xC0
#define ADTS_FRAME_LEN_MASK1 0x3
#define ADTS_FRAME_LEN_MASK2 0xE0
#define ADTS_FRAME_NUMBER_MASK 0x3

#define ADTS_AAC_ELEMENT_MASK 0x7

#define ADTS_AAC_ELEMENT_OFFSET_WITHOUT_CRC 7
#define ADTS_AAC_ELEMENT_OFFSET_WITH_CRC 9

char* mpeg4_audio_type_table[AUDIO_TYPE_MAX] = {
	"AUDIO_TYPE_NULL",
	"AUDIO_TYPE_AAC_MAIN",
	"AUDIO_TYPE_AAC_LC",
	"AUDIO_TYPE_AAC_SSR",
	"AUDIO_TYPE_AAC_LTP",
	"AUDIO_TYPE_SBR",
	"AUDIO_TYPE_AAC_SCALABLE",
	"AUDIO_TYPE_TWIN_VQ",
	"AUDIO_TYPE_CELP",
	"AUDIO_TYPE_HXVC",
	"AUDIO_TYPE_RESERVED1",
	"AUDIO_TYPE_RESERVED2 ",
	"AUDIO_TYPE_TTSI ",
	"AUDIO_TYPE_MAIN_SYNTHESIS",
	"AUDIO_TYPE_WAVETABLE_SYNTHESIS",
	"AUDIO_TYPE_GENERAL_MIDI ",
	"AUDIO_TYPE_ASAE",
	"AUDIO_TYPE_ER_AAC_LC",
	"AUDIO_TYPE_RESERVED3",
	"AUDIO_TYPE_ER_AAC_LTP",
	"AUDIO_TYPE_ER_AAC_SCALABLE",
	"AUDIO_TYPE_ER_TWIN_VQ",
	"AUDIO_TYPE_ER_BSAC",
	"AUDIO_TYPE_ER_AAC_LD",
	"AUDIO_TYPE_ER_CELP",
	"AUDIO_TYPE_ER_HVXC",
	"AUDIO_TYPE_ER_HILN",
	"AUDIO_TYPE_ER_PARAMETRIC",
	"AUDIO_TYPE_SSC",
	"AUDIO_TYPE_PS",
	"AUDIO_TYPE_MPEG_SURROUND",
	"AUDIO_TYPE_ESCAPE_VALUE",
	"AUDIO_TYPE_LAYER_1",
	"AUDIO_TYPE_LAYER_2",
	"AUDIO_TYPE_LAYER_3",
	"AUDIO_TYPE_DST",
	"AUDIO_TYPE_ALS",
	"AUDIO_TYPE_SLS",
	"AUDIO_TYPE_SLS_NON_CORE",
	"AUDIO_TYPE_ER_AAC_ELD",
	"AUDIO_TYPE_SMR",
	"AUDIO_TYPE_SMR_MAIN",
	"AUDIO_TYPE_USAC_NO_SBR",
	"AUDIO_TYPE_SAOC",
	"AUDIO_TYPE_LD_MPEG_SURROUND",
	"AUDIO_TYPE_USAC"
};

char* mpeg4_audio_sample_freq_table[AUDIO_SAMPLE_FREQ_MAX] = {
	"AUDIO_SAMPLE_FREQ_96000HZ",
	"AUDIO_SAMPLE_FREQ_88200HZ",
	"AUDIO_SAMPLE_FREQ_64000HZ",
	"AUDIO_SAMPLE_FREQ_48000HZ",
	"AUDIO_SAMPLE_FREQ_44100HZ",
	"AUDIO_SAMPLE_FREQ_32000HZ",
	"AUDIO_SAMPLE_FREQ_24000HZ",
	"AUDIO_SAMPLE_FREQ_22050HZ",
	"AUDIO_SAMPLE_FREQ_16000HZ",
	"AUDIO_SAMPLE_FREQ_12000HZ",
	"AUDIO_SAMPLE_FREQ_11025HZ",
	"AUDIO_SAMPLE_FREQ_8000HZ",
	"AUDIO_SAMPLE_FREQ_7350HZ",
	"AUDIO_SAMPLE_FREQ_RESERVED1",
	"AUDIO_SAMPLE_FREQ_RESERVED2",
	"AUDIO_SAMPLE_FREQ_UNKNOWN"
};

char* mpeg4_audio_channel_config_table[AUDIO_CHANNEL_CONFIG_MAX] = {
	"AUDIO_CHANNEL_CONFIG_AOT",
	"AUDIO_CHANNEL_CONFIG_1_FC",
	"AUDIO_CHANNEL_CONFIG_2_FL_FR",
	"AUDIO_CHANNEL_CONFIG_3_FC_FL_FR",
	"AUDIO_CHANNEL_CONFIG_4_FC_FL_FR_BC",
	"AUDIO_CHANNEL_CONFIG_5_FC_FL_FR_BL_BR",
	"AUDIO_CHANNEL_CONFIG_6_FC_FL_FR_BL_BR_LFE",
	"AUDIO_CHANNEL_CONFIG_7_FC_FL_FR_SL_SR_BL_BR_LFE",
	"AUDIO_CHANNEL_CONFIG_RESERVED1",
	"AUDIO_CHANNEL_CONFIG_RESERVED2",
	"AUDIO_CHANNEL_CONFIG_RESERVED3",
	"AUDIO_CHANNEL_CONFIG_RESERVED4",
	"AUDIO_CHANNEL_CONFIG_RESERVED5",
	"AUDIO_CHANNEL_CONFIG_RESERVED6",
	"AUDIO_CHANNEL_CONFIG_RESERVED7",
	"AUDIO_CHANNEL_CONFIG_RESERVED8"
};

char* aac_element_type_table[AAC_ELEMENT_TYPE_MAX] = {
	"AAC_ELEMENT_TYPE_SCE",
	"AAC_ELEMENT_TYPE_CPE",
	"AAC_ELEMENT_TYPE_CCE",
	"AAC_ELEMENT_TYPE_LFE",
	"AAC_ELEMENT_TYPE_DSE",
	"AAC_ELEMENT_TYPE_PCE",
	"AAC_ELEMENT_TYPE_FIL",
	"AAC_ELEMENT_TYPE_END",
};

/*
AAAAAAAA AAAABCCD EEFFFFGH HHIJKLMM MMMMMMMM MMMOOOOO OOOOOOPP (QQQQQQQQ QQQQQQQQ)
* Header consists of 7 or 9 bytes (without or with CRC).
Letter	Length (bits)	Description
A	12	syncword 0xFFF, all bits must be 1
B	1	MPEG Version: 0 for MPEG-4, 1 for MPEG-2
C	2	Layer: always 0
D	1	protection absent, Warning, set to 1 if there is no CRC and 0 if there is CRC
E	2	profile, the MPEG-4 Audio Object Type minus 1
F	4	MPEG-4 Sampling Frequency Index (15 is forbidden)
G	1	private stream, set to 0 when encoding, ignore when decoding
H	3	MPEG-4 Channel Configuration (in the case of 0, the channel configuration is sent via an inband PCE)
I	1	originality, set to 0 when encoding, ignore when decoding
J	1	home, set to 0 when encoding, ignore when decoding
K	1	copyrighted stream, set to 0 when encoding, ignore when decoding
L	1	copyright start, set to 0 when encoding, ignore when decoding
M	13	frame length, this value must include 7 or 9 bytes of header length: FrameLength = (ProtectionAbsent == 1 ? 7 : 9) + size(AACFrame)
O	11	Buffer fullness
P	2	Number of AAC frames (RDBs) in ADTS frame minus 1, for maximum compatibility always use 1 AAC frame per ADTS frame
Q	16	CRC if protection absent is 0
* 
*/

/*only dump first element type*/
void dump_aac_elements(uint8_t* buf){
	int element_type = 0;
	
	element_type = ((buf[0] >> 5) &ADTS_AAC_ELEMENT_MASK);
	printf("element type:%s\n\n", aac_element_type_table[element_type]);

	return;
}

void dump_adts_info(uint8_t* buf){
	int mpeg_version = 0;
	int protection_absent = 0;
	int profile = 0;
	int sample_freq = 0;
	int channel_conf = 0;
	int frame_len = 0;
	int frame_num = 0;
	
	if (buf == NULL){
		return;
	}
	
	if( buf[0] != 0xFF && (buf[1] >>4) != 0xF){
		printf("Not a valid adts head: %2x%1x\n",  buf[0],  (buf[1] >>4));
		return;
	}
	
	mpeg_version = ((buf[1] & ADTS_MPEG_VERSION_MASK) >> 3);
	protection_absent = (buf[1] & ADTS_PROTECTION_MASK);
	profile = ((buf[2] &ADTS_PROFILE_MASK) >>6)+ 1;
	sample_freq = ((buf[2]&ADTS_SAMPLE_FREQ_MASK) >> 2);
	channel_conf =  ((buf[2] & ADTS_CHANNEL_CONF_MASK1) <<2) + ((buf[3] & ADTS_CHANNEL_CONF_MASK2) >>6);
	frame_len = ((buf[3] & ADTS_FRAME_LEN_MASK1)  << 11)+
							((buf[4])  << 3) +
							((buf[5] & ADTS_FRAME_LEN_MASK2) >>5) ;
							
	frame_num = (buf[6] & ADTS_FRAME_NUMBER_MASK) + 1;
	
	printf("mpeg version:%s\n", (mpeg_version == 0)?"MPEG-4":"MPEG-2");
	printf("protection:%s\n", (protection_absent == 1)?"no CRC":"CRC");
	printf("profile:%s\n", mpeg4_audio_type_table[profile]);
	printf("sample req:%s\n", mpeg4_audio_sample_freq_table[sample_freq]);
	printf("channal conf:%s\n", mpeg4_audio_channel_config_table[channel_conf]);
	printf("frame len:%d\n", frame_len);
	printf("frame number:%d\n", frame_num);
	
	if(protection_absent == 1){
		//no CRC
		dump_aac_elements(buf + ADTS_AAC_ELEMENT_OFFSET_WITHOUT_CRC);
	}else{
		//have CRC
		dump_aac_elements(buf + ADTS_AAC_ELEMENT_OFFSET_WITH_CRC);
	}
	
	return;
}
