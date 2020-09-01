#ifndef SPARSE_CONV_H
#define SPARSE_CONV_H

typedef enum
 {
   SUCCESS = 0,                /**< No error */
   ARGUMENT_ERROR = -1,        /**< One or more arguments are incorrect */
   LENGTH_ERROR = -2,          /**< Length of data buffer is incorrect */
   SIZE_MISMATCH = -3,         /**< Size of matrices is not compatible with the operation. */
   NANINF = -4,                /**< Not-a-number (NaN) or infinity is generated */
   SINGULAR = -5,              /**< Generated by matrix inversion if the input matrix is singular and cannot be inverted. */
   TEST_FAILURE = -6           /**< Test Failed  */
 } status;

#include <ap_axi_sdata.h>
typedef ap_axiu<8,2,5,6> uint8_channel;
typedef ap_axiu<8,2,5,6> int8_channel;

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;

#define dim_kernel 3
#define padding 1
#define stride 1

#define dim_in 32
#define ch_in 3

#define dim_out 32
#define ch_out 32

#define num_PE 9

//#define len(array) sizeof(array) / sizeof(array[0])

#include <hls_video.h>
int sparseConv(hls::stream<int8_channel> &FM_in_stream, hls::stream<int8_channel> &FM_out_stream, hls::stream<int8_channel> &weight_stream, hls::stream<int8_channel> &bias_stream);

class PE {
public:
	int8 weight[ch_in * ch_out][2];
	int8 kernel_point[ch_in + 1];

	PE(){}
	void updateBuffer(int8 val, int8 ch_idx, int8* target_buffer) {
		for(int i = kernel_point[ch_idx]; i < kernel_point[ch_idx + 1]; i++) {
#pragma HLS PIPELINE
			*(target_buffer + weight[i][0]) += weight[i][1] * val;
		}
	}
};

#endif
