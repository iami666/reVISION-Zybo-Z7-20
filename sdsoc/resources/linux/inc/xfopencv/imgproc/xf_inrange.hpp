/***************************************************************************
 Copyright (c) 2016, Xilinx, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ***************************************************************************/
 
#ifndef _XF_INRANGE_HPP_
#define _XF_INRANGE_HPP_

#include "hls_stream.h"
#include "common/xf_common.h"
#include "common/xf_utility.h"


typedef unsigned short  uint16_t;

typedef unsigned int  uint32_t;

namespace xf{

template<int MAXCOLOR>
void apply_threshold(unsigned char low_thresh[MAXCOLOR][3], unsigned char high_thresh[MAXCOLOR][3],ap_uint<8> &outpix,ap_uint<8> &h,ap_uint<8> &s,ap_uint<8> &v)
{
#pragma HLS inline off

	ap_uint<8> tmp_val = 0;


	ap_uint<8> tmp_val1 = 0;


	for(int k=0;k<MAXCOLOR;k++)
	{
		ap_uint<8> t1, t2, t3;
		t1 = 0;
		t2 = 0;
		t3 = 0;

		if((low_thresh[k][0] <= h) && (h <= high_thresh[k][0]))
			t1 = 255;
		if((low_thresh[k][1] <= s) && (s <= high_thresh[k][1]))
			t2 = 255;
		if((low_thresh[k][2] <= v) && (v <= high_thresh[k][2]))
			t3 = 255;

		tmp_val = tmp_val | (t1 & t2 & t3);
	}

	outpix = tmp_val;
}


template<int ROWS, int COLS, int DEPTH_SRC, int DEPTH_DST, int NPC, int WORDWIDTH_SRC, int WORDWIDTH_DST,int MAXCOLOR>
void auInRange(hls::stream< XF_SNAME(WORDWIDTH_SRC) > &in_strm, hls::stream< XF_SNAME(WORDWIDTH_DST) > &out_strm,
		unsigned char low_thresh[MAXCOLOR][3], unsigned char high_thresh[MAXCOLOR][3], uint16_t img_height, uint16_t img_width)
{

	XF_PTNAME(DEPTH_SRC) in_pix;
	XF_PTNAME(DEPTH_DST) out_pix;
	ap_uint<8> h, s, v;

	for(uint16_t row = 0; row < img_height; row++)
	{
#pragma HLS LOOP_TRIPCOUNT max=ROWS
		for(uint16_t col = 0; col < img_width; col++)
		{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT max=COLS

			in_pix = in_strm.read();

			h = in_pix.range(7, 0);
			s = in_pix.range(15, 8);
			v = in_pix.range(23, 16);

			apply_threshold<MAXCOLOR>(low_thresh,high_thresh,out_pix,h,s,v);
			out_strm.write(out_pix);
		}
	}
}




template<int ROWS, int COLS, int DEPTH_SRC, int DEPTH_DST, int NPC, int WORDWIDTH_SRC, int WORDWIDTH_DST, int MAXCOLOR>
void xFCheckRange(hls::stream< XF_SNAME(WORDWIDTH_SRC) > &in_strm, hls::stream< XF_SNAME(WORDWIDTH_DST) > &out_strm,
		unsigned char low_thresh[MAXCOLOR][3], unsigned char high_thresh[MAXCOLOR][3], uint16_t img_height, uint16_t img_width)
{

	auInRange< ROWS, COLS, DEPTH_SRC, DEPTH_DST, NPC, WORDWIDTH_SRC, WORDWIDTH_DST,MAXCOLOR>(in_strm, out_strm, low_thresh, high_thresh, img_height, img_width);
}

#pragma SDS data access_pattern("_src_mat.data":SEQUENTIAL, "_dst_mat.data":SEQUENTIAL)
#pragma SDS data copy("_src_mat.data"[0:"_src_mat.size"], "_dst_mat.data"[0:"_dst_mat.size"])

#pragma SDS data copy(low_thresh[0:9], high_thresh[0:9])

#pragma SDS data mem_attribute("_src_mat.data":NON_CACHEABLE|PHYSICAL_CONTIGUOUS)
#pragma SDS data mem_attribute("_dst_mat.data":NON_CACHEABLE|PHYSICAL_CONTIGUOUS)
template<int SRC_T,int DST_T,int MAXCOLORS, int ROWS, int COLS,int NPC>
void colorthresholding(xf::Mat<SRC_T, ROWS, COLS, NPC> & _src_mat,xf::Mat<DST_T, ROWS, COLS, NPC> & _dst_mat,unsigned char low_thresh[MAXCOLORS*3], unsigned char high_thresh[MAXCOLORS*3])
{



	hls::stream< XF_TNAME(SRC_T,NPC)> _src;
	hls::stream< XF_TNAME(DST_T,NPC)> _dst;
#pragma HLS INLINE OFF
#pragma HLS DATAFLOW

	for(int i=0; i<_src_mat.rows;i++)
	{
#pragma HLS LOOP_TRIPCOUNT min=1 max=ROWS
		for(int j=0; j<(_src_mat.cols)>>(XF_BITSHIFT(NPC));j++)
		{
#pragma HLS LOOP_TRIPCOUNT min=1 max=COLS/NPC
#pragma HLS LOOP_FLATTEN off
#pragma HLS PIPELINE
			_src.write( *(_src_mat.data + i*(_src_mat.cols>>(XF_BITSHIFT(NPC))) +j) );
		}
	}

	unsigned char low_th[MAXCOLORS][3], high_th[MAXCOLORS][3];
#pragma HLS ARRAY_PARTITION variable=low_th dim=1 complete
#pragma HLS ARRAY_PARTITION variable=high_th dim=1 complete
	uint16_t j = 0;
	for(uint16_t i = 0; i < (MAXCOLORS); i++)
	{
#pragma HLS PIPELINE
		low_th[i][0]  = low_thresh[i*j];
		low_th[i][1]  = low_thresh[i*j+1];
		low_th[i][2]  = low_thresh[i*j+2];
		high_th[i][0] = high_thresh[i*j];
		high_th[i][1] = high_thresh[i*j+1];
		high_th[i][2] = high_thresh[i*j+2];
		j = j+3;
	}


	xFCheckRange<ROWS,COLS,XF_DEPTH(SRC_T,NPC),XF_DEPTH(DST_T,NPC),NPC,XF_WORDWIDTH(SRC_T,NPC),XF_WORDWIDTH(DST_T,NPC),MAXCOLORS>(_src,_dst,low_th, high_th,_src_mat.rows,_src_mat.cols);

	for(int i=0; i<_dst_mat.rows;i++)
	{
#pragma HLS LOOP_TRIPCOUNT min=1 max=ROWS
		for(int j=0; j<(_dst_mat.cols)>>(XF_BITSHIFT(NPC));j++)
		{
#pragma HLS LOOP_TRIPCOUNT min=1 max=COLS/NPC
#pragma HLS PIPELINE
#pragma HLS LOOP_FLATTEN off
			*(_dst_mat.data + i*(_dst_mat.cols>>(XF_BITSHIFT(NPC))) +j) = _dst.read();
		}
	}
}
}

#endif
