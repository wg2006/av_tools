#ifndef _ADTS_PARSER_H_
#define _ADTS_PARSER_H_
#include<stdint.h>
/*
 *  following definition, please refer to 
 * http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio#Channel_Configurations
 * */
enum mpeg4_audio_type{
	AUDIO_TYPE_NULL = 0,
	AUDIO_TYPE_AAC_MAIN = 1,
	AUDIO_TYPE_AAC_LC = 2,
	AUDIO_TYPE_AAC_SSR = 3,
	AUDIO_TYPE_AAC_LTP = 4,
	AUDIO_TYPE_SBR = 5,
	AUDIO_TYPE_AAC_SCALABLE = 6,
	AUDIO_TYPE_TWIN_VQ = 7,
	AUDIO_TYPE_CELP = 8,
	AUDIO_TYPE_HXVC = 9,
	AUDIO_TYPE_RESERVED1 = 10,
	AUDIO_TYPE_RESERVED2 = 11,
	AUDIO_TYPE_TTSI = 12,
	AUDIO_TYPE_MAIN_SYNTHESIS = 13,
	AUDIO_TYPE_WAVETABLE_SYNTHESIS = 14,
	AUDIO_TYPE_GENERAL_MIDI = 15,
	AUDIO_TYPE_ASAE = 16,
	AUDIO_TYPE_ER_AAC_LC = 17,
	AUDIO_TYPE_RESERVED3 = 18,
	AUDIO_TYPE_ER_AAC_LTP = 19,
	AUDIO_TYPE_ER_AAC_SCALABLE = 20,
	AUDIO_TYPE_ER_TWIN_VQ = 21,
	AUDIO_TYPE_ER_BSAC = 22,
	AUDIO_TYPE_ER_AAC_LD = 23,
	AUDIO_TYPE_ER_CELP = 24,
	AUDIO_TYPE_ER_HVXC = 25,
	AUDIO_TYPE_ER_HILN = 26,
	AUDIO_TYPE_ER_PARAMETRIC = 27,
	AUDIO_TYPE_SSC = 28,
	AUDIO_TYPE_PS = 29,
	AUDIO_TYPE_MPEG_SURROUND = 30,
	AUDIO_TYPE_ESCAPE_VALUE = 31,
	AUDIO_TYPE_LAYER_1 = 32,
	AUDIO_TYPE_LAYER_2 = 33,
	AUDIO_TYPE_LAYER_3 = 34,
	AUDIO_TYPE_DST = 35,
	AUDIO_TYPE_ALS = 36,
	AUDIO_TYPE_SLS = 37,
	AUDIO_TYPE_SLS_NON_CORE = 38,
	AUDIO_TYPE_ER_AAC_ELD = 39,
	AUDIO_TYPE_SMR = 40,
	AUDIO_TYPE_SMR_MAIN = 41,
	AUDIO_TYPE_USAC_NO_SBR = 42,
	AUDIO_TYPE_SAOC = 43,
	AUDIO_TYPE_LD_MPEG_SURROUND = 44,
	AUDIO_TYPE_USAC = 45,
	AUDIO_TYPE_MAX
};

enum mpeg4_audio_sample_freq_index{
	AUDIO_SAMPLE_FREQ_96000HZ = 0,
	AUDIO_SAMPLE_FREQ_88200HZ = 1,
	AUDIO_SAMPLE_FREQ_64000HZ,
	AUDIO_SAMPLE_FREQ_48000HZ,
	AUDIO_SAMPLE_FREQ_44100HZ,
	AUDIO_SAMPLE_FREQ_32000HZ,
	AUDIO_SAMPLE_FREQ_24000HZ,
	AUDIO_SAMPLE_FREQ_22050HZ,
	AUDIO_SAMPLE_FREQ_16000HZ,
	AUDIO_SAMPLE_FREQ_12000HZ,
	AUDIO_SAMPLE_FREQ_11025HZ,
	AUDIO_SAMPLE_FREQ_8000HZ,
	AUDIO_SAMPLE_FREQ_7350HZ,
	AUDIO_SAMPLE_FREQ_RESERVED1,
	AUDIO_SAMPLE_FREQ_RESERVED2,
	AUDIO_SAMPLE_FREQ_UNKNOWN,
	AUDIO_SAMPLE_FREQ_MAX
};


enum mpeg4_audio_channel_config{
	AUDIO_CHANNEL_CONFIG_AOT = 0,
	AUDIO_CHANNEL_CONFIG_1_FC,
	AUDIO_CHANNEL_CONFIG_2_FL_FR,
	AUDIO_CHANNEL_CONFIG_3_FC_FL_FR,
	AUDIO_CHANNEL_CONFIG_4_FC_FL_FR_BC,
	AUDIO_CHANNEL_CONFIG_5_FC_FL_FR_BL_BR,
	AUDIO_CHANNEL_CONFIG_6_FC_FL_FR_BL_BR_LFE,
	AUDIO_CHANNEL_CONFIG_7_FC_FL_FR_SL_SR_BL_BR_LFE,
	AUDIO_CHANNEL_CONFIG_RESERVED1,
	AUDIO_CHANNEL_CONFIG_RESERVED2,
	AUDIO_CHANNEL_CONFIG_RESERVED3,
	AUDIO_CHANNEL_CONFIG_RESERVED4,
	AUDIO_CHANNEL_CONFIG_RESERVED5,
	AUDIO_CHANNEL_CONFIG_RESERVED6,
	AUDIO_CHANNEL_CONFIG_RESERVED7,
	AUDIO_CHANNEL_CONFIG_RESERVED8,
	AUDIO_CHANNEL_CONFIG_MAX
};

/*
 In an MPEG-4 file, the AAC data is broken up into a series of variable length frames.

An AAC frame is comprised of blocks called syntax elements. Read the first 3 bits from the frame's bitstream to find the first element type. 
Decode the element. Proceed to read the first 3 bits of the next element and repeat the decoding process until the frame is depleted.

There are 8 different syntax elements:

0 SCE single channel element (codes a single audio channel)
1 CPE channel pair element (codes stereo signal)
2 CCE something to do with channel coupling, not implemented in libfaad2
3 LFE low-frequency effects? referenced as "special effects" in RTP doc
4 DSE data stream element (user data)
5 PCE program configuration element (describe bitstream)
6 FIL fill element (pad space/extension data)
7 END marks the end of the frame
This is an example layout for a 5.1 audio stream:

SCE CPE CPE LFE END
indicates

center - left/right - surround left/right - lfe - end

 */
enum aac_element_type{
	AAC_ELEMENT_TYPE_SCE = 0,
	AAC_ELEMENT_TYPE_CPE,
	AAC_ELEMENT_TYPE_CCE,
	AAC_ELEMENT_TYPE_LFE,
	AAC_ELEMENT_TYPE_DSE,
	AAC_ELEMENT_TYPE_PCE,
	AAC_ELEMENT_TYPE_FIL,
	AAC_ELEMENT_TYPE_END,
	AAC_ELEMENT_TYPE_MAX
};

/*
 * Following structure not used, because it is had to use in different platform (Little end, big end)
 * */
typedef struct adts_head_s{
	unsigned int sync:12;
	unsigned int mpeg_version:1;
	unsigned int layer:2;
	unsigned int protection:1;
	unsigned int profile:2;
	unsigned int sample_freq_index:4;
	unsigned int private_stream:1;
	unsigned int channal_conf:3;
	unsigned int originality:1;
	unsigned int home:1;
	unsigned int copyright_stream:1;
	unsigned int copyright_start:1;
	unsigned int frame_len_1:2;
	unsigned int frame_len_2:11;
	unsigned int buffer_fullness:11;
	unsigned int frame_num:2;
	unsigned int element_type1:3;
	unsigned int element_type2:3;
	unsigned int element_type3:2;
}adts_head_t;

void dump_adts_info(uint8_t* buf);

#endif
