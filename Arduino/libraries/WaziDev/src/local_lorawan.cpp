/*
 *  simple lorawan lib to encode and decode lorawan packet
 *  
 *  Copyright (C) 2016-2020 Congduc Pham, University of Pau, France
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include "local_lorawan.h"
#include "Utils.h"

///////////////////////////////////////////////////////////////////
// DO NOT CHANGE HERE
uint16_t Frame_Counter_Up = 0x0000;
// 0 for uplink, 1 for downlink - See LoRaWAN specifications
unsigned char Direction = 0x00;
///////////////////////////////////////////////////////////////////


// Create a LoRaWAN packet meaning that the payload is encrypted
uint8_t local_aes_lorawan_create_pkt(uint8_t *pl, uint8_t len, uint8_t app_key_offset)
{
  Encrypt_Payload((unsigned char *)pl, len, Frame_Counter_Up, Direction);

  // with only AES encryption, we still use a LoRaWAN packet format to reuse available LoRaWAN encryption libraries
  //
  unsigned char LORAWAN_Data[80];
  unsigned char LORAWAN_Package_Length;
  unsigned char MIC[4];
  //Unconfirmed data up
  unsigned char Mac_Header = 0x40;
  // no ADR, not an ACK and no options
  unsigned char Frame_Control = 0x00;
  // with application data so Frame_Port = 1..223
  unsigned char Frame_Port = 0x01;

  //Build the Radio Package, LoRaWAN format
  //See LoRaWAN specification
  LORAWAN_Data[OFF_DAT_HDR] = Mac_Header;

  LORAWAN_Data[OFF_DAT_ADDR] = DevAddr[3];
  LORAWAN_Data[OFF_DAT_ADDR + 1] = DevAddr[2];
  LORAWAN_Data[OFF_DAT_ADDR + 2] = DevAddr[1];
  LORAWAN_Data[OFF_DAT_ADDR + 3] = DevAddr[0];

  LORAWAN_Data[OFF_DAT_FCT] = Frame_Control;

  LORAWAN_Data[OFF_DAT_SEQNO] = (Frame_Counter_Up & 0x00FF);
  LORAWAN_Data[OFF_DAT_SEQNO + 1] = ((Frame_Counter_Up >> 8) & 0x00FF);

  LORAWAN_Data[OFF_DAT_OPTS] = Frame_Port;

  //Set Current package length
  LORAWAN_Package_Length = OFF_DAT_OPTS + 1;

  //Load Data
  for (int i = 0; i < len; i++)
  {
    // see that we don't take the appkey, just the encrypted data that starts at pl[app_key_offset]
    LORAWAN_Data[LORAWAN_Package_Length + i] = pl[i];
  }

  //Add data Lenth to package length
  LORAWAN_Package_Length = LORAWAN_Package_Length + len;

  //Calculate MIC
  Calculate_MIC(LORAWAN_Data, MIC, LORAWAN_Package_Length, Frame_Counter_Up, Direction);

  //Load MIC in package
  for (int i = 0; i < 4; i++)
  {
    LORAWAN_Data[i + LORAWAN_Package_Length] = MIC[i];
  }

  //Add MIC length to package length
  LORAWAN_Package_Length = LORAWAN_Package_Length + 4;

  // copy back to pl
  memcpy(pl, LORAWAN_Data, LORAWAN_Package_Length);
  len = LORAWAN_Package_Length;

  // in any case, we increment Frame_Counter_Up
  // even if the transmission will not succeed
  Frame_Counter_Up++;

  return len;
}

///////////////////////////////////////////////////////////////////


#define ERR_LORAWAN_MIC -1
#define ERR_LORAWAN_PAYLOAD -3
#define ERR_LORA_CRC 1
#define ERR_LORA_TIMEOUT 2

// Decode a LoRaWAN downlink packet and check the MIC.
// It returns the offset (>0) of the payload data withing the payload (skipping the LoRaWAN header).
// It returns an error <0, if any.
int8_t local_lorawan_decode_pkt(uint8_t *pl, uint8_t len)
{
  if (len < 9)
  {
    // minimum LoRaWAN frame length (headers+mic) not reached
    return ERR_LORAWAN_PAYLOAD;
  }

  unsigned char MIC[4]; // media integrity code

  // get FCntDn from LoRaWAN downlink packet
  uint16_t seqno = (uint16_t)((uint16_t)pl[OFF_DAT_SEQNO] | ((uint16_t)pl[OFF_DAT_SEQNO + 1] << 8));

  // end of payload, removing the 4-byte MIC
  uint8_t end = len - 4;

  Calculate_MIC(pl, MIC, end, seqno, 1);

  if ((uint8_t)MIC[3] != pl[len - 1] ||
    (uint8_t)MIC[2] != pl[len - 2] ||
    (uint8_t)MIC[1] != pl[len - 3] ||
    (uint8_t)MIC[0] != pl[len - 4])
  {
    return ERR_LORAWAN_MIC;
  }

  // start of payload
  uint8_t offs = OFF_DAT_OPTS + 1 + (uint8_t)(pl[OFF_DAT_FCT] & FCT_OPTLEN);

  // length of payload without LoRaWAN headers
  int16_t l = end - offs;

  if (l > 0)
  {
    // length is 0 for LoRaWAN frames with control-data only (and no payload)
    Encrypt_Payload((unsigned char *)(pl + offs), l, seqno, 1);
    return offs;
  }
  return end;
}
