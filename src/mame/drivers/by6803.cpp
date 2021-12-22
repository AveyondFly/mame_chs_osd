// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************************

PINBALL
Bally MPU A084-91786-AH06 (6803)

The characteristic 6803 pinball has 4x 7-digit displays lined up along the bottom of the
 backbox. They show scores, moving messages, and anything else that's needed.

Schematic has a number of mistakes.

Status:
- 2nd gen games boot up and display, some can coin up, but cannot start due to balls missing

ToDo:
- Outputs
- 1st gen display
- Commas are missing from the display.
- Code has provision for a round LED, to be added to layout.
- Operator keypad
- Various sound boards
- Inputs, Solenoids vary per game

*********************************************************************************************/


#include "emu.h"
//#include "audio/midway.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6801.h"
//#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/timer.h"

#include "by6803.lh"


namespace {

class by6803_state : public genpin_class
{
public:
	by6803_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia0(*this, "pia0")
		, m_pia1(*this, "pia1")
		, m_io_test(*this, "TEST")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_leds(*this, "led%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void by6803(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(activity_test);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u8 port1_r();
	void port1_w(u8 data);
	u8 port2_r();
	void port2_w(u8 data);
	u8 pia0_a_r();
	void pia0_a_w(u8 data);
	u8 pia0_b_r();
	void pia0_b_w(u8 data);
	u8 pia1_a_r();
	void pia1_a_w(u8 data);
	void pia1_b_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(pia0_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia0_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(pia1_cb2_w);
	TIMER_DEVICE_CALLBACK_MEMBER(pia0_timer);

	void by6803_map(address_map &map);

	u8 m_pia0_a = 0;
	u8 m_pia0_b = 0;
	u8 m_pia1_a = 0;
	u8 m_pia1_b = 0;
	bool m_pia0_cb2 = 0;
	bool m_pia0_timer = 0;
	u8 m_port1, m_port2 = 0;
	u8 m_digit = 0;
	u8 m_segment[2]{};
	required_device<m6803_cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_ioport m_io_test;
	required_ioport_array<6> m_io_keyboard;
	output_finder<40> m_digits;
	output_finder<1> m_leds;
	output_finder<112> m_io_outputs; // 16 solenoids + 96 lamps
};


void by6803_state::by6803_map(address_map &map)
{
	map(0x0020, 0x0023).rw(m_pia0, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0040, 0x0043).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x17ff).ram().share("nvram"); // 6116 ram
	map(0x8000, 0xffff).rom();
}

static INPUT_PORTS_START( by6803 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by6803_state, self_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by6803_state, activity_test, 0)

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP01")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP02")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP03")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP04")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP05")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP07")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP16")

	// from here, vary per game
	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP48")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( by6803_state::activity_test )
{
	if(newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( by6803_state::self_test )
{
	m_pia0->ca1_w(newval);
}

u8 by6803_state::port1_r()
{
	return m_port1;
}

// P10-17 - goes to peripheral bus
void by6803_state::port1_w(u8 data)
{
	m_port1 = data; // sound data = P10,11,12,13,24; P14-17 unknown
}

u8 by6803_state::port2_r()
{
	return m_port2;
}

// P20 - input from a phase
// P21 - output to phase circuit
// P22 - LED, connects to reset circuit, could be a watchdog
// P23 - high
// P24 - sound strobe
void by6803_state::port2_w(u8 data)
{
	m_port2 = data;
	m_leds[0] = BIT(data, 2); // P22 drives LED
}

// display latch strobes; display blanking
WRITE_LINE_MEMBER( by6803_state::pia0_ca2_w )
{
	//printf("CA02=%X ",state);

	if (state)
	{
		// comma info is available in m_pia1_a at this time.
		m_digits[m_digit] = m_segment[0];
		m_digits[m_digit+16] = m_segment[1];
	}
}

// lamp strobe 1 when high
WRITE_LINE_MEMBER( by6803_state::pia0_cb2_w )
{
	//printf("CB02=%X ",state);
}

// sol bank select (0 to enable sol selection)
WRITE_LINE_MEMBER( by6803_state::pia1_cb2_w )
{
	//printf("CB12=%X ",state);
}

u8 by6803_state::pia0_a_r()
{
	return m_pia0_a;
}

// d0=p1,2   d1=p3,4   d2=?   d3=?  (active low, also pia0:ca2 must be low)
// d4-7 do digit select; d0-4 switch matrix
// d0-3 lamp rows & d5=0 & pia0:cb2=1 (1st lamp bank)
// d0-3 lamp rows & d6=0 & pia0:cb2=1 (2nd lamp bank)
// d0-3 lamp rows & d7=0 & pia0:cb2=1 (3rd lamp bank)
void by6803_state::pia0_a_w(u8 data)
{
	m_pia0_a = data;
	if ((data & 15)==14)
	{
		m_digit = data >> 4;
		m_segment[0] = bitswap<8>(m_pia1_a, 0, 7, 6, 5, 4, 3, 2, 1) ^ 0x80;
	}
	else
	if ((data & 15)==13)
	{
		m_segment[1] = bitswap<8>(m_pia1_a, 0, 7, 6, 5, 4, 3, 2, 1) ^ 0x80;
	}
}

// switch returns
u8 by6803_state::pia0_b_r()
{
	u8 data = 0;

	for (u8 i = 0; i < 6; i++)
		if (BIT(m_pia0_a, i))
			data |= m_io_keyboard[i]->read();

	return data;
}

void by6803_state::pia0_b_w(u8 data)
{
	m_pia0_b = data;
}

u8 by6803_state::pia1_a_r()
{
	return m_pia1_a;
}

// segment data; d0 & pia0:ca2 = comma; passed to digits when PA0? is high (assume they mean pia0:pa0)
void by6803_state::pia1_a_w(u8 data)
{
	m_pia1_a = data;
}

// solenoids, activated when pia1:cb2 is low
void by6803_state::pia1_b_w(u8 data)
{
	m_pia1_b = data;
	switch (data & 15)
	{
		case 0x0: //
			//m_samples->start(0, 3);
			break;
		case 0x1: // chime 10
			//m_samples->start(1, 1);
			break;
		case 0x2: // chime 100
			//m_samples->start(2, 2);
			break;
		case 0x3: // chime 1000
			//m_samples->start(3, 3);
			break;
		case 0x4: // chime 10000
			//m_samples->start(0, 4);
			break;
		case 0x5: // knocker
			//m_samples->start(0, 6);
			break;
		case 0x6: // outhole
			//m_samples->start(0, 5);
			break;
		// from here, vary per game
		case 0x7:
		case 0x8:
		case 0x9:
			//m_samples->start(0, 5);
			break;
		case 0xa:
			//m_samples->start(0, 5);
			break;
		case 0xb:
			//m_samples->start(0, 0);
			break;
		case 0xc:
			//m_samples->start(0, 5);
			break;
		case 0xd:
			//m_samples->start(0, 0);
			break;
		case 0xe:
			//m_samples->start(0, 5);
			break;
		case 0xf: // not used
			break;
	}
}

void by6803_state::machine_start()
{
	genpin_class::machine_start();
	m_io_outputs.resolve();
	m_digits.resolve();
	m_leds.resolve();

	save_item(NAME(m_pia0_a));
	save_item(NAME(m_pia0_b));
	save_item(NAME(m_pia1_a));
	save_item(NAME(m_pia1_b));
	save_item(NAME(m_pia0_cb2));
	save_item(NAME(m_pia0_timer));
	save_item(NAME(m_port1));
	save_item(NAME(m_port2));
	save_item(NAME(m_digit));
	save_item(NAME(m_segment));
}

void by6803_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_pia0_a = 0;
	m_pia0_b = 0;
	m_pia0_cb2 = 0;
	m_pia1_a = 0;
	m_pia1_b = 0;
	m_port2 = 2+8;
}

// zero-cross detection
TIMER_DEVICE_CALLBACK_MEMBER( by6803_state::pia0_timer )
{
	// Phase A
	if ((m_pia0_timer) && (!BIT(m_port2, 1)))
		m_port2 |= 1; // set P20 high
	else
		m_port2 &= ~1;

	// Is this the correct thing to do? No other driver uses this line.
	// What polarity is it? I'm assuming that irq asserted = 1.
	m_maincpu->set_input_line(M6801_TIN_LINE, BIT(m_port2, 0));

	m_pia0_timer ^= 1;

	// Phase B
	m_pia0->cb1_w(m_pia0_timer);
}

void by6803_state::by6803(machine_config &config)
{
	/* basic machine hardware */
	M6803(config, m_maincpu, XTAL(3'579'545));
	m_maincpu->set_addrmap(AS_PROGRAM, &by6803_state::by6803_map);
	m_maincpu->in_p1_cb().set(FUNC(by6803_state::port1_r)); // P10-P17
	m_maincpu->out_p1_cb().set(FUNC(by6803_state::port1_w)); // P10-P17
	m_maincpu->in_p2_cb().set(FUNC(by6803_state::port2_r)); // P20-P24
	m_maincpu->out_p2_cb().set(FUNC(by6803_state::port2_w)); // P20-P24

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_by6803);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia0, 0);
	m_pia0->readpa_handler().set(FUNC(by6803_state::pia0_a_r));
	m_pia0->writepa_handler().set(FUNC(by6803_state::pia0_a_w));
	m_pia0->readpb_handler().set(FUNC(by6803_state::pia0_b_r));
	m_pia0->writepb_handler().set(FUNC(by6803_state::pia0_b_w));
	m_pia0->ca2_handler().set(FUNC(by6803_state::pia0_ca2_w));
	m_pia0->cb2_handler().set(FUNC(by6803_state::pia0_cb2_w));
	m_pia0->irqa_handler().set_inputline("maincpu", M6803_IRQ_LINE);
	m_pia0->irqb_handler().set_inputline("maincpu", M6803_IRQ_LINE);
	TIMER(config, "timer_z").configure_periodic(FUNC(by6803_state::pia0_timer), attotime::from_hz(120)); // mains freq*2

	PIA6821(config, m_pia1, 0);
	m_pia1->readpa_handler().set(FUNC(by6803_state::pia1_a_r));
	m_pia1->writepa_handler().set(FUNC(by6803_state::pia1_a_w));
	m_pia1->writepb_handler().set(FUNC(by6803_state::pia1_b_w));
	m_pia1->cb2_handler().set(FUNC(by6803_state::pia1_cb2_w));

	//SPEAKER(config, "mono").front_center();
	//MIDWAY_TURBO_CHEAP_SQUEAK(config, "tcs").add_route(ALL_OUTPUTS, "speaker", 1.0); // Cheap Squeak Turbo
}


/*-----------------------------------------------------------
/ Atlantis #2006
/-----------------------------------------------------------*/
ROM_START(atlantip)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u26_cpu.rom", 0x8000, 0x4000, CRC(b98491e1) SHA1(b867e2b24e93c4ee19169fe93c0ebfe0c1e2fc25))
	ROM_LOAD( "u27_cpu.rom", 0xc000, 0x4000, CRC(8ea2b4db) SHA1(df55a9fb70d1cabad51dc2b089af7904a823e1d8))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("u4_snd.rom", 0x00000, 0x8000, CRC(6a48b588) SHA1(c58dbfd920c279d7b9d2de8558d73c687b29ce9c))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
	ROM_LOAD("u19_snd.rom", 0x10000, 0x8000, CRC(1387467c) SHA1(8b3dd6c2fc94cfebc1879795532c651cda202846))
	ROM_RELOAD(0x10000+0x8000, 0x8000)
	ROM_LOAD("u20_snd.rom", 0x20000, 0x8000, CRC(d5a6a773) SHA1(30807e03655d2249c801007350bfb228a2e8a0a4))
	ROM_RELOAD(0x20000+0x8000, 0x8000)
ROM_END

/*------------------------------------
/ Beat the Clock #0C70
/------------------------------------*/
ROM_START(beatclck)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "btc_u3.cpu", 0xc000, 0x4000, CRC(9ba822ab) SHA1(f28d38411df3978bcaf24177fa1b47037a586cbb))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("btc_u2.snd", 0xc000, 0x1000, CRC(fd22fd2a) SHA1(efad3b94e91d07930ada5366d389f35377dfbd99))
	ROM_LOAD("btc_u3.snd", 0xd000, 0x1000, CRC(22311a4a) SHA1(2c22ba9228e44e68b9308b3bf8803edcd70fa5b9))
	ROM_LOAD("btc_u4.snd", 0xe000, 0x1000, CRC(af1cf23b) SHA1(ebfa3afafd7850dfa2664d3c640fbfa631012455))
	ROM_LOAD("btc_u5.snd", 0xf000, 0x1000, CRC(230cf329) SHA1(45b17a785b81cd5b1d7fdfb720cf1990994b52b7))
ROM_END

ROM_START(beatclck2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "btc_lights_pro_111385_c70-803-05_u3.cpu", 0xc000, 0x4000, CRC(dff5bad6) SHA1(915495d60be7ca12f00364b6e4b99c822ecfc7aa))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("btc_u2.snd", 0xc000, 0x1000, CRC(fd22fd2a) SHA1(efad3b94e91d07930ada5366d389f35377dfbd99))
	ROM_LOAD("btc_u3.snd", 0xd000, 0x1000, CRC(22311a4a) SHA1(2c22ba9228e44e68b9308b3bf8803edcd70fa5b9))
	ROM_LOAD("btc_u4.snd", 0xe000, 0x1000, CRC(af1cf23b) SHA1(ebfa3afafd7850dfa2664d3c640fbfa631012455))
	ROM_LOAD("btc_u5.snd", 0xf000, 0x1000, CRC(230cf329) SHA1(45b17a785b81cd5b1d7fdfb720cf1990994b52b7))
ROM_END

/*------------------------------------
/ Karate Fight
/------------------------------------*/

/*------------------------------------
/ Black Belt #0E52
/------------------------------------*/
ROM_START(blackblt)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.cpu", 0x8000, 0x4000, CRC(7c771910) SHA1(1df8ae478c3626a5200215bfca557ca42e064d2b))
	ROM_LOAD( "u3.cpu", 0xc000, 0x4000, CRC(bad0f4c3) SHA1(5e5240fda9f7f7f15f1953f12b132ba1c4fc886e))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("blck_u7.snd", 0x8000, 0x8000, CRC(db8bce07) SHA1(6327cfbb2761f4d190e2852f3321cdd0cc1e46a8))
ROM_END

ROM_START(blackblt2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.cpu", 0x8000, 0x4000, CRC(b86d16ec) SHA1(2e4601e725261aca67e4d706f310b14eb7578d8b))
	ROM_LOAD( "cpu_u3.cpu", 0xc000, 0x4000, CRC(c63e3e6f) SHA1(cd3f66c3796eaf64c36cabba9d74cc8c690d9d8b))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("blb_u2.snd", 0xc000, 0x1000, NO_DUMP)
	ROM_LOAD("blb_u3.snd", 0xd000, 0x1000, NO_DUMP)
	ROM_LOAD("blb_u4.snd", 0xe000, 0x1000, NO_DUMP)
	ROM_LOAD("blb_u5.snd", 0xf000, 0x1000, NO_DUMP)
ROM_END

/*------------------------------------
/ Blackwater 100 #0H07
/------------------------------------*/
ROM_START(black100)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.cpu", 0x8000, 0x4000, CRC(411fa773) SHA1(9756c7eee0f78792823a0b0379d2baac28cb03e8))
	ROM_LOAD( "u3.cpu", 0xc000, 0x4000, CRC(d6f6f890) SHA1(8fe4dae471f4c89f2fd72c6e647ead5206881c63))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.bin", 0x00001, 0x10000, CRC(a0ecb282) SHA1(4655e0b85f7e8af8dda853279696718d3adbf7e3))
	ROM_LOAD16_BYTE("u11.bin", 0x00000, 0x10000, CRC(3f117ba3) SHA1(b4cded8fdd90ca030c6ff12c817701402c94baba))
	ROM_LOAD16_BYTE("u14.bin", 0x20001, 0x10000, CRC(b45bf5c4) SHA1(396ddf346e8ebd8cb91777521d93564d029f40b1))
	ROM_LOAD16_BYTE("u13.bin", 0x20000, 0x10000, CRC(f5890443) SHA1(77cd18cf5541ae9f7e2dd1c060a9bf29b242d05d))
ROM_END

ROM_START(black100s)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "sb2.cpu", 0x8000, 0x4000, CRC(b6fdbb0f) SHA1(5b36a725db3a1e023bbb54b8f85300fe99174b6e))
	ROM_LOAD( "sb3.cpu", 0xc000, 0x4000, CRC(ae9930b8) SHA1(1b6c63ce98939ecded300639d872df62548157a4))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.bin", 0x00001, 0x10000, CRC(a0ecb282) SHA1(4655e0b85f7e8af8dda853279696718d3adbf7e3))
	ROM_LOAD16_BYTE("u11.bin", 0x00000, 0x10000, CRC(3f117ba3) SHA1(b4cded8fdd90ca030c6ff12c817701402c94baba))
	ROM_LOAD16_BYTE("u14.bin", 0x20001, 0x10000, CRC(b45bf5c4) SHA1(396ddf346e8ebd8cb91777521d93564d029f40b1))
	ROM_LOAD16_BYTE("u13.bin", 0x20000, 0x10000, CRC(f5890443) SHA1(77cd18cf5541ae9f7e2dd1c060a9bf29b242d05d))
ROM_END

/*------------------------------------
/ City Slicker #0E79
/------------------------------------*/
ROM_START(cityslck)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.128", 0x8000, 0x4000, CRC(94bcf162) SHA1(1d83592ad2441fc5e4c6fd3ab2373614dfe78b34))
	ROM_LOAD( "u3.128", 0xc000, 0x4000, CRC(97cb2bca) SHA1(0cbd49bbce2ce26c720d8a52bd4d1256f0ac61b3))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u7_snd.512", 0x0000, 0x10000, CRC(6941d68a) SHA1(28de4327f328d16ec4cab59642c185777535efb2))
ROM_END

/*------------------------------------
/ Dungeons & Dragons #0H06
/------------------------------------*/
ROM_START(dungdrag)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(cefd4330) SHA1(0bffb2b73229e9908a018e06daeceb736896e5f0))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(4bacc7f5) SHA1(71dd898924e0e968c4f3ba8a261e6b382d8ae0f1))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("snd_u12.512", 0x00001, 0x10000, CRC(dd95f851) SHA1(6fa46b512bced0d1862b2621e195ef0dfd24f928))
	ROM_LOAD16_BYTE("snd_u11.512", 0x00000, 0x10000, CRC(dcd461b3) SHA1(834000cfb6c6acf5c296db58971251819971f4de))
	ROM_LOAD16_BYTE("snd_u14.512", 0x20001, 0x10000, CRC(dd9e61eb) SHA1(fd1ec58f5708d5abf3d7424954ce054454514283))
	ROM_LOAD16_BYTE("snd_u13.512", 0x20000, 0x10000, CRC(1e2d9211) SHA1(f5fcf1c07f01e7f1a7abff9ac3c481b84471d3a6))
ROM_END

/*------------------------------------
/ Eight Ball Champ #0B38
/------------------------------------*/
ROM_START(eballchp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u3_cpu.128", 0xc000, 0x4000, CRC(025f3008) SHA1(25d310f169b92ce6b348330816ddc3b5710e57da))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u3_snd.532", 0xd000, 0x1000, CRC(4836d70d) SHA1(a4acc64609d91a84ba4c8101186d07397b496600))
	ROM_LOAD("u4_snd.532", 0xe000, 0x1000, CRC(4b49d94d) SHA1(52d5f4b7604601cd86f0e80ed7c4fe09d14f5976))
	ROM_LOAD("u5_snd.532", 0xf000, 0x1000, CRC(655441df) SHA1(9da5578856ded3dcdafed67679eb4c4134dc9f81))
ROM_END

/*------------------------------------------------
/ Escape from the Lost World #0H05
/-----------------------------------------------*/
ROM_START(esclwrld)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.128", 0x8000, 0x4000, CRC(b11a97ea) SHA1(29339785a67ed7dc9eb39ddc7bb7e6baaf731210))
	ROM_LOAD( "u3.128", 0xc000, 0x4000, CRC(5385a562) SHA1(a6c39532d01db556e4bdf90020a9d9905238e8ef))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.512", 0x00001, 0x10000, CRC(0c003473) SHA1(8ada2aa546a6499c5e2b5eb45a1975b8285d25f9))
	ROM_LOAD16_BYTE("u11.512", 0x00000, 0x10000, CRC(360f6658) SHA1(c0346952dcd33bbcf4c43c51cde5433a099a7a5d))
	ROM_LOAD16_BYTE("u14.512", 0x20001, 0x10000, CRC(0b92afff) SHA1(78f51989e74ced9e0a81c4e18d5abad71de01faf))
	ROM_LOAD16_BYTE("u13.512", 0x20000, 0x10000, CRC(b056842e) SHA1(7c67e5d69235a784b9c38cb31302d206278a3814))
ROM_END

ROM_START(esclwrldg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_ger.128", 0x8000, 0x4000, CRC(0a6ab137) SHA1(0627b7c67d13f305f2287f3cfa023c8dd7721250))
	ROM_LOAD( "u3_ger.128", 0xc000, 0x4000, CRC(26d8bfbb) SHA1(3b81fb0e736d14004bbbbb2edd682fdfc1b2c832))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.512", 0x00001, 0x10000, CRC(0c003473) SHA1(8ada2aa546a6499c5e2b5eb45a1975b8285d25f9))
	ROM_LOAD16_BYTE("u11.512", 0x00000, 0x10000, CRC(360f6658) SHA1(c0346952dcd33bbcf4c43c51cde5433a099a7a5d))
	ROM_LOAD16_BYTE("u14.512", 0x20001, 0x10000, CRC(0b92afff) SHA1(78f51989e74ced9e0a81c4e18d5abad71de01faf))
	ROM_LOAD16_BYTE("u13.512", 0x20000, 0x10000, CRC(b056842e) SHA1(7c67e5d69235a784b9c38cb31302d206278a3814))
ROM_END
/*------------------------------------
/ Hardbody #0E94
/------------------------------------*/
ROM_START(hardbody)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "hb_cpu2.bin", 0x8000, 0x4000, CRC(03975ea9) SHA1(8a4ba6bb5e6ab8da5fffaead283e26edd297e637))
	ROM_LOAD( "hb_cpu3.bin", 0xc000, 0x4000, CRC(10c10380) SHA1(98207c16b6d2a9990eb36b2629bfd668e45ca58e))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.512", 0x0000, 0x10000, CRC(c96f91af) SHA1(9602a8991ca0cf9a7c68710f55c245d9c675b06f))
ROM_END

ROM_START(hardbodyc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(c9248b47) SHA1(54239bd7d15574ebbb70ed306a804b7b32ed516a))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(31c255d0) SHA1(b6ffa2616ae9a4a121585cc402080ec6f26f8472))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.512", 0x0000, 0x10000, CRC(c96f91af) SHA1(9602a8991ca0cf9a7c68710f55c245d9c675b06f))
ROM_END

ROM_START(hardbodyg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "hrdbdy-g.u2", 0x8000, 0x4000, CRC(fce357cc) SHA1(f7d13c12aabcb3c5bb5826b1911817bd359f1941))
	ROM_LOAD( "hrdbdy-g.u3", 0xc000, 0x4000, CRC(ccac74b5) SHA1(d55cfc8ee866a9af4567d56890f5a9ecb9c3c02f))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.512", 0x0000, 0x10000, CRC(c96f91af) SHA1(9602a8991ca0cf9a7c68710f55c245d9c675b06f))
ROM_END

/*-----------------------------------------
/ Heavy Metal Meltdown #0H03
/-----------------------------------------*/
ROM_START(hvymetap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.rom", 0x8000, 0x4000, CRC(53466e4e) SHA1(af6d0e15821ff707f24bb99b8d9dfb9f929906db))
	ROM_LOAD( "u3.rom", 0xc000, 0x4000, CRC(0a08ae7e) SHA1(04f295fbe3a7bd7b929556338914c0ed94a77d62))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.rom", 0x00001, 0x10000, CRC(77933258) SHA1(42a01e97440dbb7d3da92dbfbad2516f4b553a5f))
	ROM_LOAD16_BYTE("u11.rom", 0x00000, 0x10000, CRC(b7e4de7d) SHA1(bcc89e10c368cdbc5137d8f585e109c0be25522d))
ROM_END

ROM_START(hvymetapg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2g.rom", 0x8000, 0x4000, CRC(e50b500a) SHA1(c4f3502bf8afaa94610e008ce6b719ab4c4be712))
	ROM_LOAD( "u3g.rom", 0xc000, 0x4000, CRC(7d018d0d) SHA1(07ba3bd5c15b96d6fc72e0a1de3b5d8defcc53b9))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.rom", 0x00001, 0x10000, CRC(77933258) SHA1(42a01e97440dbb7d3da92dbfbad2516f4b553a5f))
	ROM_LOAD16_BYTE("u11.rom", 0x00000, 0x10000, CRC(b7e4de7d) SHA1(bcc89e10c368cdbc5137d8f585e109c0be25522d))
ROM_END

/*------------------------------------
/ Lady Luck #0E34
/------------------------------------*/
ROM_START(ladyluck)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u3.cpu", 0xc000, 0x4000, CRC(129f41f5) SHA1(0351419814d3f4e98a4572fdec9d53e12fe6b6be))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u4_snd.532", 0x8000, 0x2000, CRC(e9ef01e6) SHA1(79191e776b6683b259cd1a80e9fb3183268bde56))
	ROM_RELOAD(0xa000, 0x2000)
	ROM_LOAD("u3_snd.532", 0xc000, 0x2000, CRC(1bdd6e2b) SHA1(14fc25b5f8eefe8ffab062f83e06ec19403aa00a))
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

/*--------------------------------
/ MotorDome #0E14
/-------------------------------*/
ROM_START(motrdome)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "modm_u2.dat", 0x8000, 0x4000, CRC(820ca073) SHA1(0b50712f7d65f629af934deccc52d588f390a05b))
	ROM_LOAD( "modm_u3.dat", 0xc000, 0x4000, CRC(aae7c418) SHA1(9d3ea83ffff0b9696f5113043475c6e9b9a464ae))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("modm_u7.snd", 0x8000, 0x8000, CRC(29ce4679) SHA1(f17998198b542dd99a34abd678db7e031bde074b))
ROM_END

ROM_START(motrdomeb)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "md_cpu_u2.bin", 0x8000, 0x4000, CRC(72c84e3b) SHA1(cf8d890a574e7f5299abde6fd38be2f3e63b3a54))
	ROM_LOAD( "md_cpu_u3.bin", 0xc000, 0x4000, CRC(3ae93465) SHA1(5b5ecee0c631131201bf4c52f297f87d143d0fcf))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("modm_u7.snd", 0x8000, 0x8000, CRC(29ce4679) SHA1(f17998198b542dd99a34abd678db7e031bde074b))
ROM_END

ROM_START(motrdomeg) // German version claims to be game #E69
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_11_de.bin", 0x8000, 0x4000, CRC(8a4bafd3) SHA1(d764d2e38be2df27ab982cfbedddb79f89ca2359))
	ROM_LOAD( "u3_11_de.bin", 0xc000, 0x4000, CRC(9cb10037) SHA1(7847a71a0295e8de51a8f2f8d406350eca4555bf))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("modm_u7.snd", 0x8000, 0x8000, CRC(29ce4679) SHA1(f17998198b542dd99a34abd678db7e031bde074b))
ROM_END

/*--------------------------------
/ Party Animal #0H01
/-------------------------------*/
ROM_START(prtyanim)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(abdc0b2d) SHA1(b93c7248ea83461101383023bd4e4a50292d8570))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(e48b2d63) SHA1(190fc5a805bda9617c08a29c0bde4d94a77279e9))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("snd_u12.512", 0x00001, 0x10000, CRC(265a9494) SHA1(3b631f2b1c8c685aef32fb6c5289cd792711ff7e))
	ROM_LOAD16_BYTE("snd_u11.512", 0x00000, 0x10000, CRC(20be998f) SHA1(7f98073d0f559e081b2d6dc8c1f3462e3fe9a713))
	ROM_LOAD16_BYTE("snd_u14.512", 0x20001, 0x10000, CRC(639b3db1) SHA1(e07669c3186c963f2fea29bcf5675ac86eb07c86))
	ROM_LOAD16_BYTE("snd_u13.512", 0x20000, 0x10000, CRC(b652597b) SHA1(8b4074a545d420319712a1fdd77a3bfb282ed9cd))
ROM_END

ROM_START(prtyanimg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2g.128", 0x8000, 0x4000, CRC(8abf40a2) SHA1(04ac296c99bc176faf21f1277ff59228a2031715))
	ROM_LOAD( "cpu_u3g.128", 0xc000, 0x4000, CRC(e781dd4b) SHA1(3395ddd2d774c83cac98b6d67415d3c8cd0b04fe))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("snd_u12.512", 0x00001, 0x10000, CRC(265a9494) SHA1(3b631f2b1c8c685aef32fb6c5289cd792711ff7e))
	ROM_LOAD16_BYTE("snd_u11.512", 0x00000, 0x10000, CRC(20be998f) SHA1(7f98073d0f559e081b2d6dc8c1f3462e3fe9a713))
	ROM_LOAD16_BYTE("snd_u14.512", 0x20001, 0x10000, CRC(639b3db1) SHA1(e07669c3186c963f2fea29bcf5675ac86eb07c86))
	ROM_LOAD16_BYTE("snd_u13.512", 0x20000, 0x10000, CRC(b652597b) SHA1(8b4074a545d420319712a1fdd77a3bfb282ed9cd))
ROM_END

/*------------------------------------
/ Special Force #0E47 - 1st Game to use Sounds Deluxe Sound Hardware
/------------------------------------*/
ROM_START(specforc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_revc.128", 0x8000, 0x4000, CRC(d042af04) SHA1(0a73ee6d3ce603899fd89de70f90e9efc58b8b42))
	ROM_LOAD( "u3_revc.128", 0xc000, 0x4000, CRC(d48a5eaf) SHA1(90a5d5e928abfec699bae9d0087e90316339058f))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12_snd.512", 0x00001, 0x10000, CRC(4f48a490) SHA1(6c9a594ecc68adf3b1eda315c4704e1d025a3442))
	ROM_LOAD16_BYTE("u11_snd.512", 0x00000, 0x10000, CRC(b16eb713) SHA1(461e5ed82891d17849984137536bc6d1ab2907c2))
	ROM_LOAD16_BYTE("u14_snd.512", 0x20001, 0x10000, CRC(6911fa51) SHA1(a75f75bb4493b0ea3a423bc033d49022228d79c1))
	ROM_LOAD16_BYTE("u13_snd.512", 0x20000, 0x10000, CRC(3edda92d) SHA1(dbd95bb1c534779f56cc9e30efef159feaf22712))
ROM_END

/*------------------------------------
/ Strange Science #0E35
/------------------------------------*/

ROM_START(strngsci)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "strange_scienc.u2", 0x8000, 0x4000, CRC(0a0ebf25) SHA1(6b120e5b3aa13d1650c4ee8c4c98996b13be167e)) // 12/12/86
	ROM_LOAD( "strange_scienc.u3", 0xc000, 0x4000, CRC(c5b17b07) SHA1(823eb1e2ceb33f221b69c11eb71cb53c48d0f716)) // 12/12/86
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.256", 0x8000, 0x8000, CRC(bc33901e) SHA1(5231d8f01a107742acee2d13580a461063018a11))
ROM_END

ROM_START(strngscia)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(2ffcf284) SHA1(27d66806708c983092bab4ed6965c2e91e69acdc))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(35257931) SHA1(d3d6b84e50677a4c5f9d5c13c9522ad6d3a1358d))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.256", 0x8000, 0x8000, CRC(bc33901e) SHA1(5231d8f01a107742acee2d13580a461063018a11))
ROM_END

ROM_START(strngscig)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpub_u2.128", 0x8000, 0x4000, CRC(48ef1052) SHA1(afcb0520ab834c0d6ef4a73f615c48653ccedc24))
	ROM_LOAD( "cpub_u3.128", 0xc000, 0x4000, CRC(da5b4b3b) SHA1(ff9babf2efc6622803db9ba8712dd8b76c8412b8))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.256", 0x8000, 0x8000, CRC(bc33901e) SHA1(5231d8f01a107742acee2d13580a461063018a11))
ROM_END

/*-------------------------------------------------------------
/ Truck Stop #2001
/-------------------------------------------------------------*/
ROM_START(trucksp3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_p3.128", 0x8000, 0x4000, CRC(79b2a5b1) SHA1(d3de91bfadc9684302b2367cfcb30ed0d6faa020))
	ROM_LOAD( "u3_p3.128", 0xc000, 0x4000, CRC(2993373c) SHA1(26490f1dd8a5329a88a2ceb1e6044711a29f1445))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASE00)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("u4sndp1.256", 0x00000, 0x8000, CRC(120a386f) SHA1(51b3b45eb7ea63758b21aad404ba12a9607fec44))
	ROM_RELOAD(0x00000 +0x8000, 0x8000)
	ROM_LOAD("u19sndp1.256", 0x10000, 0x8000, CRC(5cd43dda) SHA1(23dd8a52ea1340fc239a246af0d94da905464efb))
	ROM_RELOAD(0x10000 +0x8000, 0x8000)
	ROM_LOAD("u20sndp1.256", 0x20000, 0x8000, CRC(93ac5c33) SHA1(f6dc84eca4678188a58ba3c8ef18975164dd29b0))
	ROM_RELOAD(0x20000 +0x8000, 0x8000)
ROM_END

ROM_START(trucksp2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_p2.128", 0x8000, 0x4000, CRC(3c397dec) SHA1(2fc86ad39c935ce8615eafd67e571ac94c938cd7))
	ROM_LOAD( "u3_p2.128", 0xc000, 0x4000, CRC(d7ac519a) SHA1(612bf9fee0d54e8b1215508bd6c1ea61dcb99951))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASE00)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("u4sndp1.256", 0x00000, 0x8000, CRC(120a386f) SHA1(51b3b45eb7ea63758b21aad404ba12a9607fec44))
	ROM_RELOAD(0x00000 +0x8000, 0x8000)
	ROM_LOAD("u19sndp1.256", 0x10000, 0x8000, CRC(5cd43dda) SHA1(23dd8a52ea1340fc239a246af0d94da905464efb))
	ROM_RELOAD(0x10000 +0x8000, 0x8000)
	ROM_LOAD("u20sndp1.256", 0x20000, 0x8000, CRC(93ac5c33) SHA1(f6dc84eca4678188a58ba3c8ef18975164dd29b0))
	ROM_RELOAD(0x20000 +0x8000, 0x8000)
ROM_END

} // anonymous namespace


// 1st gen
GAME( 1985, eballchp,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Eight Ball Champ",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1985, beatclck,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Beat the Clock",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1985, beatclck2, beatclck, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Beat the Clock (with flasher support)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, ladyluck,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Lady Luck",                             MACHINE_IS_SKELETON_MECHANICAL )

// 2nd gen
GAME( 1986, motrdome,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "MotorDome (rev. D)",                    MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, motrdomeb, motrdome, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "MotorDome (rev. B)",                    MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, motrdomeg, motrdome, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "MotorDome (German rev. B)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, strngsci,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Strange Science (Rev C)",               MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, strngscia, strngsci, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Strange Science (Rev A)",               MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, strngscig, strngsci, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Strange Science (German, Rev A)",       MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, specforc,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Special Force",                         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, blackblt,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Black Belt",                            MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, blackblt2, blackblt, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Black Belt (Squawk and Talk)",          MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, cityslck,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "City Slicker",                          MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, hardbody,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Hardbody (rev. D)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, hardbodyc, hardbody, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Hardbody (rev. C)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, hardbodyg, hardbody, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Hardbody (German rev. B)",              MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, prtyanim,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Party Animal",                          MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, prtyanimg, prtyanim, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Party Animal (German)",                 MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, hvymetap,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Heavy Metal Meltdown",                  MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, hvymetapg, hvymetap, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Heavy Metal Meltdown (German)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, esclwrld,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Escape from the Lost World",            MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, esclwrldg, esclwrld, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Escape from the Lost World (German)",   MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, dungdrag,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Dungeons & Dragons",                    MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1988, black100,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Blackwater 100",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1988, black100s, black100, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Blackwater 100 (Single Ball Play)",     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1988, trucksp3,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Truck Stop (P-3)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1988, trucksp2,  trucksp3, by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Truck Stop (P-2)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1989, atlantip,  0,        by6803, by6803, by6803_state, empty_init, ROT0, "Bally", "Atlantis",                              MACHINE_IS_SKELETON_MECHANICAL )
