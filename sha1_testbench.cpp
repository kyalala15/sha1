/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include <iomanip>
using namespace std;

#define INPUT_SIZE 10

void sha1(hls::stream<ap_axis<32,2,5,6> > &A, hls::stream<ap_axis<32,2,5,6> > &B);

int main()
{
  hls::stream<ap_axis<32,2,5,6> > A, B;
  ap_axis<32,2,5,6> tmp1, tmp2;

  int input_array[INPUT_SIZE] = {104,101,108,108,111,104,101,108,108,111};


 for(int j = 0; j < INPUT_SIZE; j++) {
  tmp1.data = input_array[j];
  tmp1.keep = 1;
  tmp1.strb = 1;
  tmp1.user = 1;
  if(j == INPUT_SIZE - 1) {
	  tmp1.last = 1;
  }
  else {
	  tmp1.last = 0;
  }
  tmp1.id = 0;
  tmp1.dest = 1;
  A.write(tmp1);
 }

 sha1(A,B);

 printf("Result: 0x");
 for(int x = 0; x < 5; x++) {
   B.read(tmp2);
   printf("%2.2x", tmp2.data);
 }
 printf("\nExpected value: 0xaaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d");
 return 0;

}
