//
//	main.cpp
//

#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <unistd.h>

#include "mc6809.h"
#include "mc6850.h"
#include "term.h"
#include "memory.h"

static uint8_t ROM16KB[] = {
#include "../my/ROM/ROM16KB.h"
};

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: usim <hexfile>\n");
		return EXIT_FAILURE;
	}

	(void)signal(SIGINT, SIG_IGN);

	const size_t ram0_size = 0x8000;
	const size_t ram8_size = 0x4000;
	const size_t ramC_size = 0x2000;
	const size_t shmE1_size = 0x0100;
	const size_t shmE2_size = 0x0200;
	const size_t shmE4_size = 0x0400;
	const size_t shmE8_size = 0x0800;
	const size_t romF_size = 0x1000;

	mc6809			cpu;
	Terminal 		term(cpu);

	auto ram0 = std::make_shared<RAM>(ram0_size);
	auto ram8 = std::make_shared<RAM>(ram8_size);
	auto ramC = std::make_shared<RAM>(ramC_size);
	for (Word i = 0; i < ramC_size; ++i) {
		ramC->write(i, ROM16KB[i]);
	}
	for (Word i = 0x0100; i < 0x0a00; ++i) {
		ram0->write(i, ROM16KB[ramC_size + i]);
	}
	auto shmE1 = std::make_shared<RAM>(shmE1_size);
	auto shmE2 = std::make_shared<RAM>(shmE2_size);
	auto shmE4 = std::make_shared<RAM>(shmE4_size);
	auto shmE8 = std::make_shared<RAM>(shmE8_size);
#if 1
	// disable 6309
	ROM16KB[0x3047] = 0x86; // lda
	ROM16KB[0x3048] = 0x10; // #16
#endif
	auto romF = std::make_shared<ROM_Data>(&ROM16KB[sizeof(ROM16KB)-romF_size], romF_size);
	auto acia = std::make_shared<mc6850>(term);

	cpu.attach(ram0,  0x0000, 0x8000);
	cpu.attach(ram8,  0x8000, 0xc000);
	cpu.attach(ramC,  0xc000, 0xe000);
	cpu.attach(acia,  0xe000, 0xfffe);
	cpu.attach(shmE1, 0xe100, 0xff00);
	cpu.attach(shmE2, 0xe200, 0xfe00);
	cpu.attach(shmE4, 0xe400, 0xfc00);
	cpu.attach(shmE8, 0xe800, 0xf800);
	cpu.attach(romF,  0xf000, 0xf000);

	cpu.FIRQ.bind([&]() {
		return acia->IRQ;
	});

#if 0
	rom->load_intelhex(argv[1], rom_base);
#else
	(void)argv[1];
#endif

	cpu.reset();

	bool halted = false;
	while (!halted) {
		// start
		/*
		if (cpu.get_pc() == 0x0100) {
			cpu.tron();
		}
		*/
#if 0
		// IN start
		if (cpu.get_pc() == 0x0172) {
			cpu.troff();
		}
		// IN end
		if (cpu.get_pc() == 0x017F) {
			cpu.tron();
		}
		// WAITACIA start
		if (cpu.get_pc() == 0x09ED) {
			cpu.troff();
		}
		// WAITACIA end
		if (cpu.get_pc() == 0x09F0) {
			cpu.tron();
		}
		// LET start
		if (cpu.get_pc() == 0x0765) {
			cpu.tron();
		}
		// LET end
		if (cpu.get_pc() == 0x076E) {
			cpu.troff();
		}
		// PNUM start
		if (cpu.get_pc() == 0x0452) {
			cpu.tron();
		}
		// USER start
		if (cpu.get_pc() == 0x0663) {
			cpu.tron();
		}
#endif
		cpu.tick();
		if (cpu.get_trace()) {
			getc(::stdin);
		}
	}

	return EXIT_SUCCESS;
}
