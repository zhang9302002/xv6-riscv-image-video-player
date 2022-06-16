#include "kernel/types.h"
#include "kernel/sound.h"
#include "user/common.h"
#include "user/user.h"
#include "user/decode.h"

int main(int argc, char**argv)
{
	int decodepid = fork();
	if (decodepid == 0) {
		exec("mp3dec", argv);
	}
	printf("start playing mp3\n");
	setSampleRate(44100);
	Bit_stream_struc  bs;
	struct frame_params fr_ps;
	struct III_side_info_t III_side_info;
	unsigned int old_crc;
	layer info;
	unsigned long bitsPerSlot;

	fr_ps.header = &info;

    open_bit_stream_r(&bs, argv[1], BUFFER_SIZE);
	int frame_Num = 0;
	while(!end_bs(&bs)) {
		seek_sync(&bs, SYNC_WORD, SYNC_WORD_LENGTH);
		decode_info(&bs, &(fr_ps));
		hdr_to_frps(&(fr_ps));
		frame_Num = frame_Num + 1;
		printf("read frame: %d\n", frame_Num);
		if (info.error_protection)
			buffer_CRC(&bs, &old_crc);
        printf("lay=%d\n", info.lay);
		switch (info.lay) {
            case 3: {
                int nSlots, main_data_end, flush_main;
                int bytes_to_discard;
                static int frame_start = 0;

                bitsPerSlot = 8;

                III_get_side_info(&bs, &(III_side_info), &(fr_ps));
                nSlots = main_data_slots(fr_ps);
                for (; nSlots > 0; nSlots--)  /* read main data. */
                    hputbuf((unsigned int) getbits(&bs, 8), 8);
                main_data_end = hsstell() / 8; /*of privious frame*/
                if ((flush_main = (hsstell() % bitsPerSlot)) == TRUE) {
                    hgetbits((int) (bitsPerSlot - flush_main));
                    main_data_end++;
                }
                bytes_to_discard = frame_start - main_data_end - III_side_info.main_data_begin;
                if (main_data_end > 4096) {
                    frame_start -= 4096;
                    rewindNbytes(4096);
                }

                frame_start += main_data_slots(fr_ps);
                //			printf("discard : %d\n", bytes_to_discard);
                if (bytes_to_discard < 0) {
                    printf("discard: %d %d %d\n", frame_start, main_data_end, III_side_info.main_data_begin);
                    //printf("Not enough main data to decode frame %d.  Frame discarded.\n",frame_Num - 1);
                    break;
                }
                for (; bytes_to_discard > 0; bytes_to_discard--) hgetbits(8);
                beginDecode(&fr_ps, &III_side_info);
                break;
            }
            default:
                printf("\nOnly layer III supported!\n");
                exit(0);
                break;
		}
	}
	close_bit_stream_r(&bs);
	printf("\nPlaying done.\n");
	kill(decodepid);
	wait(0);
	exit(0);
}
