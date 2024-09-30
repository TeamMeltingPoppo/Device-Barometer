// use twelite mwx c++ template library
#include <TWELITE>
#include <NWK_SIMPLE>
#include <SensorPacket.h>
#include <TweliteLibrary/dps310/dps310.h>

/*** Config part 
 * Meisterと同様のセンサーネットワークシステムであるため、Meisterと相談して設定すること
*/


// application ID

const uint32_t APP_ID = 0x1234dcba;

// channel
const uint8_t CHANNEL = 13;

// Global Variables

uint32_t timestamp=0;

// #define DEBUG_MODE

/*** setup procedure (run once at cold boot) */
void setup()
{
	/*** SETUP section */

	// the twelite main class
#ifdef DEBUG_MODE
	the_twelite
		<< TWENET::appid(APP_ID)	// set application ID (identify network group)
		<< TWENET::channel(CHANNEL) // set channel (pysical channel)
		<< TWENET::rx_when_idle();	// open receive circuit (if not set, it can't listen packts from others)

	auto &&nwksmpl = the_twelite.network.use<NWK_SIMPLE>();
	nwksmpl << NWK_SIMPLE::logical_id(DeviceData::DeviceID::MainBoard | 0x01) // set Logical ID. (0x00 means a parent device)
			<< NWK_SIMPLE::dup_check(16, 50, 5)								  // set
			<< NWK_SIMPLE::repeat_max(0);									  // can repeat a packet up to three times. (being kind of a router)
	the_twelite.begin();													  // start twelite!
#else
	the_twelite
		<< TWENET::appid(APP_ID)	 // set application ID (identify network group)
		<< TWENET::channel(CHANNEL); // set channel (pysical channel)

	auto &&nwksmpl = the_twelite.network.use<NWK_SIMPLE>();
	nwksmpl << NWK_SIMPLE::logical_id(DeviceData::DeviceID::Barometer | 0x01) // set Logical ID. (0x00 means a parent device)
			<< NWK_SIMPLE::repeat_max(0);									  // can repeat a packet up to three times. (being kind of a router)
	the_twelite.begin();													  // start twelite!
#endif

	// Serial.begin(9600); // defaultでは115200bpsで通信
	Wire.begin();
	delay(1000);
	DPS310::init();
}

/*** loop procedure (called every event) */
void loop()
{
	// loopはすぐに抜けること
	if (timestamp < millis())
	{
		timestamp = millis() + 100;
		union
		{
			DeviceData::BarometerData data;
			uint8_t bytes[sizeof(data)];
		} spkt;
		spkt.data.id = DeviceData::DeviceID::Barometer;
		spkt.data.timestamp = millis();
		spkt.data.temperature = (float)DPS310::readTemp();
		spkt.data.pressure = (float)DPS310::readPressure();
		if (auto &&pkt = the_twelite.network.use<NWK_SIMPLE>().prepare_tx_packet())
		{
			// set tx packet behavior
			pkt << tx_addr(0xFF)				// 0..0xFF (LID 0:parent, FE:child w/ no id, FF:LID broad cast), 0x8XXXXXXX (long address)
				<< tx_retry(0x0)				// set retry (0x3 send four times in total)
				<< tx_packet_delay(0, 100, 30); // send packet w/ delay (send first packet with randomized delay from 100 to 200ms, repeat every 20ms)

			pack_bytes(
				pkt.get_payload(), // set payload data objects.
				make_pair(spkt.bytes, (int)sizeof(spkt)));

			pkt.transmit(); // do transmit!
		}else{
			Serial << "failed to prepare tx packet" << mwx::crlf;
		}
	}
}

#ifdef DEBUG_MODE

void on_rx_packet(packet_rx &rx, bool_t &handled)
{
	auto pkt = rx.get_payload();

	// 上位4bitで種類を識別、下位4bitで基板を識別
	if ((pkt[0] & 0xF0) == DeviceData::DeviceID::Barometer)
	{
		union
		{
			DeviceData::BarometerData data;
			uint8_t bytes[sizeof(data)];
		} spkt;
		expand_bytes(pkt.begin(), pkt.end(), spkt.bytes);
		Serial << format("pressure: %f,%d", spkt.data.pressure, spkt.data.timestamp) << mwx::crlf;
	}

	// uint8_t cobs_buf[256];
	// auto len = rx.get_length();
	// uint8_t cobs_buf_idx = 0;

	// for (uint8_t i = 0; i < len; i++)
	// {
	// 	if (pkt[i] == 0x00)
	// 	{
	// 		Serial1.write(cobs_buf_idx + 1);
	// 		Serial.write(cobs_buf_idx + 1);
	// 		for (uint8_t j = 0; j < cobs_buf_idx; j++)
	// 		{
	// 			Serial1.write(cobs_buf[j]);
	// 			Serial.write(cobs_buf[j]);
	// 		}
	// 		cobs_buf_idx = 0;
	// 	}
	// 	else
	// 	{
	// 		cobs_buf[cobs_buf_idx] = pkt[i];
	// 		cobs_buf_idx++;
	// 	}
	// }
	// Serial1.write(cobs_buf_idx + 1);
	// Serial.write(cobs_buf_idx + 1);
	// for (uint8_t j = 0; j < cobs_buf_idx; j++)
	// {
	// 	Serial1.write(cobs_buf[j]);
	// 	Serial.write(cobs_buf[j]);
	// }
	// Serial1.write(0x00);
	// Serial.write(0x00);
}

#endif

/* Copyright (C) 2019-2021 Mono Wireless Inc. All Rights Reserved. *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE     *
 * AGREEMENT).                                                     */