#include "sparseConv.h"

int sparseConv(hls::stream<int8_channel> &FM_in_stream, hls::stream<int8_channel> &FM_out_stream, hls::stream<int8_channel> &weight_stream, hls::stream<int8_channel> &bias_stream){

#pragma HLS INTERFACE axis port=FM_in_stream
#pragma HLS INTERFACE axis port=FM_out_stream
#pragma HLS INTERFACE axis port=weight_stream
#pragma HLS INTERFACE axis port=bias_stream
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS

	PE pe[num_PE];

	int8_channel weight_channel;
	int8_channel FM_in_channel;
	int8_channel FM_out_channel;

	//load weight
	for(int i = 0; i < num_PE; i++) {
		int idx = 0;
		for(int j = 0; j < ch_in; j++) {
			pe[i].kernel_point[j] = idx;
			weight_channel = weight_stream.read();
			for(int k = 0; k < weight_channel.data; k++) {
				pe[i].weight[idx][0] = weight_stream.read().data;
				pe[i].weight[idx][1] = weight_stream.read().data;
				idx++;
			}
		}
		pe[i].kernel_point[ch_in] = idx;
	}

	//update FM_out
	int8 out_buffer[dim_out][dim_out][ch_out];
	//memset(out_buffer, 0, sizeof(out_buffer)); //memset 不可以综合
	int idx = 0;
	while(!FM_in_stream.empty()) {
		FM_in_channel = FM_in_stream.read();
		uint8 flag = 0x80;
		int8 sm = FM_in_channel.data;
		while(flag >= 1) {
			if((sm & flag) != 0) {
				int8 data = FM_in_stream.read().data;
				for(int i = 0; i < num_PE; i++) {
#pragma HLS PIPELINE
					int rowIdx = (idx / ch_in) / dim_out + (1 - i / 3);
					int colIdx = (idx / ch_in) % dim_out + (1 - i % 3);
					if(rowIdx >= 0 && rowIdx <= dim_out && colIdx >= 0 && colIdx <= dim_out) {
						pe[i].updateBuffer(data, idx % ch_in, out_buffer[rowIdx][colIdx]);//随便写的
					}
				}
			}
			flag >>= 1;
			idx++;
		}
	}

	for(int i = 0; i < dim_out * dim_out; i++) {
		for(int j = 0; j < ch_out; j++) {
			if(out_buffer[i][j] != 0) {
				FM_out_channel.data = 8;
				FM_out_channel.keep = FM_in_channel.keep;
				FM_out_channel.strb = FM_in_channel.strb;
				FM_out_channel.user = FM_in_channel.user;
				FM_out_channel.id = FM_in_channel.id;
				FM_out_channel.dest = FM_in_channel.dest;
				if(FM_in_stream.empty()){
					FM_out_channel.last = 1;
				}else{
					FM_out_channel.last = 0;
				}
				// Send to the stream (Block if the FIFO receiver is full)
				FM_out_stream.write(FM_out_channel);
			}
		}
	}

	return SUCCESS;
}

